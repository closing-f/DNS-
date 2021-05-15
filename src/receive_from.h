//#pragma once
#ifndef INC_RECEIVE_FROM_H
#define INC_RECEIVE_FROM_H
#include"funs_with_localout.h"

//从远端DNS接收报文并转发到本机
void receive_from_out();

//从本机读取DNS查询，从缓存读取或发送到外部DNS服务器查询
void receive_from_local();

#endif