### 并发编程中的屏障


```c
// test_barrier.c
// gcc -O2 -c test_barrier.c && objdump -d test_barrier.o
extern int g;
void foo(int x){
     g++;
     // asm volatile("nop"::"r"(x) : "memory"); // complier barrier，汇编不能被编译器优化
     __sync_synchronize();  // memory barrier
     g++;
 }
 ```
