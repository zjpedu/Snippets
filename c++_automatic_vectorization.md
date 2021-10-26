```C++
#include <iostream>
using namespace std;
#define IMAX 4096


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
        std::cout << "Time-consuming:" << std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(m_end - m_begin).count() << std::endl;  // timers变量析构时自动输出时间
    }

private:
    std::chrono::high_resolution_clock::time_point m_begin;
    std::chrono::high_resolution_clock::time_point m_end;
};

void add(int*  z, int*  x, int*  y){
    for(int i = 0; i < IMAX; i++){
      z[i] = x[i] = y[i];
    }
}

// #pragma ivdep
void addVector(int* __restrict z, int* __restrict x, int* __restrict y){
    for(int i = 0; i < IMAX; i++){
      z[i] = x[i] = y[i];
    }
}

int main(){
  int z1[IMAX];
  int x1[IMAX];
  int y1[IMAX];
  
  int z2[IMAX];
  int x2[IMAX];
  int y2[IMAX];

  for(int i = 0 ; i < IMAX; i++){
    x1[i] = i + 1;
  }
  for(int i = 0; i < IMAX; i++){
    y1[i] = i * 2;
  }

  for(int i = 0; i < IMAX;i++){
    x2[i] = i+1;
  }
  for(int i = 0; i < IMAX; i++){
    y2[i] = i * 2;
  }

  {
    timers timer;
    add(z1, x1, y1);
  }
  {
    timers timer;
    addVector(z2, x2, y2);
  }
  return 0;
}

```

g++ test.cpp -std=c++11 -O3

## 参考资料

https://www.intel.com/content/www/us/en/develop/documentation/cpp-compiler-developer-guide-and-reference/top/optimization-and-programming-guide/vectorization/automatic-vectorization/using-automatic-vectorization.html
