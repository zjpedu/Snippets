## **C++实现字符串split功能**

### **概述**

众多比C++更高级的语言都实现了字符串的split功能，该功能在实际开发中非常重要。



### **代码**

* 目前我再开发过程中用到的方法，后面的方法涉及到高级特性

```C++
std::string s = "hello world split";
std::string delimiter = " ";

size_t pos = 0;
std::string token;
while ((pos = s.find(delimiter)) != std::string::npos) {
    token = s.substr(0, pos);
    std::cout << token << std::endl;
    s.erase(0, pos + delimiter.length());
}
std::cout << s << std::endl;
```

具体使用实例参见 https://github.com/Ethanzjp/Algorithms/blob/master/Traditional-Algorithms/LeetCode297.cpp

* 使用C++11提供的正则表达式实现.

```c++
std::string text = "hello world!";
std::regex sp("\\s+");
std::vector<std::string> v(std::sregex_token_iterator(text.begin(), text.end(), sp, -1), std::sregex_token_iterator());
for(auto& s: v){
    std::cout << s << "\n";
}
```

上述实现的问题是存在大量的内存拷贝，效率低下。

* C++17以后使用views实现.

```c++
string str("hello wordld");
auto sv = str
          | ranges::viewssplit(' ')
          | ranges::views::transform([](auto&& i){
          		return i | ranges::to<string>();
          })
          | ranges::to<vector>();
for(auto&& s: sv){
    std::cout << s << "\n";
}
```



如果字符串为`char*`类型，在C语言中提供了strtok函数实现split.

```c++
#include <string.h>
#include <iostream>
#include <string>
using namespace std;

int main(){
    string str = "hello world";
    char* token = strtok(str.data(), " ");
    while(token != nullptr){
        cout << token << "\n";
        token = strtok(nullptr, " ");
    }
}
```

参考文献： https://stackoverflow.com/questions/26328793/how-to-split-string-with-delimiter-using-c
