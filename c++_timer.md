## C++11 实现程序计时功能

```C++
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

int main(){
   // 程序其他逻辑
   
   {
       timers timer;  // 块变量，启动定时，当块语句结束之后，释放该变量
   }
}
```
