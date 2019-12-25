#pragma once
class MCom_port
{
public:
	MCom_port();
	~MCom_port();





	//寻找所有串口
	//ports[8],序号0，2，4，6串口号；1，3，5，7是是否被占用的标记为，=0是未被使用，=1是被使用了
	//因为最多有256个串口
	BOOL find_comms(int ports[512]);

	/*
	创建串口对象
	parameter[0]，parameter[1]是输入缓冲区和输出缓冲区的大小；parameter[2]是串口的波特率parameter[3],parameter[4]是读写超时
	CString ss 是串口提示信息
	*/
	BOOL create_port(HANDLE& hCom, int port_number, int parameter[5], CString ss);

	//向下位机发送数据
	BOOL send_date(HANDLE& hCom, BYTE dateParam[], int n, int wait_time, CString ss);

	//接收数据，接收到的数据保存在out_date,C++尽可能不要用指针
	BOOL receive_date(HANDLE& hCom, BYTE out_date[], int n, int wait_time, CString ss);

	//为串口配置参数 ---- param[0]:输入缓冲区;param[1]:输出缓冲区;param[2]:波特率
	//给find_port()函数使用
	BOOL config_param(HANDLE& hCom, int param[5], CString ss);

	//串口握手的时候，输入数据后，判断输出数据是否是自己预期的数据
	BOOL judge_port(HANDLE& hCom, BYTE date1[], int n1, int wait_time1, BYTE date2[], int n2, int wait_time2);

	//接收数据 并且 和期望的数据进行对比
	//n 是接收数据的长度
	BOOL receive_for_judge(HANDLE& hCom, BYTE dateParam[], int n, int wait_time, CString ss);

	//对下位机1个串口自动寻址
	//返回串口号
	int find_port(BYTE shake_date[], int n1, int wait_time1, BYTE shake_return[], int n2, int wait_time2, int param[], int ports[], CString ss);
};

