### 高效开发工具篇

最近在做大型分布式 MPP 数据库 Greenplum 的优化工作，早期调试程序习惯了使用 gdb + vim，但是在调试 Greenplum 这样的庞然大物时发现 gdb 对于我而言（请注意是对于我这个**菜鸡**而言）不是很完美，上古时代的程序员会选择使用 ctag 之类插件生成各种符号链接，但是对于我而言，还是不友好，经常调试着就发忘记了什么。于是自己琢磨选择一款集成开发测试工具，早期用过 Eclipse CDT，不过现在感觉这个系列的颜值比较低，慎重选择了 CLion 和 Visual Studio Code Insider。在我周边各式大佬中，这两种 IDE 用的最多，CLion 能够自动创建 CMake 工程，相对更加简洁友好，后续我在 CLion 中集成了 IDEAVIM 插件，保留了 vim 强大的编辑功能。Visual Studio Code 也非常强大，在远程连接服务器集群开发方面有独特的优势，本文将详细说明两个软件的使用方法。

#### Clion 篇

Clion 是付费软件，学生可以享受免费 License。

* 首先，如果你安装了 CLion， 先完全卸载，重新安装

```shell
# 在这里推荐使用 https://freemacsoft.net/appcleaner/ 卸载软件，后续手动删除下述文件夹 
cd /Users/xxx/Library/ # 请首先进入到该目录
cd Logs # 在上面目录下找到 Logs 目录，进入后删除其中与 Jetbrains 有关的文件与文件夹
cd ..
cd Preferences # 再该目录下删除与 Jetbrains 有关的所有文件与文件夹
cd ..
cd Application\ Support # 在该目录下删除与 Jetbrains 有关的所有文件与文件夹
cd ..
cd Caches # 进入到该目录，删除与 Jetbrains 有关的所有文件与文件夹
```

* 下载你喜欢的版本的 CLion (推荐 CLion 2021.2.2 以下版本)

https://www.jetbrains.com/clion/download/other.html

* 试试这个办法

https://www.macwk.com/article/jetbrains-crack 如果不能下载，下面链接有可以用的版本。**请注意，只有 2021.2.2 (包括) 以下的版本才能使用 reset 方法，其它版本测试好像不行**。

插件下载链接: https://pan.baidu.com/s/1HCSMhWzOR_DPMAYVKEFAZQ 提取码: 6aga

配置好 CLion 之后，导入你自己的工程，对于非常庞大的工程，你一定希望能够自动生成 CMakeLists.txt 文件，很简单，如下图：

<img width="1297" alt="Screenshot 2021-11-16 at 17 15 31" src="https://user-images.githubusercontent.com/13810907/141957024-997e1381-db92-4230-b97f-7c44d00937c3.png">

* 如果你的工程中包含一个不完整的 CMakeLists.txt 文件，那么你首先删除它；
* 接着按照图上的步骤点击 “Unload CMake Project”，点开随便一个文件，它会提示你为是否为项目生成 CMakeLists.txt 文件，此时选择生成会跳出选择源码文件的界面，选择即可.

在我的 demo 中 CLion 为我生成了如下 CMakeLists.txt 文件：

```shell
cmake_minimum_required(VERSION 3.20)
project(gendata)

set(CMAKE_CXX_STANDARD 14)
include_directories(/usr/local/include)
add_executable(gendata
        gendata.cpp)
```

在大型工程中往往会有很多的系统依赖库，为了给出场景，我故意制造了下图所示的情况:

<img width="336" alt="Screenshot 2021-11-16 at 17 20 09" src="https://user-images.githubusercontent.com/13810907/141957672-95776d7e-bea9-4f66-9239-f6fad71ec4f9.png">

看到缺少库文件 <parquet/types.h>，在 bash 中执行

```shell
$ pkg-config --cflags parquet # 比如回写 -I/usr/local/include
```

说明我的 parquet 在 /usr/local/include 目录下，于是我需要在我的 CMakeLists.txt 中增加下面选项：

```shell
cmake_minimum_required(VERSION 3.17)
project(gendata)

set(CMAKE_CXX_STANDARD 14)
include_directories(/usr/local/include)  #新增加的系统库位置
add_executable(gendata
        gendata.cpp)
```

接着按照下图方式 reload 的我 CMake 工程，发现错误消失，**当你缺少其他文件时，请尝试这样解决！**

<img width="606" alt="Screenshot 2021-11-16 at 17 27 05" src="https://user-images.githubusercontent.com/13810907/141958732-d8fa5a6c-2763-4b14-94fe-5de9ff888442.png">


接着是代码风格的问题，使用 tab 缩进，并且在 IDE 中 show whitespace

* 勾选 "show whitespace"

<img width="970" alt="Screenshot 2021-11-16 at 17 33 44" src="https://user-images.githubusercontent.com/13810907/141959882-b916d9cf-52de-480f-bddd-ca8f2378276f.png">

* 勾选 "use tab character" 和 "smart tab" 选项

<img width="616" alt="Screenshot 2021-11-16 at 17 35 48" src="https://user-images.githubusercontent.com/13810907/141960056-902077ad-6858-4ace-a262-093a84cfe3e1.png">

* 如何调试 greenplum，选择 attach to process 到 segment 的 idle 进程即可。


#### VSCode 篇
  
* lldb 配置方法
```json
{
            "name": "(lldb) Attach",
            "type": "cppdbg",
            "request": "attach",
            "program": "/Users/admin/greenplum/bin/postgres",  # 这里是你的主进程 postgres 所在的位置，，一定要有该文件路径
            "processId": "${command:pickProcess}",
            "MIMode": "lldb",
            "targetArchitecture": "x86_64"  # 指定了目标架构，否则会报 Warning
        },
```
* gdb 使用方法同上

在 vscode 中打开显示所有空白的方法：(同上 CLion 设置)

1. 打开setting,在搜索框中输入renderControlCharacters,选中勾选框,即可显示tab.
2. 在 setting 搜索框中输入renderWhitespace,选择all,即可显示空格.

launch.json 文件如下：

```shell
{
    // Use IntelliSense to learn about possible attributes.
    // Hover to view descriptions of existing attributes.
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [
        {
            "name": "(gdb) Attach",
            "type": "cppdbg",
            "request": "attach",  // 配置 gdb 连接方式为 attach
            "program": "/home/gpadmin/opt/gpdb/bin/postgres",   // 配置可执行 postgres 进程的位置
            "processId": "${command:pickProcess}",
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                },
                {
                    "description":  "Set Disassembly Flavor to Intel",
                    "text": "-gdb-set disassembly-flavor intel",
                    "ignoreFailures": true
                }
            ],
            "targetArchitecture": "x64"
        }

    ]

}
```

#### GDB

```shell
gdb attach pid  # GDB CLion VSCODE 只能有一个工具 attach 当前进程，否则会报错
```
