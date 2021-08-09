## **C++实现trim功能**

### **概述**

在很多更为高级的语言如Python等都实现了trim功能，C++用户只能自己实现该功能。

### **代码**

```C++
void trim(string &s) {
	if (s.empty()) {
		return;
	}
	s.erase(0,s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
}
```