### 高效开发工具篇

最近在做大型分布式 MPP 数据库 Greenplum 的优化工作，早期调试程序习惯了使用 gdb （因为程序都很小，gdb 已经足够了），但是在调试 Greenplum 这样的庞然大物时发现 gdb 对于我而言（请注意是对于我这个**菜鸡**而言）不是很完美，于是想选择一些集成开发工具，本文主要摸索了两个集成开发环境，它们分别是 Clion 和 VSCode Insider，接下来将给出一些详细的配置方法以便参考。

#### Clion 篇

Clion 是付费软件，学生可以享受学生版。本文给出了一种其它方法，不鼓励你使用这种方法。

* 首先，如果你安装了 CLion， 先完全卸载，重新安装

```shell
cd /Users/xxx/Library/ # 请首先进入到该目录
cd Logs # 在上面目录下找到 Logs 目录，进入后删除其中与 Jetbrains 有关的文件与文件夹
cd ..
cd Perferences # 再该目录下删除与 Jetbrains 有关的所有问价与文件夹
cd ..
cd Application\ Support # 在该目录下删除与 Jetbrains 有关的所有文件与文件夹
cd ..
cd Cachesn # 进入到该目录，删除与 Jetbrains 有关的所有文件与文件夹
```

* 下载你喜欢的版本的 CLion

https://www.jetbrains.com/clion/download/other.html

* 破解 Clion

https://www.macwk.com/article/jetbrains-crack

破解插件下载链接: https://pan.baidu.com/s/1HCSMhWzOR_DPMAYVKEFAZQ 提取码: 6aga

#### VSCode 篇
  
```json
{
            "name": "(lldb) Attach",
            "type": "cppdbg",
            "request": "attach",
            "program": "/Users/admin/matrix/bin/postgres",  # 这里是你的主进程 postgres 所在的位置，，一定要有该文件路径
            "processId": "${command:pickProcess}",
            "MIMode": "lldb",
            "targetArchitecture": "x86_64"  # 指定了目标架构，否则会报 Warning
        },
```

#### CLion activation code

不建议用下面的方法破解 clion
Clion Key link: https://jetbra.in/test-7b06e3ddfa68ff9b483adfcd08cdd243831f7928.html?t=16018385822&__cf_chl_jschl_tk__=pmd_kkCMoEn9xwQxhrFDmjjPNlYTXGk9Kj.Rk0rC6.Nt4Nc-1634874565-0-gqNtZGzNAqWjcnBszQgR


