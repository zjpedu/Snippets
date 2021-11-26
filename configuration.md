### mac m1 pro 环境

最近新买了 mac m1 pro，搭建了开发环境，除了 brew 的过程会下载源码编译工具链以外，其它没有特别之处，这里主要记录我的环境变量。

#### shell 选择

我习惯了用 bash shell，很多用 mac 的人估计都会选择 bash shell。

```shell
which $SHELL # 返回 /bin/zsh
chsh -s /bin/bash # 输入密码切换
which $SHELL # 返回 /bin/bash，关闭当前终端，重新打开一个新的
```

**注意：切换完成之后，一定要重新打开 bash**

#### 免密登陆

```shell
hostname # 我的主机名为 jpzhu-db-edu
ssh jpzhu-db-edu # 这个命令下一定要保证能通过，不能返回拒绝，如果拒绝，则需要“系统偏好设置”->"共享"->选中"远程登陆"，我的 hostname 为 jpzhu-db-edu
ssh jpzhu-db-edu # 如果上次不通过，则这次通过
ssh-keygen
cat ~/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys# 设置免密登陆
ssh jpzhu-db-edu #不需要密码也可以登陆进去
exit
```

将下面信息写入到 /etc/hosts 中：

```shell
127.0.0.1 jpzhu-db-edu
```

#### 环境变量

mac 中环境变量设置一定要小心，尤其涉及到存在 python 环境时，那家伙叫一个乱。为了分布式集群能够通信，所以需要维护两个环境变量，分别是 ~/.bashrc 和  ~/.bash_profile。
用于分布式集群中通信时，**仅仅会检查 ~/.bashrc 变量，因此，如果安装 greenplum 等依赖 python 环境的数据库时，一定要注意认真维护 ~/.bashrc。** 下面是我的环境变量，会随时更新

```shell
# bash_profile
# go
export GOPATH=/Users/admin/go
export GOPROXY="https://goproxy.cn,direct"
export GO111MODULE=on

# database
export MXHOME=/Users/admin/opt/mxdb
export PATH=/opt/homebrew/bin:$MXHOME/bin:$GOPATH/bin:$PATH

# alias
alias ls='ls -G'
alias ll='ls -lG'
alias la='ls -laG'
```

```shell
# bashrc
export PATH=/opt/homebrew/bin:$PATH
alias ls='ls -G'
alias ll='ls -lG'
alias la='ls -laG'
```
