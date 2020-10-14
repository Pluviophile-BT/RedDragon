#ifndef RDMUTEX_H
#define RDMUTEX_H

/* *
 *Author: Pluviophile
 *Date: 2020.7.10
 *
 * 进程锁，避免惊群效应，使用共享内存,
 * 针对多进程epoll惊群现象进行处理，高版本内核本身已对其进行处理
 * 
 * */

#include<sys/mman.h>
#include<iostream>
#include<cstring>
#include<unistd.h>

#define _In_
#define _Inout_
#define _Out_

class RDMutex
{
public:
    RDMutex();
    ~RDMutex();
private:

};
#endif