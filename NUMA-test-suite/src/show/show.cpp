#include <numa.h>
#include <cstdio>
#include <iostream>
#include "../share/optParser.h"

// bitmask (defined in numa.h)

// struct bitmask {
// 	unsigned long size; /* number of bits in the map */
// 	unsigned long *maskp;
// };



// The bitmask is allocated by a call to numa_allocate_nodemask() using size numa_max_possible_node().
// The set of nodes to record is derived from /proc/self/status, field "Mems_allowed". The user should not alter this bitmask.

void show_numa_nodes() {
  int num_possible = numa_num_possible_nodes();
  printf("number of possible nodes: %d\n", num_possible);
  printf("  - the maximum number of nodes that the kernel can handle\n");

  int num_configured = numa_num_configured_nodes();
  printf("number of configured nodes: %d\n", num_configured);
  printf("  - number of memory nodes in the system\n");
  printf("  - derived from the node numbers in /sys/devices/system/node\n");

  int num_cpus = numa_num_configured_cpus();
  printf("number of cpus in the system: %d\n", num_cpus);
  printf("  - includes any cpus that are currently disabled\n");
  printf("  - derived from the cpu numbers in /sys/devices/system/cpu\n");

  bitmask* mask_allowed = numa_get_mems_allowed();
  // printf("len: %lld", mask_allowed->size); // 1024
  printf("allowed memory mask: %lu\n", *(mask_allowed->maskp));
  printf("  - the mask of nodes from which the process is allowed to allocate memory in it's current cpuset context\n");

  int cpu_num = numa_num_task_cpus();
  printf("number of cpus that the calling task is allowed to use: %lu\n", cpu_num);

  int node_num = numa_num_task_nodes();
  printf("number of nodes that the calling task is allowed to use: %lu\n", node_num);

  printf("node of cpu:\n");
  int range[2];
  range[0] = 0;
  int start = 0;
  int pre_id = -1;
  int node_id;
  for (int i = 0; i < cpu_num; i++) {
    node_id = numa_node_of_cpu(i);
    if (node_id != pre_id) {
      if (pre_id >= 0) {
        printf("  cpu %d-%d: %d\n", start, i-1, pre_id);
      }
      
      pre_id = node_id;
      start = i;
    }
    //printf("  cpu %d: %d\n", i, );
  }
  printf("  cpu %d-%d: %d\n", start, cpu_num-1, node_id);

  int node = 0;
  long long freep = 0;
  int G = 1024 * 1024 * 1024;
  for (int i = 0; i < num_configured; i++) {
    long long node_size = numa_node_size64(i, &freep);
    printf("node %d size: %.3fG, free: %.3fG\n", i, (double)node_size/G, (double)freep/G);
  }  
}

void parse_args(int argc, char** argv) {
  GetOpt parser(argc, argv);
  parser.require_arg_num(0);
  parser.add_doc("Usage: ./executable");
  parser.add_doc("Description: print basic NUMA configuration in the system.\n"
                 "involved interfaces: numa_num_configured_nodes(), etc");
  parser.parse();
}

int main(int argc, char** argv) {
  parse_args(argc, argv);
  show_numa_nodes();
  return 0;
}
