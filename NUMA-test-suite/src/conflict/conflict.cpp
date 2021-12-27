
#include <numa.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <sys/time.h>
#include <iostream>

#include "../share/optParser.h"

/* definition of bitmask */
// struct bitmask {
// 	unsigned long size; /* number of bits in the map */
// 	unsigned long *maskp;
// };


/* config */
int bind_node = 0;
int cpu_node = 0;
int mem_node = 0;
char* char_arr;
long long arr_size = 20 * 1024 * 1024;


void test_numa_bind(int node) {
  bitmask* m = new bitmask;
  m->size = numa_num_task_nodes();
  m->maskp = new unsigned long;
  *m->maskp = 0;
  *m->maskp |= 1 << node;
  
  numa_bind(m);
  // numa_run_on_node_mask(m);
  // numa_set_membind(m);

  int node_num = numa_num_task_nodes();
  printf("number of nodes that the calling task is allowed to use: %lu\n", node_num);

  char_arr = (char*)numa_alloc_onnode(arr_size, 1);
  if (char_arr == NULL) {
    fprintf(stderr, "numa_alloc_onnode failed");
    exit(0);
  }
}

int main(int argc, char** argv) {
  test_numa_bind(bind_node);
  return 0;
}
