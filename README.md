# RedDragon
    本意是想仿造nginx编写一个基于C++的高性能WEB服务器，但我高估了我的精力......
    已将核心的epoll多路IO复用socket与aio异步读写函数写完，除此以外还有一个解析文件mime type的类。
    
  RDEpollSocket为使用epoll实现多路复用socket的类，并且在类中实现了异步读写函数进行socket读写，从服务器角度来讲应该是已经具有了一定的并发性能，只是从一个web服务器的角度来讲，还有很多基本内容还未完成。<br><br>
    其基本实现思路与nginx一样，也是利用一个master进程去管理多个work子进程，实际的工作进程为work进程，不同的地方在与nginx是多个子进程共用一个epoll fd与socket fd，而这个则是子进程各自用各自的epoll fd使用同一个socket fd，做了基本的负载均衡，但针对惊群效应的互斥锁和其他更多内容还未写完，等后期会逐渐补上。<br><br>
    此项目因个人精力有限而代码量又较为庞大，所以进度十分缓慢，如果有同样对nginx原理与高并发服务器编写感兴趣的同好，可以+QQ：<strong>1565203609</strong>或者发送邮件到：<strong>pluviophile12138@outlook.com</strong>在研究学习的同时，一起完成这个开源项目。
<br><br>
更多有关代码内容解释在代码注释中
