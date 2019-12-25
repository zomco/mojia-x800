#include "stdafx.h"
#include "MScan.h"
#include "MDate.h"
#include "MCom_port.h"



MScan::MScan()
{
}


MScan::~MScan()
{
}




//自动寻址串口号
BOOL MScan::find_port(int ports[])
{
	//被调用的类
	MCom_port com_port;
	port_number = com_port.find_port(shake_date, 8, 500, shake_return, 8, 500, param, ports, _T("扫码枪串口"));
	if (port_number != 0)
	{
		MDate::message(port_number, _T("扫码枪串口号"));
		return TRUE;
	}
	AfxMessageBox(_T("未发现扫码枪 ---- 请关闭软件，检测错误"));

	return FALSE;
}

//创建串口
BOOL MScan::create_port(HANDLE& hCom)
{
	//被调用的类
	MCom_port com_port;
	BOOL bl_1 = com_port.create_port(hCom, port_number, param, _T("扫码枪串口"));

	//创建后，做一次验证
	BOOL bl_2 = com_port.judge_port(hCom, shake_date, 8, 1000, shake_return, 8, 1000);
	if (bl_1 == TRUE && bl_2 == TRUE)
	{
		MDate::message(_T("扫码枪串口创建成功"));
		return TRUE;
	}
	else
	{
		AfxMessageBox(_T("扫码枪创建失败 ---- 请关闭软件，检测错误"));
		return FALSE;
	}
}

//扫码枪有可能是不能工作的，必须发送初始化数据后，才可以工作
BOOL MScan::init(HANDLE& hCom)
{
	//被调用的类
	MCom_port com_port;
	//创建后，做一次验证
	BOOL bl = com_port.judge_port(hCom, work_date, 8, 2000, work_return, 8, 2000);

	//因为这里做了串口数据操作，操作完之后，做一次缓冲区清空操作
	PurgeComm(hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);
	if (bl == FALSE)
	{
		AfxMessageBox(_T("警告：扫码枪初始化--错误"));
		return FALSE;
	}

	return TRUE;
}

//读取头数据信息----如果是-1，就清空本次读取的数据，进程必须进行下一次循环
//每次读取数据成功之后，就要让扫码枪暂停工作
BOOL MScan::read_date(HANDLE hCom, OVERLAPPED over_lapped)
{
	//因为OVERLAPPED over_lapped，要和调用函数保持一致，所以没有使用MCom_port com_port;里面的函数

	//1、在读写之前做清除错误的处理，和获取缓冲区数据大小。
	DWORD dwError;
	COMSTAT cs;
	if (!ClearCommError(hCom, &dwError, &cs))
	{
		AfxMessageBox(_T(" 警告：扫码枪串口清除错误失败"));
		CloseHandle(hCom);//??????????????????    这里不合理
		return FALSE;
	}

	//2、读取信息----判断数据是否读取(因为条码数据的长度你不知道)？？？？？？？？？？？？？？？？？？？？？？
	//联系3次判断如果cs.cbInQue的值是一样的，就认为数据读取完毕
	//BOOL MScan::read_date(HANDLE hCom, OVERLAPPED over_lapped) 这个函数是在主函数线程中检测到有数据，才会去执行的。
	int date_mumber = cs.cbInQue;
	int flag_three = 0;

	while (1)
	{
		if (!ClearCommError(hCom, &dwError, &cs))
		{
			AfxMessageBox(_T(" 警告：扫码枪串口清除错误失败"));
			CloseHandle(hCom);
			return FALSE;
		}
		int date_number_1 = cs.cbInQue;

		if (date_number_1 == date_mumber)
		{
			flag_three++;
		}
		else
		{
			flag_three = 0;
			date_mumber = date_number_1;
		}
		
		Sleep(5);
		//如果有3次联系读取的数据不变了，说明就读取数据完毕
		if (flag_three == 10)
		{
			break;
		}
	}

	//3、开始读取数据
	//读取数据
	BYTE buf[1024];
	memset(buf, 0, sizeof(buf));
	//实际读取到的数据数量
	DWORD nLenOut = 0;
	//开始读数据，因为是开始第一次读数据，所以要对缓冲区做一次清理操作
	BOOL bReadStatus = ReadFile(hCom,
		buf,
		date_mumber,
		&nLenOut,
		&over_lapped);

	//如果读取数据异常，清空缓冲区，退出本次循环
	if (!bReadStatus || date_mumber != nLenOut || nLenOut > 255)
	{
		//执行读取数据之前，有代码会还想消除串口错误的操作，所有这里不用操心
		AfxMessageBox(_T("扫码枪读取条码数据异常"));
		//发送数据之前清空缓冲区
		PurgeComm(hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);
		return FALSE;
	}

	//如果读数据的长度是0，说明有问题。
	if ((int)nLenOut == 0)
	{
		return FALSE;
	}
	

	//4、给保存条码的全局变量赋值
	bar_code[0] = nLenOut;
	for (int i = 0; i <(int)nLenOut; i++)
	{
		bar_code[i + 1] = buf[i];
	}

	//操作完之后，做一次缓冲区清空操作
	PurgeComm(hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);

	//读取数据成功后，就暂时关闭扫码枪读取数据的功能，防止操作逻辑混乱
	//被调用的类
	if (MDate::yi_jida_flag == 1)
	{
		MCom_port com_port;
		BOOL bl = com_port.judge_port(hCom, suspend_date, 8, 2000, suspend_return, 8, 2000);
		//因为这里做了串口数据操作，操作完之后，做一次缓冲区清空操作
		PurgeComm(hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);
		if (bl == FALSE)
		{
			AfxMessageBox(_T("扫码枪关闭失败 ---- 11请重新启动软件"));
			return FALSE;
		}
	}


	return TRUE;
}

BOOL MScan::get_bar_code()
{
	//1、读取扫码枪串口数据，赋值给scan.ss_bar_code
	int n = bar_code[0];
	if (n == 0)
	{
		AfxMessageBox(_T("警告：接收到的条码数据是空的"));
		return FALSE;
	}

	//1、获取条码信息之前，需要给条码变量清零
	str_bar_code = _T("");

	for (int i = 1; i < n; i++)//i < n + 1,现在扫码枪扫码后，后面还有一个会侧键
	{
		//2、将bar_code[i]里面的数据提取出来，生产条码信息
		CString ss;
		ss.Format(_T("%c"), bar_code[i]);

		str_bar_code = str_bar_code + ss;
	}

	return TRUE;
}

//关掉扫码枪工作模式
BOOL MScan::scan_close(HANDLE& hCom)
{
	//被调用的类
	MCom_port com_port;
	//创建后，做一次验证
	BOOL bl = com_port.judge_port(hCom, suspend_date, 8, 2000, suspend_return, 8, 2000);
	if (bl == FALSE)
	{
		AfxMessageBox(_T("警告：扫码枪暂停工作--错误"));
		return FALSE;
	}

	return TRUE;
}

//开启扫码枪工作模式
BOOL MScan::scan_start(HANDLE& hCom)
{
	//被调用的类
	MCom_port com_port;
	//开启光幕工作
	BOOL bl = com_port.judge_port(hCom, work_date, 8, 2000, work_return, 8, 2000);
	//因为这里做了串口数据操作，操作完之后，做一次缓冲区清空操作
	PurgeComm(hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);
	if (bl == FALSE)
	{
		AfxMessageBox(_T("警告：扫码枪初始化--错误"));
		return FALSE;
	}

	return TRUE;
}

