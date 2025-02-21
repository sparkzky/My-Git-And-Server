## MyGit

***main.cpp***：服务器的启动，设置回调函数，利用简单工厂模式，根据输入进行判断而调用不同的Repository函数

***sha1***：sha1加密算法

***Blob***：用于操作每一个单独的文件，并生成各自的blob文件

***Commit***：管理每次commit，并记录在commit文件中

***Stage***：管理工作区，并写入stage文件中记录

***Tree***：用于记录每次更改的记录，并记录到tree文件中

***Repository***：在此处实现类似git的基本功能

## MyGitServer

***AidClass***：实现Socket，Buffer，InetAddr类，对基本函数进行包装

***Factory***：实现逻辑判断的简单工厂

***ThreadPool***：利用泛型实现线程池

***EpollEvent***：包装系统的epoll，实现用于监听的Epoll类

***Client***：客户端实现

***Server***：服务器实现

## 流程

在Server类定义的时候会设置具体的回调函数

new_connection_callback是传给Acceptor，使其监听到新连接后生成Connection类对象并设置read_callback函数和remove_connection_callback函数

Connection接收到客户端发送来的信息后调用read_callback处理

Acceptor和Connection底层都是靠epoll的监听的channel事件，都会被加入Epoll中监听

channel被设置为ready后会被poll下来，在loop函数里面被调用

Epoll靠线程池来loop，进而执行每一个channel事件
