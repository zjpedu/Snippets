#include <numa.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>

void bind_cpu_mem();
const int size = 10;

int fork_process();

int main(){
    char* char_arr = (char *)malloc(size);
    for(int i = 0; i < 5; ++i) fork_process();
    int status = 0;
    wait(&status);
    return 0;
}

int fork_process(){
    bind_cpu_mem();
    int pid = fork();
    return pid;
}

void bind_cpu_mem(){
	int nnodes = numa_num_task_nodes();
    int ncpus = numa_num_configured_cpus();
    // printf("%d\n", ncpus); 
    /* random select node */
    // srand((unsigned)time(NULL));
    int node = rand() % ncpus;
    /* bind task to cpu_node */
    int ret = numa_run_on_node(node % nnodes);
    assert(ret == 0);
    
    /* bind mem node */
    bitmask* bm = (bitmask*)malloc(sizeof(bitmask));
    bm->size = numa_num_task_nodes();
    bm->maskp = (unsigned long *)malloc(sizeof(unsigned long));
    *bm->maskp = 0;
    /* bind mem node */
    *bm->maskp |= 1 << (node % nnodes);
    numa_set_membind(bm);
    printf("cpu cores: %d of nodes %d\n", node, (node % nnodes));
}
