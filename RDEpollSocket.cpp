#include"RDEpollSocket.h"
#include"RDHttpHeader.h"

RDEpollSocket::RDEpollSocket()
{
    nullptr;
}

bool RDEpollSocket::SubmitRead
(
    _In_ io_context_t ctx,
    _In_ int fd,
    _In_ int io_fd,
    _Inout_ char* buff,
    _In_ int buff_len
)
{
   	iocb iocb1;
	iocb *ic = &iocb1;
	io_prep_pread(ic, fd, buff, buff_len, 0);
	io_set_eventfd(ic, io_fd);
	//提交异步IO任务
	int reio = io_submit(ctx, 1, &ic);
	if (reio != 1)	
        return false;
		
    return true;
}

bool RDEpollSocket::SubmitWrite
(
    _In_ io_context_t ctx,
    _In_ int fd,
    _In_ int io_fd,
    _In_ char* buff,
    _In_ int buff_len
)
{
   	iocb iocb1;
	iocb *ic = &iocb1;
	io_prep_pwrite(ic, fd, buff, buff_len, 0);
	io_set_eventfd(ic, io_fd);
	//提交异步IO任务
	int reio = io_submit(ctx, 1, &ic);
	if (reio != 1)	
        return false;

    return true;
}


/* *
 * set方法设置各属性值
 * */
void RDEpollSocket::setLport
(
    _In_ short lport
)
{
    this->lport = lport;
}

void RDEpollSocket::setMaxSize
(
     _In_ int MaxSize
)
{
    this->MaxSize = MaxSize;
}

/* *
 * epoll ctl add
 * */
bool RDEpollSocket::AddEpollCtl
(
    _In_ int epfd,
    _In_ int fd,
    _In_ int mask
)
{
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = mask;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev))
		return false;
    return true;
}

/**
 * epoll ctl delete
 * */
bool RDEpollSocket::DelEpollCtl
(
    _In_ int epfd,
    _In_ int fd
)
{
	if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL))
		return false;
    return true;
}

/* *
 * accept add
 * */
bool RDEpollSocket::AcceptConn
(
    _In_ int fd,
    _In_ int epoll_fd,
	_Inout_ int& num
)
{
	struct sockaddr_in client_addr;
	socklen_t clen;
	while (true)
	{
		int new_socket = accept4(fd, (sockaddr*)&client_addr, &clen, SOCK_NONBLOCK);
		if (new_socket < 0)
			return false;
		//注册新的文件描述符, 客户端链接网络套接字
	    AddEpollCtl(epoll_fd, new_socket, 
                            EPOLLIN | EPOLLET | EPOLLOUT | EPOLLRDHUP | EPOLLERR);
		//负载均衡变量+1
		num++;
		//非阻塞client socket fd设置
		fcntl(new_socket, F_SETFL, 
				fcntl(new_socket, F_GETFL, NULL) | O_NONBLOCK);
	}
    return true;
}

/* *
 * 设置socket属性，并进行绑定以及监听操作
 * */
bool RDEpollSocket::SettingSocket
(
    _In_ int socket_fd
)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(lport);
    addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;

	//非阻塞server socket fd设置
	fcntl(socket_fd, F_SETFL, 
			fcntl(socket_fd, F_GETFL, NULL) | O_NONBLOCK);
	//设置允许在端口释放后立即被再次使用
    if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        return false;
	//设置允许完全的重复绑定
    if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)))
        return false;
    if(bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)))
        return false;
    if(listen(socket_fd, 5))
        return false;
    return true;
}

void RDEpollSocket::setRDCallBack
(
        _In_ RDRecvData_Callback CallBack
)
{
	this->CallBack = CallBack;
}

/* *
 * 初始化socket套接字
 * */
bool RDEpollSocket::InitializeSocket()
{
    //创建套接字,以非阻塞的方式
    socket_fd = socket(AF_INET, 
								  SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);

    //设置可打开的文件数
    struct rlimit rl;
    rl.rlim_max=rl.rlim_cur = MaxSize;
    if(setrlimit(RLIMIT_NOFILE, &rl) == -1)
	{
		close(socket_fd);
        return false;
	}

	SettingSocket(socket_fd);
	return true;
}

/* *
 * 实际工作进程,多个进程使用相同的socket fd,不同的epoll fd
 * */
