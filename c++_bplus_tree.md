## 内存 B+ tree 实现与测试

### B+ Tree 实现

```c++
// 将上述代码保存为 btree.hpp
#include <cassert>
#include <climits>
#include <iostream>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>

#define PAGESIZE 512
using entry_key_t = int64_t;
using namespace std;
class page;

class btree
{
private:
  int height;
  char *root;

public:
  btree();
  void setNewRoot(char *);
  void btree_insert(entry_key_t, char *);
  void btree_insert_internal(char *, entry_key_t, char *, uint32_t);
  void btree_delete(entry_key_t);
  void btree_delete_internal(entry_key_t, char *, uint32_t, entry_key_t *,
                             bool *, page **);
  char *btree_search(entry_key_t);
  void btree_search_range(entry_key_t, entry_key_t, unsigned long *, int &offset);
  void printAll();

  friend class page;
};

class header
{
private:
  page *leftmost_ptr;     // 8 bytes
  page *sibling_ptr;      // 8 bytes
  uint32_t level;         // 4 bytes
  uint8_t switch_counter; // 1 bytes
  uint8_t is_deleted;     // 1 bytes
  int16_t last_index;     // 2 bytes
  char dummy[8];          // 8 bytes

  friend class page;
  friend class btree;

public:
  header()
  {
    leftmost_ptr = nullptr;
    sibling_ptr = nullptr;
    switch_counter = 0;
    last_index = -1;
    is_deleted = false;
  }

  ~header() {}
};

class entry
{
private:
  entry_key_t key; // 8 bytes
  char *ptr;       // 8 bytes
public:
  entry()
  {
    key = LONG_MAX;
    ptr = nullptr;
  }

  friend class page;
  friend class btree;
};

const int cardinality = (PAGESIZE - sizeof(header)) / sizeof(entry); // total number of entry

class page
{
private:
  header hdr;                 // header in memory, 16 bytes
  entry records[cardinality]; // slots in memory, 16 bytes * n

public:
  friend class btree;
  page(uint32_t level = 0)
  {
    hdr.level = level;
    records[0].ptr = nullptr;
  }

  page(page *left, entry_key_t key, page *right, uint32_t level = 0)
  {
    hdr.leftmost_ptr = left;
    hdr.level = level;
    records[0].key = key;
    records[0].ptr = (char *)right;
    records[1].ptr = nullptr;

    hdr.last_index = 0;
  }

  void *operator new(size_t size)
  {
    void *ret;
    posix_memalign(&ret, 64, size);
    return ret;
  }

  inline int count()
  {
    uint8_t previous_switch_counter;
    int count = 0;
    do
    {
      previous_switch_counter = hdr.switch_counter;
      count = hdr.last_index + 1;

      while (count >= 0 && records[count].ptr != nullptr)
      {
        if (previous_switch_counter % 2 == 0)
          ++count;
        else
          --count;
      }

      if (count < 0)
      {
        count = 0;
        while (records[count].ptr != nullptr)
        {
          ++count;
        }
      }

    } while (previous_switch_counter != hdr.switch_counter);

    return count;
  }

  inline bool remove_key(entry_key_t key)
  {
    // Set the switch_counter
    if (hdr.switch_counter % 2 == 0)
      ++hdr.switch_counter;

    bool shift = false;
    int i;
    for (i = 0; records[i].ptr != nullptr; ++i)
    {
      if (!shift && records[i].key == key)
      {
        records[i].ptr =
            (i == 0) ? (char *)hdr.leftmost_ptr : records[i - 1].ptr;
        shift = true;
      }

      if (shift)
      {
        records[i].key = records[i + 1].key;
        records[i].ptr = records[i + 1].ptr;
      }
    }

    if (shift)
    {
      --hdr.last_index;
    }
    return shift;
  }

  bool remove(btree *bt, entry_key_t key, bool only_rebalance = false,
              bool with_lock = true)
  {
    if (!only_rebalance)
    {
      register int num_entries_before = count();

      // root
      if (this == (page *)bt->root)
      {
        if (hdr.level > 0)
        {
          if (num_entries_before == 1 && !hdr.sibling_ptr)
          {
            bt->root = (char *)hdr.leftmost_ptr;
            hdr.is_deleted = 1;
          }
        }

        bool ret = remove_key(key);
        return true;
      }

      bool should_rebalance = true;
      if (num_entries_before - 1 >= (int)((cardinality - 1) * 0.5))
      {
        should_rebalance = false;
      }

      bool ret = remove_key(key);

      if (!should_rebalance)
      {
        return (hdr.leftmost_ptr == nullptr) ? ret : true;
      }
    }

    entry_key_t deleted_key_from_parent = 0;
    bool is_leftmost_node = false;
    page *left_sibling;
    bt->btree_delete_internal(key, (char *)this, hdr.level + 1,
                              &deleted_key_from_parent, &is_leftmost_node,
                              &left_sibling);

    if (is_leftmost_node)
    {
      hdr.sibling_ptr->remove(bt, hdr.sibling_ptr->records[0].key, true,
                              with_lock);
      return true;
    }

    register int num_entries = count();
    register int left_num_entries = left_sibling->count();

    // Merge
    int total_num_entries = num_entries + left_num_entries;
    if (hdr.leftmost_ptr)
      ++total_num_entries;

    entry_key_t parent_key;

    if (total_num_entries > cardinality - 1)
    {
      register int m = (int)ceil(total_num_entries / 2);

      if (num_entries < left_num_entries)
      { // left -> right
        if (hdr.leftmost_ptr == nullptr)
        {
          for (int i = left_num_entries - 1; i >= m; i--)
          {
            insert_key(left_sibling->records[i].key,
                       left_sibling->records[i].ptr, &num_entries);
          }
          left_sibling->records[m].ptr = nullptr;

          left_sibling->hdr.last_index = m - 1;

          parent_key = records[0].key;
        }
        else
        {
          insert_key(deleted_key_from_parent, (char *)hdr.leftmost_ptr,
                     &num_entries);

          for (int i = left_num_entries - 1; i > m; i--)
          {
            insert_key(left_sibling->records[i].key,
                       left_sibling->records[i].ptr, &num_entries);
          }

          parent_key = left_sibling->records[m].key;

          hdr.leftmost_ptr = (page *)left_sibling->records[m].ptr;

          left_sibling->records[m].ptr = nullptr;

          left_sibling->hdr.last_index = m - 1;
        }

        if (left_sibling == ((page *)bt->root))
        {
          page *new_root =
              new page(left_sibling, parent_key, this, hdr.level + 1);
          bt->setNewRoot((char *)new_root);
        }
        else
        {
          bt->btree_insert_internal((char *)left_sibling, parent_key,
                                    (char *)this, hdr.level + 1);
        }
      }
      else
      { // from leftmost case
        hdr.is_deleted = 1;

        page *new_sibling = new page(hdr.level);
        new_sibling->hdr.sibling_ptr = hdr.sibling_ptr;

        int num_dist_entries = num_entries - m;
        int new_sibling_cnt = 0;

        if (hdr.leftmost_ptr == nullptr)
        {
          for (int i = 0; i < num_dist_entries; i++)
          {
            left_sibling->insert_key(records[i].key, records[i].ptr,
                                     &left_num_entries);
          }

          for (int i = num_dist_entries; records[i].ptr != nullptr; i++)
          {
            new_sibling->insert_key(records[i].key, records[i].ptr,
                                    &new_sibling_cnt, false);
          }

          left_sibling->hdr.sibling_ptr = new_sibling;
          parent_key = new_sibling->records[0].key;
        }
        else
        {
          left_sibling->insert_key(deleted_key_from_parent,
                                   (char *)hdr.leftmost_ptr, &left_num_entries);

          for (int i = 0; i < num_dist_entries - 1; i++)
          {
            left_sibling->insert_key(records[i].key, records[i].ptr,
                                     &left_num_entries);
          }

          parent_key = records[num_dist_entries - 1].key;

          new_sibling->hdr.leftmost_ptr =
              (page *)records[num_dist_entries - 1].ptr;
          for (int i = num_dist_entries; records[i].ptr != nullptr; i++)
          {
            new_sibling->insert_key(records[i].key, records[i].ptr,
                                    &new_sibling_cnt, false);
          }

          left_sibling->hdr.sibling_ptr = new_sibling;
        }

        if (left_sibling == ((page *)bt->root))
        {
          page *new_root =
              new page(left_sibling, parent_key, new_sibling, hdr.level + 1);
          bt->setNewRoot((char *)new_root);
        }
        else
        {
          bt->btree_insert_internal((char *)left_sibling, parent_key,
                                    (char *)new_sibling, hdr.level + 1);
        }
      }
    }
    else
    {
      hdr.is_deleted = 1;
      if (hdr.leftmost_ptr)
        left_sibling->insert_key(deleted_key_from_parent,
                                 (char *)hdr.leftmost_ptr, &left_num_entries);

      for (int i = 0; records[i].ptr != nullptr; ++i)
      {
        left_sibling->insert_key(records[i].key, records[i].ptr,
                                 &left_num_entries);
      }

      left_sibling->hdr.sibling_ptr = hdr.sibling_ptr;
    }

    return true;
  }

  inline void insert_key(entry_key_t key, char *ptr, int *num_entries,
                         bool update_last_index = true)
  {
    // update switch_counter
    if (hdr.switch_counter % 2 != 0)
      ++hdr.switch_counter;

    if (*num_entries == 0)
    { // this page is empty
      entry *new_entry = (entry *)&records[0];
      entry *array_end = (entry *)&records[1];
      new_entry->key = (entry_key_t)key;
      new_entry->ptr = (char *)ptr;

      array_end->ptr = (char *)nullptr;
    }
    else
    {
      int i = *num_entries - 1, inserted = 0;
      records[*num_entries + 1].ptr = records[*num_entries].ptr;

      for (i = *num_entries - 1; i >= 0; i--)
      {
        if (key < records[i].key)
        {
          records[i + 1].ptr = records[i].ptr;
          records[i + 1].key = records[i].key;
        }
        else
        {
          records[i + 1].ptr = records[i].ptr;
          records[i + 1].key = key;
          records[i + 1].ptr = ptr;
          inserted = 1;
          break;
        }
      }
      if (inserted == 0)
      {
        records[0].ptr = (char *)hdr.leftmost_ptr;
        records[0].key = key;
        records[0].ptr = ptr;
      }
    }

    if (update_last_index)
    {
      hdr.last_index = *num_entries;
    }
    ++(*num_entries);
  }

  // Insert a new key
  page *store(btree *bt, char *left, entry_key_t key, char *right,
              page *invalid_sibling = nullptr)
  {
    // If node has a sibling node
    if (hdr.sibling_ptr && (hdr.sibling_ptr != invalid_sibling))
    {
      if (key > hdr.sibling_ptr->records[0].key)
      {
        return hdr.sibling_ptr->store(bt, nullptr, key, right,
                                      invalid_sibling);
      }
    }

    register int num_entries = count();

    if (num_entries < cardinality - 1)
    {
      insert_key(key, right, &num_entries);
      return this;
    }
    else
    {
      // overflow
      page *sibling = new page(hdr.level);
      register int m = (int)ceil(num_entries / 2);
      entry_key_t split_key = records[m].key;

      int sibling_cnt = 0;
      if (hdr.leftmost_ptr == nullptr)
      { // leaf node
        for (int i = m; i < num_entries; ++i)
        {
          sibling->insert_key(records[i].key, records[i].ptr, &sibling_cnt,
                              false);
        }
      }
      else
      {
        for (int i = m + 1; i < num_entries; ++i)
        {
          sibling->insert_key(records[i].key, records[i].ptr, &sibling_cnt,
                              false);
        }
        sibling->hdr.leftmost_ptr = (page *)records[m].ptr;
      }

      sibling->hdr.sibling_ptr = hdr.sibling_ptr;

      hdr.sibling_ptr = sibling;

      if (hdr.switch_counter % 2 == 0)
        hdr.switch_counter += 2;
      else
        ++hdr.switch_counter;
      records[m].ptr = nullptr;

      hdr.last_index = m - 1;

      num_entries = hdr.last_index + 1;

      page *ret;

      // insert the key
      if (key < split_key)
      {
        insert_key(key, right, &num_entries);
        ret = this;
      }
      else
      {
        sibling->insert_key(key, right, &sibling_cnt);
        ret = sibling;
      }

      // Set a new root or insert the split key to the parent
      if (bt->root == (char *)this)
      {
        page *new_root =
            new page((page *)this, split_key, sibling, hdr.level + 1);
        bt->setNewRoot((char *)new_root);
      }
      else
      {
        bt->btree_insert_internal(nullptr, split_key, (char *)sibling,
                                  hdr.level + 1);
      }

      return ret;
    }
  }

  void linear_search_range(entry_key_t min, entry_key_t max,
                           unsigned long *buf, int &off)
  {
    int i;
    uint8_t previous_switch_counter;
    page *current = this;

    while (current)
    {
      int old_off = off;
      do
      {
        previous_switch_counter = current->hdr.switch_counter;
        off = old_off;

        entry_key_t tmp_key;
        char *tmp_ptr;

        if (previous_switch_counter % 2 == 0)
        {
          if ((tmp_key = current->records[0].key) > min)
          {
            if (tmp_key < max)
            {
              if ((tmp_ptr = current->records[0].ptr) != nullptr)
              {
                if (tmp_key == current->records[0].key)
                {
                  if (tmp_ptr)
                  {
                    buf[off++] = (unsigned long)tmp_ptr;
                  }
                }
              }
            }
            else
              return;
          }

          for (i = 1; current->records[i].ptr != nullptr; ++i)
          {
            if ((tmp_key = current->records[i].key) > min)
            {
              if (tmp_key < max)
              {
                if ((tmp_ptr = current->records[i].ptr) !=
                    current->records[i - 1].ptr)
                {
                  if (tmp_key == current->records[i].key)
                  {
                    if (tmp_ptr)
                      buf[off++] = (unsigned long)tmp_ptr;
                  }
                }
              }
              else
                return;
            }
          }
        }
        else
        {
          for (i = count() - 1; i > 0; --i)
          {
            if ((tmp_key = current->records[i].key) > min)
            {
              if (tmp_key < max)
              {
                if ((tmp_ptr = current->records[i].ptr) !=
                    current->records[i - 1].ptr)
                {
                  if (tmp_key == current->records[i].key)
                  {
                    if (tmp_ptr)
                      buf[off++] = (unsigned long)tmp_ptr;
                  }
                }
              }
              else
                return;
            }
          }

          if ((tmp_key = current->records[0].key) > min)
          {
            if (tmp_key < max)
            {
              if ((tmp_ptr = current->records[0].ptr) != nullptr)
              {
                if (tmp_key == current->records[0].key)
                {
                  if (tmp_ptr)
                  {
                    buf[off++] = (unsigned long)tmp_ptr;
                  }
                }
              }
            }
            else
              return;
          }
        }
      } while (previous_switch_counter != current->hdr.switch_counter);

      current = current->hdr.sibling_ptr;
    }
  }

  char *linear_search(entry_key_t key)
  {
    int i = 1;
    uint8_t previous_switch_counter;
    char *ret = nullptr;
    char *t;
    entry_key_t k;

    if (hdr.leftmost_ptr == nullptr)
    { // Search a leaf node
      do
      {
        previous_switch_counter = hdr.switch_counter;
        ret = nullptr;

        // search from left ro right
        if (previous_switch_counter % 2 == 0)
        {
          if ((k = records[0].key) == key)
          {
            if ((t = records[0].ptr) != nullptr)
            {
              if (k == records[0].key)
              {
                ret = t;
                continue;
              }
            }
          }

          for (i = 1; records[i].ptr != nullptr; ++i)
          {
            if ((k = records[i].key) == key)
            {
              if (records[i - 1].ptr != (t = records[i].ptr))
              {
                if (k == records[i].key)
                {
                  ret = t;
                  break;
                }
              }
            }
          }
        }
        else
        { // search from right to left
          for (i = count() - 1; i > 0; --i)
          {
            if ((k = records[i].key) == key)
            {
              if (records[i - 1].ptr != (t = records[i].ptr) && t)
              {
                if (k == records[i].key)
                {
                  ret = t;
                  break;
                }
              }
            }
          }

          if (!ret)
          {
            if ((k = records[0].key) == key)
            {
              if (nullptr != (t = records[0].ptr) && t)
              {
                if (k == records[0].key)
                {
                  ret = t;
                  continue;
                }
              }
            }
          }
        }
      } while (hdr.switch_counter != previous_switch_counter);

      if (ret)
      {
        return ret;
      }

      if ((t = (char *)hdr.sibling_ptr) && key >= ((page *)t)->records[0].key)
        return t;

      return nullptr;
    }
    else
    { // internal node
      do
      {
        previous_switch_counter = hdr.switch_counter;
        ret = nullptr;

        if (previous_switch_counter % 2 == 0)
        {
          if (key < (k = records[0].key))
          {
            if ((t = (char *)hdr.leftmost_ptr) != records[0].ptr)
            {
              ret = t;
              continue;
            }
          }

          for (i = 1; records[i].ptr != nullptr; ++i)
          {
            if (key < (k = records[i].key))
            {
              if ((t = records[i - 1].ptr) != records[i].ptr)
              {
                ret = t;
                break;
              }
            }
          }

          if (!ret)
          {
            ret = records[i - 1].ptr;
            continue;
          }
        }
        else
        { // search from right to left
          for (i = count() - 1; i >= 0; --i)
          {
            if (key >= (k = records[i].key))
            {
              if (i == 0)
              {
                if ((char *)hdr.leftmost_ptr != (t = records[i].ptr))
                {
                  ret = t;
                  break;
                }
              }
              else
              {
                if (records[i - 1].ptr != (t = records[i].ptr))
                {
                  ret = t;
                  break;
                }
              }
            }
          }
        }
      } while (hdr.switch_counter != previous_switch_counter);

      if ((t = (char *)hdr.sibling_ptr) != nullptr)
      {
        if (key >= ((page *)t)->records[0].key)
          return t;
      }

      if (ret)
      {
        return ret;
      }
      else
        return (char *)hdr.leftmost_ptr;
    }

    return nullptr;
  }

  // print a node
  void print()
  {
    if (hdr.leftmost_ptr == nullptr)
      printf("[%d] leaf %x \n", this->hdr.level, this);
    else
      printf("[%d] internal %x \n", this->hdr.level, this);
    printf("last_index: %d\n", hdr.last_index);
    printf("switch_counter: %d\n", hdr.switch_counter);
    printf("search direction: ");
    if (hdr.switch_counter % 2 == 0)
      printf("->\n");
    else
      printf("<-\n");

    if (hdr.leftmost_ptr != nullptr)
      printf("%x ", hdr.leftmost_ptr);

    for (int i = 0; records[i].ptr != nullptr; ++i)
      printf("%ld,%x ", records[i].key, records[i].ptr);

    printf("%x ", hdr.sibling_ptr);

    printf("\n");
  }

  void printAll()
  {
    if (hdr.leftmost_ptr == nullptr)
    {
      printf("printing leaf node: ");
      print();
    }
    else
    {
      printf("printing internal node: ");
      print();
      ((page *)hdr.leftmost_ptr)->printAll();
      for (int i = 0; records[i].ptr != nullptr; ++i)
      {
        ((page *)records[i].ptr)->printAll();
      }
    }
  }
};

/*
 *  class btree
 */
btree::btree()
{
  root = (char *)new page();
  height = 1;
}

void btree::setNewRoot(char *new_root)
{
  this->root = (char *)new_root;
  ++height;
}

char *btree::btree_search(entry_key_t key)
{
  page *p = (page *)root;

  while (p->hdr.leftmost_ptr != nullptr)
  {
    p = (page *)p->linear_search(key);
  }

  page *t;
  while ((t = (page *)p->linear_search(key)) == p->hdr.sibling_ptr)
  {
    p = t;
    if (!p)
    {
      break;
    }
  }

  if (!t)
  {
    printf("NOT FOUND %lu, t = %x\n", key, t);
    return nullptr;
  }

  return (char *)t;
}

// insert the key in the leaf node
void btree::btree_insert(entry_key_t key, char *right)
{
  page *p = (page *)root;

  while (p->hdr.leftmost_ptr != nullptr)
  {
    p = (page *)p->linear_search(key);
  }

  if (!p->store(this, nullptr, key, right))
  {
    btree_insert(key, right);
  }
}

void btree::btree_insert_internal(char *left, entry_key_t key, char *right,
                                  uint32_t level)
{
  if (level > ((page *)root)->hdr.level)
    return;

  page *p = (page *)this->root;

  while (p->hdr.level > level)
    p = (page *)p->linear_search(key);

  if (!p->store(this, nullptr, key, right))
  {
    btree_insert_internal(left, key, right, level);
  }
}

void btree::btree_delete(entry_key_t key)
{
  page *p = (page *)root;

  while (p->hdr.leftmost_ptr != nullptr)
  {
    p = (page *)p->linear_search(key);
  }

  page *t;
  while ((t = (page *)p->linear_search(key)) == p->hdr.sibling_ptr)
  {
    p = t;
    if (!p)
      break;
  }

  if (p)
  {
    if (!p->remove(this, key))
    {
      btree_delete(key);
    }
  }
  else
  {
    printf("not found the key to delete %lu\n", key);
  }
}

void btree::btree_delete_internal(entry_key_t key, char *ptr, uint32_t level,
                                  entry_key_t *deleted_key,
                                  bool *is_leftmost_node, page **left_sibling)
{
  if (level > ((page *)this->root)->hdr.level)
    return;

  page *p = (page *)this->root;

  while (p->hdr.level > level)
  {
    p = (page *)p->linear_search(key);
  }

  if ((char *)p->hdr.leftmost_ptr == ptr)
  {
    *is_leftmost_node = true;
    return;
  }

  *is_leftmost_node = false;

  for (int i = 0; p->records[i].ptr != nullptr; ++i)
  {
    if (p->records[i].ptr == ptr)
    {
      if (i == 0)
      {
        if ((char *)p->hdr.leftmost_ptr != p->records[i].ptr)
        {
          *deleted_key = p->records[i].key;
          *left_sibling = p->hdr.leftmost_ptr;
          p->remove(this, *deleted_key, false, false);
          break;
        }
      }
      else
      {
        if (p->records[i - 1].ptr != p->records[i].ptr)
        {
          *deleted_key = p->records[i].key;
          *left_sibling = (page *)p->records[i - 1].ptr;
          p->remove(this, *deleted_key, false, false);
          break;
        }
      }
    }
  }
}

// range search
void btree::btree_search_range(entry_key_t min, entry_key_t max,
                               unsigned long *buf, int &offset)
{
  page *p = (page *)root;

  while (p)
  {
    if (p->hdr.leftmost_ptr != nullptr)
    {
      p = (page *)p->linear_search(min);
    }
    else
    {
      // leaf node
      p->linear_search_range(min, max, buf, offset);

      break;
    }
  }
}

void btree::printAll()
{
  int total_keys = 0;
  page *leftmost = (page *)root;
  printf("root: %x\n", root);
  if (root)
  {
    do
    {
      page *sibling = leftmost;
      while (sibling)
      {
        if (sibling->hdr.level == 0)
        {
          total_keys += sibling->hdr.last_index + 1;
        }
        sibling->print();
        sibling = sibling->hdr.sibling_ptr;
      }
      printf("-----------------------------------------\n");
      leftmost = leftmost->hdr.leftmost_ptr;
    } while (leftmost);
  }

  printf("total number of keys: %d\n", total_keys);
}
```

### 测试 B+ tree 功能

```c++
// 将上述代码保存为 test.cpp
#include "btree.hpp"

typedef struct Row{
    int a;
    int b;
}Row;

void task1(Row* rows, int nrows){
    // construct b plus tree index
    btree* bt = new btree();
    for(int i = 0; i < nrows; i++){
        bt->btree_insert(rows[i].b, (char*) &rows[i]);
    }
    // enum b plus tree
    bt->printAll();
    
    bt->btree_search(77);
}

int main(){
    Row rows[] = {{1000, 20}, {1000, 31}, {500, 75}, {2000, 31}, {2000, 16}};
    int len = sizeof(rows) / sizeof(rows[0]);
    task1(rows, len);
    return 0;
}
```



```shell
# Makefile
.PHONY: all clean
.DEFAULT_GOAL := all

LIBS=-lrt -lm
INCLUDES=-I./include
CFLAGS=-O3 -std=c++11 -g

output = btree

all: main

main: test.cpp
        g++ $(CFLAGS) -o btree test.cpp $(LIBS)

clean:
        rm $(output)
```
