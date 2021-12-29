// g++ -o test numatest.cc  -std=c++11 -lpthread -lnuma
#include <numa.h>
#include <iostream>
#include <thread>
#include <pthread.h>
#include <boost/date_time/posix_time/posix_time.hpp>


void pin_to_core(size_t core)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

std::ostream& operator<<(std::ostream& os, const bitmask& bm)
{
    for(size_t i=0;i<bm.size;++i)
    {
        os << numa_bitmask_isbitset(&bm, i);
    }
    return os;
}

void* thread1(void** x, size_t core, size_t N, size_t M)
{
    pin_to_core(core);

    void* y = numa_alloc_local(N);

    boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::universal_time();
    
    char c;
    for (size_t i(0);i<M;++i)
        for(size_t j(0);j<N;++j)
        {
            *(((char*)y) + ((j * 1009) % N)) += 1;
        }

    boost::posix_time::ptime t2 = boost::posix_time::microsec_clock::universal_time();

    std::cout << "Elapsed read/write by same thread that allocated on core " << core << ": " << (t2 - t1) << std::endl;

    *x = y;
}

void thread2(void* x, size_t core, size_t N, size_t M)
{
    pin_to_core(core);

    boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::universal_time();

    char c;
    for (size_t i(0);i<M;++i)
        for(size_t j(0);j<N;++j)
        {
            *(((char*)x) + ((j * 1009) % N)) += 1;
        }

    boost::posix_time::ptime t2 = boost::posix_time::microsec_clock::universal_time();
    
    std::cout << "Elapsed read/write by thread on core " << core << ": " << (t2 - t1) << std::endl;
}

int main(int argc, const char **argv)
{
    int numcpus = numa_num_task_cpus();
    std::cout << "numa_available() " << numa_available() << std::endl;
    numa_set_localalloc();

    bitmask* bm = numa_bitmask_alloc(numcpus);
    for (int i=0;i<=numa_max_node();++i)
    {
        numa_node_to_cpus(i, bm);
        std::cout << "numa node " << i << " " << *bm << " " << numa_node_size(i, 0) << std::endl;
    }
    numa_bitmask_free(bm);

    void* x;
    size_t N(100000000);
    size_t M(50);

    std::thread t1(thread1, &x, 0, N, M);
    t1.join();

    for (size_t i(0);i<numcpus;++i)
    {
        std::thread t2(thread2, x, i, N, M);
        t2.join();
    }

    numa_free(x, N);

    return 0;
}