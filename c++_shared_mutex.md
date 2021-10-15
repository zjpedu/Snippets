## C++ shared_mutex 使用

官方 API 文档中这样说道：
Shared mutexes are especially useful when shared data can be safely read by any number of threads simultaneously, but a thread may only write the same data when no other thread is reading or writing at the same time.
The shared_mutex class satisfies all requirements of SharedMutex and StandardLayoutType.

因此，可以将 shared_mutex 作为 lockable 对象，并结合 shared_lock、unique_lock、lock_guard 实现读写锁。

* shared_lock通常是read lock。被锁后仍允许其他线程执行同样被 shared_lock 的代码。
* lock_guard 和 unique_lock是write lock。被锁后不允许其他线程执行被shared_lock或unique_lock的代码。


### 代码实现

```C++
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <list>
#include <iostream>
#include <vector>

#define READ_THREAD_COUNT 8  
#define LOOP_COUNT 5000000  

typedef std::shared_lock<std::shared_mutex> ReadLock;
typedef std::unique_lock<std::shared_mutex> WriteLock;
typedef std::lock_guard<std::mutex> NormalLock;

class shared_mutex_counter {
public:
    shared_mutex_counter() = default;

    unsigned int get() const {
        ReadLock lock(mutex);
        return value;
    }

    void increment() {
        WriteLock lock(mutex);
        value++;
    }

private:
    mutable std::shared_mutex mutex;
    unsigned int value = 0;
};

class mutex_counter {
public:
    mutex_counter() = default;

    unsigned int get() const {
        NormalLock lock(mutex);
        return value;
    }

    void increment() {
        NormalLock lock(mutex);
        value++;
    }

private:
    mutable std::mutex mutex;
    unsigned int value = 0;
};

class timers
{
public:
    timers()
    {
        m_begin = std::chrono::high_resolution_clock::now();
    }

    ~timers()
    {
        m_end = std::chrono::high_resolution_clock::now();
        Consuming();
    }

    void Consuming()
    {
        std::cout << "Time-consuming:" << std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(m_end - m_begin).count() << std::endl;
    }

private:
    std::chrono::high_resolution_clock::time_point m_begin;
    std::chrono::high_resolution_clock::time_point m_end;
};


void test_shared_mutex()
{
    shared_mutex_counter counter;
    unsigned int temp;

    auto writer = [&counter]() {
        for (unsigned int i = 0; i < LOOP_COUNT; i++){
            counter.increment();
        }
    };

    auto reader = [&counter, &temp]() {
        for (unsigned int i = 0; i < LOOP_COUNT; i++) {
            temp = counter.get();
        }
    };

    std::cout << "----- shared mutex test ------" << std::endl;
    std::list<std::shared_ptr<std::thread>> threadlist;
    {
        timers timer;

        for (int i = 0; i < READ_THREAD_COUNT; i++)
        {
            threadlist.push_back(std::make_shared<std::thread>(reader));
        }
        std::shared_ptr<std::thread> pw = std::make_shared<std::thread>(writer);

        for (auto &it : threadlist)
        {
            it->join();
        }
        pw->join();
    }
    std::cout <<"count:"<< counter.get() << ", temp:" << temp << std::endl;
}

void test_mutex()
{
    mutex_counter counter;
    unsigned int temp;

    auto writer = [&counter]() {
        for (unsigned int i = 0; i < LOOP_COUNT; i++) {
            counter.increment();
        }
    };

    auto reader = [&counter, &temp]() {
        for (unsigned int i = 0; i < LOOP_COUNT; i++) {
            temp = counter.get();
        }
    };

    std::cout << "----- mutex test ------" << std::endl;
    std::list<std::shared_ptr<std::thread>> threadlist;
    {
        timers timer;

        for (int i = 0; i < READ_THREAD_COUNT; i++)
        {
            threadlist.push_back(std::make_shared<std::thread>(reader));
        }

        std::shared_ptr<std::thread> pw = std::make_shared<std::thread>(writer);

        for (auto &it : threadlist)
        {
            it->join();
        }
        pw->join();
    }
    std::cout << "count:" << counter.get() << ", temp:" << temp << std::endl;
}



int main()
{
    test_shared_mutex();
    test_mutex();
    return 0;
}
```

### 参考文献

API 文档 https://en.cppreference.com/w/cpp/thread/shared_mutex
