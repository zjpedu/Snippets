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
int cpu_node = 0;
int mem_node = 0;
char* char_arr;
long long arr_size = 100 * 1024 * 1024;
//long long arr_size = 4000 * 1024;
float delay = 0; // seconds
float timeout = 0;
int loop_times = 1;
int test_cases = 4;
bool set_time_limit = false;
bool sequential_only = false;
bool random_only = false;


void* malloc_array(int size) {
  char_arr = (char*)malloc(size);
  if (char_arr == NULL) {
    fprintf(stderr, "malloc failed");
    exit(0);
  }
  return char_arr;
}

void* numa_alloc_array(int size, int node) {
  long long node_size = numa_node_size64(node, NULL);
  assert(arr_size < node_size && "requested array too big for the NUMA node");
  char_arr = (char*)numa_alloc_onnode(arr_size, node);
  if (char_arr == NULL) {
    fprintf(stderr, "numa_alloc_onnode failed");
    exit(0);
  }
  return char_arr;
}

void* numa_alloc_array_interleaved(int size) {
  char_arr = (char*)numa_alloc_interleaved(arr_size);
  if (char_arr == NULL) {
    fprintf(stderr, "numa_alloc_interleaved failed");
    exit(0);
  }
  return char_arr;
}

void sequential_access(int mode) {
  printf("  sequential access test\n");
  struct timeval tv_start, tv_end, tv_delta;  
  gettimeofday(&tv_start, NULL);
  srand(time(NULL));

  for (long long i = 0; i < arr_size; i++) {
    if (mode == 0) {
      int dummy = char_arr[i];
    }
    else if (mode == 1) {
      char_arr[i] = 0;
    }
    else if (mode == 2) {
      ++char_arr[i];
    }

    if (delay > 0) {
      usleep(delay*1000000);
    }
  }    
  
  gettimeofday(&tv_end, NULL);
  timersub(&tv_end, &tv_start, &tv_delta);
  printf("    elapsed: %lld.%06ld\n", (long long int)tv_delta.tv_sec, (long int)tv_delta.tv_usec);
}


void do_access(int pattern, int mode) {
  const char* patterns[2] = {"sequential", "random"};

  printf("  %s access test\n", patterns[pattern]);

  
  struct timeval tv_start, tv_end, tv_delta;  
  gettimeofday(&tv_start, NULL);
  srand(time(NULL));
  for (long long i = 0; i < arr_size; i++) {
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

    if (delay > 0) {
      usleep(delay*1000000);
    }
  }    
  
  gettimeofday(&tv_end, NULL);
  timersub(&tv_end, &tv_start, &tv_delta);
  printf("    elapsed: %lld.%06ld\n", (long long int)tv_delta.tv_sec, (long int)tv_delta.tv_usec);
}

void loop_access(int pattern, int mode) {
  if (timeout > 0) {
    set_time_limit = true;
  }
 
  const char* patterns[2] = {"sequential", "random"};

  printf("  %s access test\n", patterns[pattern]);

  
  struct timeval tv_start, tv_end, tv_delta;  
  gettimeofday(&tv_start, NULL);
  srand(time(NULL));

  int cnt = 0;
  while (1) {
    int page_size = 1024 * 1024;
    for (long long i = 0; i < arr_size/page_size-2; i++) {
      memcpy(char_arr + page_size*(i+1), char_arr + page_size*i, page_size);
      //memcpy(char_arr +1, char_arr, 1);
    }
    ++cnt;
    if (!set_time_limit) {
      if (cnt == loop_times) {
        break;
      }
    }
    else {
      gettimeofday(&tv_end, NULL);
      timersub(&tv_end, &tv_start, &tv_delta);

      if (tv_delta.tv_sec > timeout) {
        break;
      }
    }  
    
  }

  gettimeofday(&tv_end, NULL);
  timersub(&tv_end, &tv_start, &tv_delta);
  printf("    elapsed: %lld.%06ld\n", (long long int)tv_delta.tv_sec, (long int)tv_delta.tv_usec);
}

// void test_numa(int acc_mode) {
//   int node = 0;
//   int ret = numa_run_on_node(node); // runs the current task and its children on node 0
//   if (ret != 0) {
//     fprintf(stderr, "numa_run_on_node failed");
//     exit(0);
//   }

