## **C++实现字符串split功能**

### **概述**

众多比C++更高级的语言都实现了字符串的split功能，该功能在实际开发中非常重要。



### **代码**

* 目前我在开发过程中用到的方法，后面的方法涉及到高级特性

工作中用到的使用C函数的写法，用到了C语言的strtok函数，但是该函数不是线程安全，因此使用了线程安全版本strtok_r

```C++
#include <iostream>
#include <string.h>
#include <string>
#include <vector>
using namespace std;

void Split(const string&, const char*, vector<string>&);
int main(){
  string str = "hello, world!";
  vector<string> result;
  Split(str, " ", result);
  for(auto u: result){
    cout << u << endl;
  }
  return 0;
}

void Split(const string& str, const char* delim, vector<string>& result){
  char *pstr;
  char **saveptr;  // strtok_r内部使用，用来保存拆分之后的字符串
  size_t buflen = str.size()+1;
  pstr = new char[buflen];
  snprintf(pstr, buflen, "%s", str.c_str());  // 会将string类型的str格式化成char* 类型的pstr

  result.clear();
  char *tok = strtok_r(pstr, delim, saveptr);  // 使用C语言的strtok_r(它是strtok的线程安全版本)拆分pstr字符串
  while(tok){
    result.push_back(tok);
    tok = strtok_r(nullptr, delim, saveptr);  // 除第一次调用strtok_r使用pstr作为第一个参数外，后续的调用全部使用nullptr，因为上一次拆分之后的剩下的字符串保存在saveptr中
  }
}
~ 
```

工作中常用的第二种写法，也是比较优美的，没有用任何C语言特性

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

第三种工作中常用的写法

```C++
#include <iostream>
#include <string.h>
#include <string>
#include <vector>
using namespace std;

vector<string> Split(const string&, const char);
int main(){
  string str = "hello, world!";
  vector<string> result;
  result = Split(str, ' ');
  for(auto u: result){
    cout << u << endl;
  }
  return 0;
}

std::vector<std::string> Split(const std::string& str, const char delim) {
    std::vector<std::string> result;

    if (str.empty()) { return {}; }

    size_t l = -1;
    size_t r = -1;
    do {
        r = str.find(delim, r + 1);
        auto substr = str.substr(l + 1, r - l - 1);
        if (!substr.empty()){
            result.push_back(std::move(substr));
        }
        l = r;
    } while (r != std::string::npos);

    return result;
}
```

* 使用C++11提供的正则表达式实现.

这种写法下辈子见吧，记不住哇！

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
* 
这种写法下辈子见吧，记不住哇！

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



* 如果字符串为`char*`类型，在C语言中提供了strtok函数实现split.

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
