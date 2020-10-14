#ifndef RDEPOLLSOCKET_H
#define RDEPOLLSOCKET_H
/* * 
 * Author:Pluviophile
 * Date:2020.6.20
 * 
 * 此类会初始化AIO与epoll，实现异步非阻塞的IO多路复用的socket,
 * 并对客户端连接进行负载均衡
 * 
 * AddEpollCtl: epoll_ctl函数的封装函数，此函数内将fd添加进epoll
 * DelEpollCtl：epoll_ctl函数的封装函数，此函数内将fd从epoll中删除
 * SettingSocket: 为创建的socket_fd设置本地地址与端口复用，并设置其绑定监听
 * AcceptCon: accept函数封装类接收新传来的客户端socket_fd，并将其添加进epoll后设置其为非阻塞模式
 * SubmitRead: 异步非阻塞读函数
 * SubmitWrite: 异步非阻塞写函数
 * setLport: 设置要监听的端口
 * setMaxSize: 设置最大数量
 * InitializSocket: 初始化套接字
 * work: 工作进程
 * start: 开始执行
 * */

#include<sys/epoll.h>
#include<fcntl.h>
#include<libaio.h>
#include<time.h>
#include<errno.h>
#include<sys/syscall.h>
#include<sys/ioctl.h>
#include<sys/eventfd.h>
#include<sys/types.h>
#include<sys/resource.h>
#include<sys/sysinfo.h>
#include<arpa/inet.h>
#include<wait.h>
#include<unistd.h>
#include<iostream>
#include<cstring>

#define _In_
#define _Inout_
#define _Out_

using namespace std;

typedef void (*RDRecvData_Callback)(
    _In_ io_context_t ctx,
    _In_ int fd,
    _In_ int io_fd,
    _In_ const char* buff
);


class RDEpollSocket
{
private:
    short lport = 0;
    int epoll_fd = 0;
    int MaxSize = 0;
    int socket_fd = 0;
    RDRecvData_Callback CallBack = NULL;
    
    bool InitializeSocket();

    bool AddEpollCtl
    (
        _In_ int epfd,
        _In_ int fd,
        _In_ int mask
    );
    
    bool DelEpollCtl
    (
        _In_ int epfd,
        _In_ int fd
    );
    bool AcceptConn
(
    _In_ int fd,
    _In_ int epoll_fd,
	_Inout_ int& num
);

    bool work();
public:
    RDEpollSocket();
    void CloseServer();
    bool SettingSocket
    (
        _In_ int socket_fd
    );

    static bool SubmitRead
    (
        _In_ io_context_t ctx,
        _In_ int fd,
        _In_ int io_fd,
        _Inout_ char* buff,
        _In_ int buff_len
    );

    static bool SubmitWrite
    (
        _In_ io_context_t ctx,
        _In_ int fd,
        _In_ int io_fd,
        _In_ char* buff,
        _In_ int buff_len
    );

    void setLport
    (
        _In_ short lport
    );
    void setMaxSize
    (
        _In_ int MaxSize
    );

    void setRDCallBack
    (
        _In_ RDRecvData_Callback CallBack
    );

   bool start();
   ~RDEpollSocket();
};
#endif


