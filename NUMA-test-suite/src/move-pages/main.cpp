#include <numa.h>
#include <numaif.h>
#include <unistd.h>
#include <sys/time.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <iostream>

#include "../share/utils.h"
#include "../share/optParser.h"

/* definition of bitmask */
// struct bitmask {
// 	unsigned long size; /* number of bits in the map */
// 	unsigned long *maskp;
// };


/* config */
int cpu_node = 0;
int mem_node = 0;
int migrated_node = 1;

float timeout = 1;
bool sequential_only = false;
bool random_only = false;

int page_size = getpagesize();
int page_num = 5000;
char* char_arr;
long long arr_size = page_size * page_num;

/* Note that alignment must be a power of two. */
/* free() does not free the ignored part */
void * allocate_aligned(size_t size, size_t alignment)
{
  const size_t mask = alignment - 1;
  const uintptr_t mem = (uintptr_t) malloc(size + alignment);
  return (void *) ((mem + mask) & ~mask);
}

void* allocate_pages_aligned() {
  void* ret = malloc((page_num+1)*page_size);
  return align_ptr_up(ret, page_size);
}

void alloc_pages() {
  char_arr = (char*)allocate_pages_aligned();
  arr_size = page_num * page_size;
  printf("start %p\n", char_arr);
}


void bind_cpu(int node) {
  int ret = numa_run_on_node(node); // runs the current task and its children on node 0
  if (ret != 0) {
    fprintf(stderr, "numa_run_on_node failed\n");
    exit(0);
  }
  printf("bind the current task to run on node %d\n", node);
}

void bind_mem(int node) {
  bitmask* m = new bitmask;
  m->size = numa_num_task_nodes();
  m->maskp = new unsigned long;
  *m->maskp = 0;
  *m->maskp |= 1 << node;
  
  numa_set_membind(m);

  printf("bind the current task to allocate on node %d\n", node);

}

void loop_access(int pattern, int mode) {
  const char* patterns[2] = {"sequential", "random"};

  printf("  %s access test\n", patterns[pattern]);
  
  struct timeval tv_start, tv_end, tv_delta;  
  gettimeofday(&tv_start, NULL);
  srand(time(NULL));

  while (1) {
    for (int i = 0; i < arr_size; i++) {
      int j = i;
      if (pattern == 1) {
        j = rand() % arr_size;
      }

      if (mode == 0) {
        int dummy = char_arr[j];
      }
      else if (mode == 1) {
        char_arr[j] = 0;
      }
      else if (mode == 2) {
        ++char_arr[j];
      }
    }

    gettimeofday(&tv_end, NULL);
    timersub(&tv_end, &tv_start, &tv_delta);

    if (tv_delta.tv_sec > timeout) {
      break;
    }
    
  }
  
    
  printf("    elapsed: %lld.%06ld\n", (long long int)tv_delta.tv_sec, (long int)tv_delta.tv_usec);
}


void migrate() {
  char* pages[page_num];
  int nodes[page_num];
  for (int i = 0; i < page_num; i++) {
    pages[i] = char_arr + i*page_size;
    nodes[i] = migrated_node;
  }
  int status[page_num];
  move_pages(0, page_num, (void**)pages, nodes, status, MPOL_MF_MOVE);
  // for (int i = 0; i < page_num; i++) {
  //   printf("status %d: %d", i, status[i]);
  // }
}

void test_access() {
  if (sequential_only) {
    loop_access(0, 2);
  }
  else if (random_only) {
    loop_access(1, 2);
  }
  else {
    loop_access(0, 2);
    loop_access(1, 2);
  }
}

void parse_args(int argc, char** argv) {
  GetOpt parser(argc, argv);
  parser.require_arg_num(0);
  parser.add_doc("Usage: ./executable [option]");
  parser.add_doc("Description: allocate a set of pages on local node, access them; then call move_pages(), and access again\n"
                 "involved interfaces: move_pages()");
  parser.add_option("cpu-node", 'c', "specify which cpu node to run on", required_argument, &cpu_node, OptionArgType::INT);
  parser.add_option("mem-node", 'm', "specify which mem node to allocate on", required_argument, &mem_node, OptionArgType::INT);
  parser.add_option("migrated-to-node", 'g', "migrate pages to node [val]", required_argument, &migrated_node, OptionArgType::INT);
  parser.add_option("page-num", 'p', "specify number of pages to migrate", required_argument, &page_num, OptionArgType::INT);
  parser.add_option("timeout", 't', "run each test at least [val] seconds", required_argument, &timeout, OptionArgType::FLOAT);
  parser.add_option("sequential-only", 2, "only test sequential access", no_argument, &sequential_only, OptionArgType::BOOL);
  parser.add_option("random-only", 3, "only test random access", no_argument, &random_only, OptionArgType::BOOL);
  
  parser.parse();
}


int main(int argc, char** argv) {
  parse_args(argc, argv);
  bind_cpu(cpu_node);
  bind_mem(mem_node);
  alloc_pages();
  test_access();
  migrate();
  test_access();
  return 0; 
}
