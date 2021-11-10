## 高效实现除法运算

```c++
// 文件保存为 slow.cc
// g++ -o test slow.cc -std=c++11 -march=native -O3
#include <stdlib.h> 
#include <vector>
#include <chrono>
#include "libdivide.h"
using namespace std;

const int N = 1e7;
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

void slow_div(vector<int> &xs, int y, vector<int>& zs){
    
    for (auto i = 0; i < 32; ++i)
        zs[i] = xs[i] / y;
}

void fast_div(vector<int> &xs, libdivide::divider<int> y, vector<int>& zs){
    for (auto i = 0; i < 32; ++i)
        zs[i] = xs[i] / y;
}

void faster_div(vector<int>& xs, vector<int>& zs){
    constexpr int y = 1000000;
    for (auto i = 0; i <32; ++i)
        zs[i] = xs[i] / y;
}

int main(void)
{
    vector<int> xs(N);
    srand((unsigned)time(nullptr));
    for(int i = 0; i < 32; i++){
        // 随机生成 [1000, 100000] 之间的数字
        xs.push_back(rand() % (100000 - 1000 + 1) + 1000);
    }
    vector<int> zs(32);
    {
        timers timer;
        int y = atoi("1000000");
        for(int i = 0; i < N; i++)
                slow_div(xs, y, zs);
    }
    {
        timers timer;
        int divisor = 1000000;
        libdivide::divider<int> fast_d(divisor);
        
        for(int i = 0; i < N; i++)
            fast_div(xs, fast_d, zs);
    }
    {
        timers timer;
        for(int i = 0; i < N; i++)
                faster_div(xs, zs);
    }

}
```
执行结果：

```shell
Time-consuming:557.441
Time-consuming:179.02
Time-consuming:161.884
```
