#include <numa.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <sys/types.h>

void bind_cpu_mem();

char *char_arr;

int main(){
    bind_cpu_mem();
    int pid = fork();
    return 0;
}

void bind_cpu_mem(){
	int nnodes = numa_num_task_nodes();
    int ncpus = numa_num_configured_cpus();
    printf("%d\n", ncpus); 
    /* random select node */
    int node = rand() % nnodes;
    /* bind task to cpu_node */
    int ret = numa_run_on_node(node);
    assert(ret == 0);
    
    /* bind mem node */
    bitmask* bm = (bitmask*)malloc(sizeof(bitmask));
    bm->size = numa_num_task_nodes();
    bm->maskp = (unsigned long *)malloc(sizeof(unsigned long));
    *bm->maskp = 0;
    /* bind mem node */
    *bm->maskp |= 1 << node;
    numa_set_interleave_mask(bm);
}
