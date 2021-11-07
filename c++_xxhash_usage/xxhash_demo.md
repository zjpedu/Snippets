## 使用示例

* 下载 xxhash 头文件
https://create.stephan-brumme.com/xxhash/

* 在代码中使用 xxhash

```c++
// 将代码保存为 test.cpp
#include "xxhash64.h"
#include <iostream>
#include <stdint.h>
#include <string>
using namespace std;
int main(){
    string a = "hahahahaha";
    uint64_t result1 = XXHash64::hash(&a, a.size(), 0);
    std::cout << result1 << std::endl;

    string b = "hahahahaha";
    uint64_t result2 = XXHash64::hash(&b, b.size(), 0);
    std::cout << result2 << std::endl;
}
```

* 编译
```shell
g++ -o test test.cpp
```

* 运行结果
```shell
594230086268525197
594230086268525197
```

我们可以看到计算出上述两个字符串的 hash 值，在大量应用中可以使用该值分桶。