bool RDEpollSocket::work()
{
	io_context_t ctx = 0;
	//设置负载均衡变量num等必要局部变量
	int num = 0;
    int io_fd = 0;

    struct epoll_event* epevents;
	struct io_event* events;
	events = (struct io_event*)malloc(sizeof(struct io_event)*MaxSize);
	epevents = (struct epoll_event*)malloc(sizeof(struct epoll_event)*MaxSize);
	epoll_fd = epoll_create(MaxSize);
	if(epoll_fd < 0)
		return false;

	AddEpollCtl(epoll_fd, socket_fd, 
						EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET | EPOLLERR);
	io_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if(io_fd < 0)
	{
		close(epoll_fd);
		return false;
	}

	AddEpollCtl(epoll_fd, io_fd, EPOLLIN | EPOLLET);
	if(io_setup(MaxSize, &ctx) < 0)
	{
		close(epoll_fd);
		return false;
	}

	//main loop
	while (true)
	{
		int ret = 0;
		//轮询等待事件触发, 返回事件个数
		ret = epoll_wait(epoll_fd, epevents, MaxSize, 10);
		for(int i=0; i<ret; i++)
		{
			//触发事件的文件描述符如果为网络套接字
			if(epevents[i].data.fd == socket_fd)
			{
				//负载均衡设置，超过最大连接数7/8就不再接受请求
				if(num < ((int)((float)MaxSize*(float)(7.0/8.0))))
					AcceptConn(socket_fd, epoll_fd, num);
			}
			//触发事件的文件描述符如果为io描述符
			else if(epevents[i].data.fd == io_fd)
			{
				uint64_t io_finshed = 0;
				if((read(io_fd, &io_finshed, sizeof(io_finshed))) != 
					sizeof(io_finshed))
			{
				close(epoll_fd);
				return false;
			}

				struct timespec tim;
				while(io_finshed > 0)
				{
					tim.tv_nsec = tim.tv_sec = 0;
					int gt = io_getevents(ctx, 0, MaxSize, events, &tim);
					for(int j=0;j<gt;j++)
					{
						int fd = events[j].obj->aio_fildes;
						int comple_type = events[j].obj->aio_lio_opcode;
						int comple_len = events[j].res;
						int success = events[j].res2;
						char* temp_buff = NULL;

						if(comple_type == IO_CMD_PREAD)
							if(success == 0)
							{
								if(comple_len <= 0)
								{
									close(fd);
									continue;
								}
							}
							else
							{
								close(fd);
								close(epoll_fd);
								return false;
							}

						else if(comple_type == IO_CMD_PWRITE)
							if(success != 0)
							{
								close(fd);
								close(epoll_fd);
								return false;
							}
					}
					io_finshed -= gt;
				}
			}
			//其他
			else
			{
				//有数据到来
				if(epevents[i].events & EPOLLIN)
				{
					/**
					*此处获取数据并进行响应处理
					**/
					char buff[5242880] = "\0";
					SubmitRead(ctx, epevents[i].data.fd, io_fd, buff, 5242880);
					if(strlen(buff) == 0)
						DelEpollCtl(epoll_fd, epevents[i].data.fd);
					else
					{
						//如果回调函数指针不为空
						if(CallBack)
							CallBack(ctx,epevents[i].data.fd,io_fd, buff);
						//通讯完成后从链中删除套接字
						DelEpollCtl(epoll_fd, epevents[i].data.fd);
					}
				}
				else if(epevents[i].events & EPOLLRDHUP)
				{
					DelEpollCtl(epoll_fd, epevents[i].data.fd);
					close(epevents[i].data.fd);
				}
				else if(epevents[i].events & EPOLLERR)
				{
					DelEpollCtl(epoll_fd, epevents[i].data.fd);
					close(epevents[i].data.fd);
				}
			}
		}
	}
	return true;
}

/* *
 * 整个类的开始入口
 * */
bool RDEpollSocket::start()
{
	//初始化socket套接字
	if(! InitializeSocket())
		return false;

	//从此开始处理并进入子进程
	pid_t pro[3];
	for(int i=0;i<3;i++)
	{
		pro[i] = fork();
		if(pro[i] == 0)
			work();
		else if(pro[i] > 0)
			continue;
		else
			return -1;
	}

	//等待
	pause();
	return true;
}
    
void RDEpollSocket::CloseServer()
{
	close(socket_fd);
}

RDEpollSocket::~RDEpollSocket()
{
	close(epoll_fd);
	close(socket_fd);
}