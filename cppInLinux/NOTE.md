 # Makefile
 > [参考Makefile实战](https://www.yuque.com/linuxer/xngi03/nfsm1k?)
 # CMake
 > [参考-CMake实战](https://www.yuque.com/linuxer/xngi03/cs5y85?)

 语法：INCLUDE_DIRECTORIES找头文件
 ```
 INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/dir1")
 ```
 语法：ADD_SUBDIRECTORY添加子目录
 ```
 ADD_SUBDIRECTORY("${CMAKE_CURRENT_SOURCE_DIR}/dir1")
 ```
 语法：ADD_LIBRARY生成库文件
 ```
 ADD_LIBRARY(hello_shared SHARED libHelloSLAM.cpp) # 生成动态库
 ADD_LIBRARY(hello_static STATIC libHelloSLAM.cpp) # 生成静态库
 ```
 语法：TARGET_LINK_LIBRARIES链接库文件到执行文件上
 ```
 TARGET_LINK_LIBRARIES(hello_shared ${OpenCV_LIBS}) # 链接动态库到执行文件上
 ```
 语法：AUX_SOURCE_DIRECTORY获取当前目录下所有源文件
 ```
 AUX_SOURCE_DIRECTORY(. DIRSRCS) # 获取当前目录下所有源文件
 ```
 语法：INSTALL设置安装路径
 ```
 INSTALL(TARGETS hello_shared DESTINATION lib) # 安装动态库到指定目录
 INSTALL(TARGETS hello_static DESTINATION lib) # 安装静态库到指定目录
```

# GIT
<img src="./sources/gitOverall.png"/>

## 逆向操作
index $\rightarrow$ workspace
```
git restore -S
```
$\begin{cases}本地仓库 \rightarrow index\\
本地仓库 \rightarrow workspace \\
本地仓库 \rightarrow null\\
\end{cases}$
```
git reset --soft
git reset --mixed
git reset --hard
```
workspace $\rightarrow$ null
```
git checkout
git checkout --file
git checkout -f
```
## 本地仓库整理
|类型|说明|
|--|--|
|feat|新功能|
|fix|修复BUG|
|docs|文档|
|style|代码格式|
|refactor|重构|
|test|增加测试|
|chore|构建过程或辅助工具的变动|
|perf|性能优化|

整理上一次的提交：将本次修改内容合并到上次的提交中
```
git commit --amend
```
整理多次提交：将多次提交合并为一次提交
左开右闭
```
git rebase -i h1 h2
```
## 分支操作
查看所有分支，加-a可以查看所有分支包括远程分支
```
git branch
```
创建分支
```
git branch <branchName>
```
切换分支
```
git checkout <branchName>
```
创建并切换分支
```
git checkout -b <branchName>
```
合并分支
先拉去master最新的代码，在把master的代码合并到当前分支，测试通过后再合并到master
```
git merge <branchName>
```
删除分支
```
git branch -d <branchName>
```
## 解决冲突
冲突产生的原因：不同分支修改了同一文件的同一行或相邻行
解决冲突的原则：不要影响其他人提交的功能，并协商解决
解决后提交代码：
```
git commit -i -m ""
```
<img src="sources/gitBranch.png" />

# Linux系统
将windows换行符换成linux换行符
```
dos2unix [filename]
```
将linux换行符换成windows换行符
```
unix2dos [filename]
```
`uptime`
- 当前时间
- 系统运行时间
- 在线用户数(总的连接数，开一个终端就算一个用户)
- 系统平均负载(1分钟，5分钟，15分钟的平均负载)
> 当前负载为2意味着？ 
- 在只有两个cpu的系统上，意味着所有cpu被刚好全部占满
- 在只有4个cpu的系统上，意味着cpu有50%的空闲
- 在只有一个cpu的系统上，意味着有一半的进程竞争不到cpu
查看cpu信息 cat /proc/cpuinfo
查看cpu核数 grep 'model name' /proc/cpuinfo | wc -l

查找对应进程是否在运行
```
ps -ef | grep [进程名]
```

压力测试和系统性能分析
安装命令
```
apt install stress sysstat
```
`mpstat`是一个常见的多核性能分析工具，可以查看每个cpu的负载情况
```
mpstat -P ALL 1 5
```
`pidstat`是一个常见的进程分析工具，用来实时查看进程的cpu、内存、io等资源占用情况

---

`vmstat`是一个系统性能分析工具，可以查看系统的整体负载情况
```
vmstat [interval] [time]
```
r: 表示就绪队列长度，就是正在运行和等待cpu的进程数
b：表示不可中断睡眠状态的进程数
buff：是i/o系统存储的此版块文件的元数据统计信息
cache：操作系统用来缓存磁盘数据的缓存区，操作系统会自动调节这个参数，在内存紧张时会减少cache占用保证其他进程可用
si和so较大时，说明系统频繁使用交换区，应该查看操作系统的内存是否够用
bi和bo代表i/o活动，磁盘io负载情况
in(interrupt)是每秒中断次数
cs：表示线程环境的切换次数，此数据较大时表示线程的同步机制有问题

---

使用`netstat`或者`ss`，来查看套接字、网络栈、网络接口以及路由表信息

-a(all) 显示所有选项默认不显示LISTEN相关
-t(tcp) 仅显示tcp相关选项
-u(udp) 仅显示udp相关选项
-n(numeric) 拒绝显示别名，能显示数字的全部转化为数字
-l 仅列出有在 Listen (监听) 的服務状态

-p 显示建立相关连接的程序名
-r 显示路由信息，路由表
-e 显示扩展信息，例如uid等
-s 按各个协议进行统计
-c 每隔指定的时间，执行指定次数的统计

- 当套接字处于连接状态（Established)时：
    `Recv-Q`表示套接字缓冲还没有被应用程序取走的字节数（即接受队列长度）
    `Send-Q`表示还没有被远端主机确认的字节数（即发送队列长度）

- 当套接字处于监听状态（LISTEN)时：
    `Recv-Q`表示全连接队列的长度
    `Send-Q`表示全连接队列的最大长度

检测网络带宽
给`sar`命令加上-n参数就可以查看网络的统计信息，比如网络接口（DEV）、网络接口错误（EDEV）、TCP、UDP、ICMP等等。
```
sar -n DEV 1 5
```
测试服务器端口
```
nc -l [port] #开启一个本地9999的tcp协议端口，有客户端主动发起连接，一旦连接必须由服务端发起关闭
nc -vw [time] [ip] [port] 通过nc访问ip的端口确认是否存活，等待time秒，默认3秒
```
nslookup解析域名
```
nslookup [domain]
```