#include "stdafx.h"
#include "MWeight.h"
#include "MDate.h"
#include "MCom_port.h"


MWeight::MWeight()
{
}


MWeight::~MWeight()
{
}




//寻址串口号
BOOL MWeight::find_port(int ports[])
{
	//被调用的类，这个类负责处理串口相关的
	MCom_port com_port;
	port_number = com_port.find_port(shake_date, 1, 1000, shake_return, 2, 5000, param, ports, _T("称重串口"));
	if (port_number != 0)
	{
		MDate::message(port_number, _T("称重串口号"));
		return TRUE;
	}
	AfxMessageBox(_T("未发现称重器 ---- 请关闭软件，检测错误"));

	return FALSE;
}


//创建串口
BOOL MWeight::create_port(HANDLE& hCom)
{
	//被调用的类
	MCom_port com_port;
	BOOL bl_1 = com_port.create_port(hCom, port_number, param, _T("称重串口"));
	//创建后，做一次验证

	//这是一个奇怪的地方，我猜：当下位机给上位机传输数据的时候还没有传过来，上位机是不能给下位机发数据，发了也没有用。
	Sleep(30);

	BOOL bl_2 = com_port.judge_port(hCom, shake_date, 1, 2000, shake_return, 2, 2000);
	if (bl_1 == TRUE && bl_2 == TRUE)//bl_1 == TRUE && bl_2 == TRUE
	{
		MDate::message(_T("称重串口创建成功"));
		return TRUE;
	}
	else
	{
		AfxMessageBox(_T("称重器创建失败 ---- 请关闭软件，检测错误"));
		return FALSE;
	}
}


//获取重量信息
BOOL MWeight::get_weight(HANDLE& hCom)
{
	//1、发送数据
	//给串口发送一个R，就等到返回数据，就是重量的值
	//指令：0x52
	//发送二进制数据
	BYTE date[1] = { 0x52 };
	//1、发送数据
	MCom_port com_port;
	BOOL bl_send = com_port.send_date(hCom, date, 1, 1000,_T("称取重量"));
	if (bl_send == FALSE)
	{
		return FALSE;
	}

	//2、接收数据
	//仪表返回值：14个BYTE类型的数据。后面2个是"\r\n"
	BYTE date_receive[14] = { 0x00};
	BOOL bl_receive = com_port.receive_date(hCom, date_receive, 14, 2000, _T("称取重量"));
	if (bl_receive == FALSE)
	{
		return FALSE;
	}

	//按照仪表的数据格式，读取数据，已g为单位   ww 00000.00kg
	weight = (date_receive[4] - 48) * 100 * 1000 + (date_receive[5] - 48) * 10 * 1000 + (date_receive[6] - 48) * 1000 + (date_receive[8] - 48) * 100 + (date_receive[9] - 48) * 10;

	return TRUE;
}