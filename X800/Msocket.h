#include <afxsock.h>

#pragma once

class Msocket:public CSocket 
{
public:
	Msocket();
	~Msocket();

	//链接服务器 
	void Mconnect();

	//接收到数据后，会自动运行的函数
	virtual void OnReceive(int nErrorCode);
};

