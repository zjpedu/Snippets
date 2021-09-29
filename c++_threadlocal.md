### c++11 thread_local 变量生命周期管理

#### 概述

threa_local 是 C++11 中新增的存储生命周期关键字，属于线程级别生命周期。

#### 代码示例

```C++
// 将下面代码保存为 test.cpp
#include <iostream>
#include <thread>
using namespace std;

thread_local int i=0;

void f(int newval){
    i=newval;
}

void g(){
    std::cout<<i;
}

void threadfunc(int id){
    g(); // 尽管在 main 线程中将 i 设置为 9，但是在t1 t2 t3 这三个线程中，i 的初始值仍然为 thread_local int i = 0 处设置的 0.
    f(id);
    ++i;
    g();
}

int main(){
    i=9;
    std::thread t1(threadfunc,1);
    std::thread t2(threadfunc,2);
    std::thread t3(threadfunc,3);

    t1.join();
    t2.join();
    t3.join();
    std::cout<<i<<std::endl;
}
```

使用 g++-9 编译

```shell
g++ -o test test.cpp -lpthread
```

可能的运行结果：
0203049、0204039、0302049、0304029、0402039、0403029

只可能为上述 6 种运行结果，这说明尽管在 main 线程中将 i 的值设置为 9，但是除 main 之外的其它线程仍然将 i 设置为 0.

### 参考资料 

https://stackoverflow.com/questions/11983875/what-does-the-thread-local-mean-in-c11
