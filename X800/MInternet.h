#pragma once
/*
和服务器连接相关功能都在这个类里面。

*/

#include <WinInet.h>
#include <afxinet.h>



class MInternet
{
public:
	MInternet();
	~MInternet();

public:
	//相关变量
	//服务器地址：www.huodull.com
	//服务器测试地址：test.huodull.com
	CString strServer = _T("test.huodull.com");
	//端口号
	INTERNET_PORT  nPort = 80;
	//接口在服务器的地址
	CString pObject = _T("/api/wh/add.json");

	//用户名
	CString user_name = _T("measuring");
	CString test_name = _T("testmeasuring");

	//密码
	//测试key:8a68e8bcb7f17e106b535fd71a88baf3
	//正式key:4afda99e4e510c5f758e8ee4a07140bc
	CString pass_word = _T("4afda99e4e510c5f758e8ee4a07140bc");//e2bad2
	CString text_word = _T("8a68e8bcb7f17e106b535fd71a88baf3");//994be6

	//发送数据的变量
	CString send_type;		//类型 ---- 新增/修改
	CString send_parcelNo;	//编号

	CString send_length;	//长
	CString send_width;		//宽
	CString send_height;	//高
	CString send_weight;	//重量
	CString send_orderNo;	//条码


	//对要长传的变量进行复制操作
	void init_http_date(CString p_length, CString p_width, CString p_height, CString p_weight, CString p_orderNo);
	

	//发送数据函数
	BOOL send_date_yijida();
	BOOL send_date_yijida_test();
};

