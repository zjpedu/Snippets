### 概述

通过代码判断计算机是 big endian 还是 little endian。

### 判断方法

**方法一**

```python
import sys
print(sys.byteorder);  
```

**方法二**

```C++
// test.cc
// g++ -o test test.cc
#include <iostream>
using namespace std;

int main(){
  int i = 0x11223344;
  char* p = (char*)&i;
  if(*p == 0x44){
    cout << "little endian" << endl;
  }else{
    cout << "big endian" << endl;
  }
}
```