//   int M = 1024 * 1024;
//   long long freep;
//   for (int i = 0; i < numa_num_configured_nodes(); i++) {
//     long long node_size = numa_node_size64(i, &freep);
//     assert(arr_size < node_size && "requested array too big for the NUMA node");
//     //long long arr_size = node_size/10;
//     char* arr = (char*)numa_alloc_onnode(arr_size, i);
//     if (arr == NULL) {
//       fprintf(stderr, "numa_alloc_onnode failed");
//       exit(0);
//     }

//     struct timeval tv_start, tv_end, tv_delta;
  
//     gettimeofday(&tv_start, NULL);

//     srand (time(NULL));
//     for (long long i = 0; i < arr_size; i++) {
//       switch (acc_mode) {
//       case 0: {
//         arr[i] = 0;
//       }
//       case 1: {
//         int j = rand() % arr_size;
//         arr[j] = 'a';
//       }
//       }
      
//     }    
  
//     gettimeofday(&tv_end, NULL);
//     timersub(&tv_end, &tv_start, &tv_delta);
//     printf("Time elapsed: %lld.%06ld\n", (long long int)tv_delta.tv_sec, (long int)tv_delta.tv_usec);

//     numa_free(arr, arr_size);

//   }
  
// }


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

void test_malloc() {
  if (test_cases < 1) {
    return;
  }
  printf("allocate by malloc\n");
  malloc_array(arr_size);

  test_access();
}

void test_remote() {
  for (int i = 0; i < numa_num_configured_nodes(); i++) {
    if (test_cases < 2+i) {
      return;
    }

    if (i == cpu_node) {
      printf("allocate on local node %d\n", i);
    }
    else {
      printf("allocate on remote node %d\n", i);
    }
    
    numa_alloc_array(arr_size, i);

    test_access();
  }
}

void test_interleaved() {
  printf("allocate interleavely on all nodes\n");
  numa_alloc_array_interleaved(arr_size);

  test_access();
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

void bind_cpu_mem(int node) {
  bitmask* m = new bitmask;
  m->size = numa_num_task_nodes();
  m->maskp = new unsigned long;
  *m->maskp = 0;
  *m->maskp |= 1 << node;
  
  numa_bind(m);
  // numa_run_on_node_mask(m);
  // numa_set_membind(m);

  // int node_num = numa_num_task_nodes();
  // printf("number of nodes that the calling task is allowed to use: %lu\n", node_num);

  printf("bind the current task to run and allocate on node %d\n", node);
}

void parse_args(int argc, char** argv) {
  GetOpt parser(argc, argv);
  parser.require_arg_num(0);
  parser.add_doc("Usage: ./executable [option]");
  parser.add_doc("Description: bind the process to one cpu node, and test accessing array from the local node, remote node or interleavely.\n"
                 "involved interfaces: numa_run_on_node(), numa_alloc_onnode()");
  parser.add_option("cpunodebind", 'c', "specify which cpu node to run on", required_argument, &cpu_node, OptionArgType::INT);
  parser.add_option("membind", 'm', "specify which mem node to allocate on", required_argument, &mem_node, OptionArgType::INT);
  parser.add_option("array-size", 's', "specify the size of the array (in Mb)", required_argument, &arr_size, OptionArgType::INT);
  parser.add_option("loop-times", 'l', "loop each test [val] times", required_argument, &loop_times, OptionArgType::INT);
  parser.add_option("test-case", 'e', "specify what test to run", required_argument, &test_cases, OptionArgType::INT);
  parser.add_option("delay", 'd', "specify the delay after each access", required_argument, &delay, OptionArgType::FLOAT);
  parser.add_option("timeout", 't', "run each test for about [val] seconds", required_argument, &timeout, OptionArgType::FLOAT);
  parser.add_option("use-time-limit", 4, "run each test for a specified time to monitor bandwidth, in stead of N times, which is used for comparison and is the default case", no_argument, &set_time_limit, OptionArgType::BOOL);
  parser.add_option("sequential-only", 2, "only test sequential access", no_argument, &sequential_only, OptionArgType::BOOL);
  parser.add_option("random-only", 3, "only test random access", no_argument, &random_only, OptionArgType::BOOL);
  parser.parse();

  
}

int main(int argc, char** argv) {
  parse_args(argc, argv);
  bind_cpu_mem(cpu_node);
  bind_cpu(cpu_node);
  bind_mem(mem_node);
  test_malloc();
  test_remote();
  test_interleaved();
  return 0;
}
