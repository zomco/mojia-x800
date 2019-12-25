#pragma once
class MWeight
{
public:
	MWeight();
	~MWeight();

public:
	//------------------------------串口参数------------------------------
	//配置参数----param[0]----输入缓冲区；param[1]----输出缓冲区；param[2]----波特率；
	int param[5] = { 1024, 1024, 1200, 2000, 2000 };
	//称重串口号；
	int port_number = 0;
	//串口握手数据
	//注意握手数据的长度变化了
	/*
	发送数据R，给下位机，就会返回串口重量，R对应的16进制数据是0x52
	数据格式：
	发送： R							52
	返回： wn-001.218kg					77 6E 2D 30 30 31 2E 32 31 38 6B 67 0D 0A
	我们自己定义握手数据格式
	发送： 52							52
	返回： wn-							77 6E 
	*/
	BYTE shake_date[20] = { 0x52 };
	BYTE shake_return[20] = { 0x77, 0x6E };
	//------------------------------串口参数------------------------------
	//存贮重量信息的变量
	int weight = 0;



	//寻址串口号码函数
	BOOL find_port(int ports[]);
	//创建串口
	BOOL create_port(HANDLE& hCom);
	//计算一次称重的重量 ---- 实际称重里面的
	//重量的值会赋值给int weight变量
	BOOL get_weight(HANDLE& hCom);
};

