#pragma once


class MScan
{
public:
	MScan();
	~MScan();


public:
	//串口号
	int port_number = 0;
	//握手数据
	BYTE shake_date[8] = { 0xee, 0x66, 0x06, 0x00, 0x01, 0x01, 0x01, 0x01 };
	BYTE shake_return[8] = { 0xee, 0x66, 0x06, 0x01, 0x01, 0x01, 0x01, 0x01 };
	//启动工作数据
	//一个工作周期完成之后，就发送这个数据，告诉扫码枪可以开始工作了
	BYTE work_date[8] = { 0xee, 0x66, 0x06, 0x10, 0x01, 0x01, 0x01, 0x01 };
	BYTE work_return[8] = { 0xee, 0x66, 0x06, 0x11, 0x01, 0x01, 0x01, 0x01 };
	//暂停工作数据
	//扫码枪发送数据，上位机接收数据完后，发送这个数据给扫码枪，告诉扫码枪可以暂停工作了
	BYTE suspend_date[8] = { 0xee, 0x66, 0x06, 0x20, 0x01, 0x01, 0x01, 0x01 };
	BYTE suspend_return[8] = { 0xee, 0x66, 0x06, 0x21, 0x01, 0x01, 0x01, 0x01 };

	//配置参数----param[0]----输入缓冲区；param[1]----输出缓冲区；param[2]----波特率
	//parameter[3] ---- 串口读数据的超时;parameter[4] ---- 串口写数据的超时;
	int param[5] = { 1024, 1024, 9600, 2000, 2000 };

	//用于保存条码信息的全局变量----数组第一个数据保存的是有效数据的长度信息
	int bar_code[256] = { 0 };
	//条码
	CString str_bar_code = _T("");



	//寻址串口号码函数
	BOOL find_port(int ports[]);
	//创建串口
	BOOL create_port(HANDLE& hCom);
	//扫码枪有可能是不能工作的，必须发送初始化数据后，才可以工作
	BOOL init(HANDLE& hCom);
	//读取数据信息 
	BOOL read_date(HANDLE hCom, OVERLAPPED over_lapped);
	//将条码信息转换成字符串 ---- 读取扫码枪串口数据，赋值给ss_bar_code
	BOOL get_bar_code();
	//关掉扫码枪
	BOOL scan_close(HANDLE& hCom);
	//开启扫码枪
	BOOL scan_start(HANDLE& hCom);





};

