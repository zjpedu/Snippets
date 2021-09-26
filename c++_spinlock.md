## spin lock 实现方法

### 概述

spin lock 在存储系统中有非常广泛地应用，如何高效实现 spin lock 是本文的重点。本文采用 atomic 原子操作来实现，相对使用 mutex 等锁的方式拥有更好得性能。

### 使用 atomic_flag 实现 spin lock

 ```C++
// 将上述代码保存为 test.cpp
#include <thread>
#include <vector>
#include <iostream>
#include <atomic>

std::atomic_flag lock = ATOMIC_FLAG_INIT;  // 初始化时表示当前 lock 处于 clear 状态
int sum = 0;
void f(int n)
{
    for (int cnt = 0; cnt < 100; ++cnt) {
        while (lock.test_and_set(std::memory_order_acquire))  // acquire lock
             ; // spin
        // std::cout << "Output from thread " << n << '\n';
        sum++;
        lock.clear(std::memory_order_release);               // release lock
    }
}

int main()
{
    std::vector<std::thread> v;
    for (int n = 0; n < 10; ++n) {
        v.emplace_back(f, n);
    }
    for (auto& t : v) {
        t.join();
    }
    std::cout << "sum = " << sum << std::endl;
}
 ```
 
在上面的程序中，std::atomic_flag 对象 lock 的上锁操作可以理解为 lock.test_and_set(std::memory_order_acquire); 解锁操作相当与 lock.clear(std::memory_order_release)。

在上锁的时候，如果 lock.test_and_set 返回 false，则表示上锁成功（此时 while 不会进入自旋状态），因为此前 lock 的标志位为 false(即没有线程对 lock 进行上锁操作)，但调用 test_and_set 后 lock 的标志位为 true，说明某一线程已经成功获得了 lock 锁。

如果在该线程解锁（即调用 lock.clear(std::memory_order_release)） 之前，另外一个线程也调用 lock.test_and_set(std::memory_order_acquire) 试图获得锁，则 test_and_set(std::memory_order_acquire) 返回 true，则 while 进入自旋状态。如果获得锁的线程解锁（即调用了 lock.clear(std::memory_order_release)）之后，某个线程试图调用 lock.test_and_set(std::memory_order_acquire) 并且返回 false，则 while 不会进入自旋，此时表明该线程成功地获得了锁。

### 使用 atomic 实现 spin lock

```c++
// 将上述代码保存为 test.cpp
#include <thread>
#include <vector>
#include <iostream>
#include <atomic>

std::atomic<bool> lock(false);  // 初始化为 false
int sum = 0;
void f(int n)
{
    for (int cnt = 0; cnt < 100; ++cnt) {
        while (lock.exchange(true, std::memory_order_consume))  // acquire lock
             ; // spin
        // std::cout << "Output from thread " << n << '\n';
        sum++;
        lock.exchange(false, std::memory_order_release);       // release lock
    }
}

int main()
{
    std::vector<std::thread> v;
    for (int n = 0; n < 10; ++n) {
        v.emplace_back(f, n);
    }
    for (auto& t : v) {
        t.join();
    }
    std::cout << "sum = " << sum << std::endl; 
}
```

注意在上面的程序中 while (lock.exchange(true, std::memory_order_consume)) 使用了 memory_order_consume 内存顺序，该内存顺序规定了本线程中其它对 lock 的操作都必须在当前操作之后执行，编译器不能任意调换执行顺序。lock.exchange(false, std::memory_order_release) 使用了 memory_order_release  内存顺序，该内存规定了本线程中其它对 lock 的写操作都将先完成之后该修改操作才会做。当然其它内存顺序也能保证 spin lock 实现的正确性，但是我认为上述代码的内存顺序最能表达 spin lock 的语义。

### 编译与测试

安装 g++9

```shell
yum install centos-release-scl
yum-config-manager --enable rhel-server-rhscl-9-rpms
yum install devtoolset-9
scl enable devtoolset-9 bash

yum install devtoolset-9-gcc-c++
source /opt/rh/devtoolset-9/enable
ccache -C   #对于找不到ccache命令这种错误，说明相关模块没有安装，先安装ccache

```

编译 test.cpp

```shell
g++ -o test test.cpp -lpthread
```

使用下列脚本测试上述 spin lock 实现的正确性

```shell
#!/bin/bash
echo > output
for varible1 in {1..1000}
do
  ./test >> output
done
```

### pthread 库中包含的锁

* pthread_mutex_t mutex_
    * pthread_mutex_init(&mutex_, nullptr)
    * pthread_mutex_destroy(&mutex_)
    * pthread_mutex_lock(&mutex_)
    * pthread_mutex_unlock(&mutex_)
    * pthread_mutex_trylock(&mutex_)
* pthread_spinlock_t spin_
    * pthread_spin_init(&spin_, PTHREAD_PROCESS_PRIVATE)
    * pthread_spin_lock(&spin_)
    * pthread_spin_unlock(&spin_)
    * pthread_spin_trylock(&spin_)
* pthread_rwlock_t rwl_
    * pthread_rwlock_init(&rwl_, nullptr)
    * pthread_rwlock_rdlock(&rwl_)
    * pthread_rwlock_wrlock(&rwl_)
    * pthread_rwlock_unlock(&rwl_)
    * pthread_rwlock_tryrdlock(&rwl_)
    * pthread_rwlock_trywrlock(&rwl_)  
实现参考 https://github.com/Ethanzjp/Algorithms/tree/master/Concurrency/Thread-Parallel-Programming/criticalSection
