#include "stdafx.h"
#include "MVolume.h"
#include "MCoordinate.h"
#include <opencv2/opencv.hpp>
#include "MCom_port.h"
#include "MDate.h"
#include "MAlgorithm.h"



MVolume::MVolume()
{
}


MVolume::~MVolume()
{
}



//***********************************************************串口*******************************************************
//1、寻找串口
BOOL MVolume::find_port(int ports[])
{
	//被调用的类
	MCom_port com_port;
	port_number = com_port.find_port(shake_date, 6, 2000, shake_return, 6, 2000, param, ports, _T("侧体积串口"));
	if (port_number != 0)
	{
		MDate::message(port_number, _T("侧体积串口号"));
		return TRUE;
	}

	AfxMessageBox(_T("未发现体积测量---- 请关闭软件，检测错误"));
	return FALSE;
}

//2、创建串口
BOOL MVolume::create_port(HANDLE& hCom)
{
	//被调用的类
	MCom_port com_port;
	BOOL bl_1 = com_port.create_port(hCom, port_number, param, _T("体积测量串口"));
	//创建后，做一次验证
	BOOL bl_2 = com_port.judge_port(hCom, shake_date, 6, 2000, shake_return, 6, 2000);
	if (bl_1 == TRUE && bl_2 == TRUE)
	{
		MDate::message(_T("侧体积串口创建成功"));
		return TRUE;
	}
	else
	{
		AfxMessageBox(_T("体积测量创建失败 ---- 请关闭软件，检测错误"));
		return FALSE;
	}
}

//3、读取数据
/*
1、返回0，就是错误的意思
2、返回1，就是正确的意思
3、返回2、就是
*/
int MVolume::read_date(HANDLE hCom)
{
	//1、box_date[]保持数据的数组， 对这个数组数据清零。
	/*
	读取数据之前，先将保存数据的数组赋值为0
	作用是在一段内存块中填充某个给定的值，它是对较大的结构体或数组进行清零操作的一种最快方法
	*/
	memset(box_date, 0, sizeof(box_date));

	//2、定义重叠变量，设置变量里面的hEvent属性
	OVERLAPPED over_lapped_1;
	memset(&over_lapped_1, 0, sizeof(OVERLAPPED));
	over_lapped_1.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	//3、在读写之前做清除错误的处理， 和获取缓冲区数据大小。
	DWORD dwError_1;
	COMSTAT cs_1;
	if (!ClearCommError(hCom, &dwError_1, &cs_1))
	{
		AfxMessageBox(_T(" 警告：体积测量串口清除错误失败----请重新启动软件"));
		return 0;
	}

	//3、开始读取数据----头3个字节的数据
	/*
	第1位：数据头--0xdd		标志位
	第2，3位：测量次数N		扫码周期数量
	*/
	BYTE buf_1[3];
	memset(buf_1, 0, sizeof(buf_1));
	//实际读取到的数据数量
	DWORD nLenOut_1 = 0;
	//开始读数据
	BOOL bReadStatus = ReadFile(hCom,
		buf_1,
		3,
		&nLenOut_1,
		&over_lapped_1);
	if (!bReadStatus)
	{
		if (GetLastError() == ERROR_IO_PENDING)	//GetLastError()函数返回ERROR_IO_PENDING,表明串口正在进行读操作
		{
			//使用WaitForSingleObject函数等待，直到读操作完成或延时已达到20秒钟
			DWORD error = WaitForSingleObject(over_lapped_1.hEvent, wait_time);	//INFINITE	wait_time
			switch (error)
			{
			case WAIT_OBJECT_0:
				//所代表的进程在wait_time秒内结束，关键这个语句可以返回实际收到的数据的数量 
				//如果此参数为FALSE且操作仍处于暂挂状态，则该函数返回FALSE， GetLastError函数返回ERROR_IO_INCOMPLETE。
				if (!GetOverlappedResult(hCom, &over_lapped_1, &nLenOut_1, false))
				{
					AfxMessageBox(_T("警告：体积测量接收数据----串口出现错误"));
					PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
					return 0;
				}
				//返回值成功，break退出switch语句
				break;
			case WAIT_TIMEOUT:
				// 等待时间超时
				//情况缓冲区---- 否则不会释放串口接收数据的功能，否则后续能发数据，但是不能接受
				PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
				return 0;
			case WAIT_FAILED:
				// 函数调用失败，比如传递了一个无效的句柄   
				AfxMessageBox(_T("警告：体积测量接收数据----阻塞函数返回“WAIT_FAILED”"));
				PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
				return 0;
			}
		}
		else
		{
			AfxMessageBox(_T("警告：体积测量接收数据----前3个字节的数据读取不到"));
			PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
			return 0;
		}
	}
	//因为后面还要接收其他的数据，所有这里不能用清空缓冲区函数
	//PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);//情况缓冲区

	//4、开始提取数据----头3个字节的数据
	//先对接收到的数据做判断
	if (buf_1[0] != 0xdd)
	{
		AfxMessageBox(_T("警告：体积测量----头部标志数据不是“ 0xdd”"));
		PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
		return 0;
	}
	if (nLenOut_1 != 3)
	{
		AfxMessageBox(_T("警告：体积测量----接收到前面3个头部数据,接收的数据数量不是3个"));
		PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
		return 0;
	}
	// 通过buf数字第二和第三个数据，计算出测量包裹的次数.
	cycle_counts = (int(256 * buf_1[1]) + int(buf_1[2]));
	//总的数据数量 ---- 理论上接收缓冲区还剩下的数据的数量
	//8 * cycle_counts + 8 是光幕数据
	//2 * cycle_counts + 4 是编码器时间
	int counts = 8 * cycle_counts + 8 + 2 * cycle_counts + 4;

	//5、开始读取数据----读取后面光幕的实际数据
	OVERLAPPED over_lapped_2;
	memset(&over_lapped_2, 0, sizeof(OVERLAPPED));
	over_lapped_2.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	//在读写之前做清除错误的处理， 和获取缓冲区数据大小。
	DWORD dwError_2;
	COMSTAT cs_2;
	if (!ClearCommError(hCom, &dwError_2, &cs_2))
	{
		AfxMessageBox(_T(" 警告：扫码枪串口清除错误失败----请重新启动软件"));
		PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
		return 0;
	}

	//BYTE buf_2[16384]//之前的大概可以接收1.6m长包裹的数据量
	BYTE buf_2[30000];//最多可以接收大概3m的包裹数据量
	memset(buf_2, 0, sizeof(buf_2));
	//实际读取到的数据数量
	DWORD nLenOut_2 = 0;
	//开始读数据
	BOOL bReadStatus_2 = ReadFile(hCom,
		buf_2,
		counts,
		&nLenOut_2,
		&over_lapped_2);
	if (!bReadStatus_2)
	{
		if (GetLastError() == ERROR_IO_PENDING)	//GetLastError()函数返回ERROR_IO_PENDING,表明串口正在进行读操作
		{
			//使用WaitForSingleObject函数等待，直到读操作完成或延时已达到1秒钟
			DWORD error = WaitForSingleObject(over_lapped_2.hEvent, 1000);	//INFINITE	
			switch (error)
			{
			case WAIT_OBJECT_0:
				//所代表的进程在2秒内结束，关键这个语句可以返回实际收到的数据的数量  
				if (!GetOverlappedResult(hCom, &over_lapped_2, &nLenOut_2, false))
				{
					AfxMessageBox(_T("警告：光幕接收数据----串口出现错误"));
					PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
					return 0;
				}
				//返回值成功，break退出switch语句
				break;
			case WAIT_TIMEOUT:
				// 等待时间超时
				MDate::message(_T("警告：光幕接收数据（后面的包裹数据）----超时----在规定时间里面没有读取到包裹数据，请重新扫描、称重、侧体积"));
				//清空缓冲区---- 否则不会释放串口接收数据的功能，否则后续能发数据，但是不能接受
				PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
				return 0;
			case WAIT_FAILED:
				// 函数调用失败，比如传递了一个无效的句柄   
				AfxMessageBox(_T("警告：光幕接收数据----阻塞函数返回“错误”"));
				PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
				return 0;
			}
		}
		else
		{
			AfxMessageBox(_T("警告：光幕接收数据----包裹实际数据读取不到"));
			PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
			return 0;
		}
	}

	//6、开始提取数据----后面date_count个数量的包裹数据
	//先对接收到的数据做判断
	int n = cycle_counts;
	if (nLenOut_2 != counts)//nLenOut_2他的值好像是GetOverlappedResult(hCom, &over_lapped_2, &nLenOut_2, false)返回的
	{
		CString ss;
		ss.Format(_T("警告：体积测量接收数据----接收到包裹实际数据,不是%d个，而是%d个"), counts, nLenOut_2);
		AfxMessageBox(ss);
		Sleep(200);
		PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
		return 2;
	}

	if (buf_2[2 * n] != 0xff || buf_2[2 * n + 1] != 0xff || buf_2[4 * n + 2] != 0xff || buf_2[4 * n + 2 + 1] != 0xff || buf_2[6 * n + 4] != 0xff || buf_2[6 * n + 4 + 1] != 0xff || buf_2[8 * n + 6] != 0xff || buf_2[8 * n + 6 + 1] != 0xff || buf_2[9 * n + 8] != 0xff || buf_2[9 * n + 8 + 1] != 0xff || buf_2[10 * n + 10] != 0xee || buf_2[10 * n + 10 + 1] != 0xee)
	{
		//AfxMessageBox(_T("警告：光幕接收数据----包裹实际数据的标志字符不对"));
		Sleep(200);
		PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
		return 3;
	}

	//☆☆☆☆☆
	//7、包裹实际数据赋值$$$$$$$$$$$$$$$$$$$$$$$$下一个版本直接将box_date  作为读取串口的缓冲区测试一下效果 ---- ???
	for (int i = 0; i < counts; i++)
	{
		box_date[i] = buf_2[i];
	}

	//8、数据接收完毕，清理一下串口
	PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);//情况缓冲区

	return 1;
}




//***********************************************************光幕*******************************************************
//1、初始化光幕 
BOOL MVolume::init(HANDLE& hCom)
{
	//1、光幕，变频器同时初始化
	//被调用的类
	MCom_port com_port;
	BOOL bl = com_port.judge_port(hCom, volume_init_send, 6, 5000, volume_init_right, 6, 5000);
	if (bl == FALSE)
	{
		AfxMessageBox(_T("警告：光幕初始化--错误"));
		return FALSE;
	}

	return TRUE;
}

//2、光幕开始工作
BOOL MVolume::volume_work(HANDLE hCom)
{
	//被调用的类
	MCom_port com_port;
	//发送数据，光幕开始工作。
	BOOL bl_1 = com_port.send_date(hCom, volume_work_send, 6, 2000, _T("光幕开始工作--发送数据"));
	if (bl_1 == FALSE)
	{
		AfxMessageBox(_T("警告：光幕开始工作，发送数据：失败"));
		return FALSE;
	}
	//光幕开始工作，接收返回数据
	BOOL bl_2 = com_port.receive_for_judge(hCom, volume_work_right, 6, 2000, _T("光幕开始工作----接收握手数据"));
	if (bl_2 == FALSE)
	{
		AfxMessageBox(_T("警告：光幕开始工作，接返回数据：失败"));
		return FALSE;
	}

	return TRUE;
}

//3、光幕暂停工作
BOOL MVolume::volume_suspend(HANDLE hCom)
{
	//被调用的类
	MCom_port com_port;
	//发送数据，光幕暂停工作
	BOOL bl_1 = com_port.send_date(hCom, volume_suspend_send, 6, 2000, _T("光幕暂停工作--发送数据"));
	if (bl_1 == FALSE)
	{
		AfxMessageBox(_T("光幕暂停工作 -- 发送数据：失败"));
		return FALSE;
	}
	//光幕暂停工作，接收返回数据
	BOOL bl_2 = com_port.receive_for_judge(hCom, volume_suspend_right, 6, 2000, _T("光幕暂停工作----接收返回数据"));
	if (bl_2 == FALSE)
	{
		AfxMessageBox(_T("光幕暂停工作 -- 接收返回数据：失败"));
		return FALSE;
	}

	return TRUE;
}

//4、开启光幕电源
BOOL MVolume::volume_start(HANDLE hCom)
{
	//被调用的类
	MCom_port com_port;
	//发送数据，开启光幕电源
	BOOL bl_1 = com_port.send_date(hCom, volume_start_send, 6, 5000, _T("开启光幕电源--发送数据"));
	if (bl_1 == FALSE)
	{
		AfxMessageBox(_T("开启光幕电源 -- 发送数据：失败"));
		return FALSE;
	}
	//开启光幕电源,接收返回数据
	BOOL bl_2 = com_port.receive_for_judge(hCom, volume_start_right, 6, 5000, _T("开启光幕电源----接收返回数据"));
	if (bl_2 == FALSE)
	{
		AfxMessageBox(_T("开启光幕电源 -- 接收返回数据：失败"));
		return FALSE;
	}

	return TRUE;
}

//5、关掉光幕电源 
BOOL MVolume::volume_close(HANDLE hCom)
{
	//被调用的类
	MCom_port com_port;
	//发送数据，关闭光幕电源
	BOOL bl_1 = com_port.send_date(hCom, volume_close_send, 6, 5000, _T("光幕关闭电源--发送握手数据"));
	if (bl_1 == FALSE)
	{
		AfxMessageBox(_T("光幕关闭电源 -- 发送握手数据：失败"));
		return FALSE;
	}
	//关闭光幕电源,接收返回数据
	BOOL bl_2 = com_port.receive_for_judge(hCom, volume_close_right, 6, 5000, _T("关闭光幕电源----接收返回数据"));
	if (bl_2 == FALSE)
	{
		AfxMessageBox(_T("关闭光幕电源 -- 接收返回数据：失败"));
		return FALSE;
	}

	return TRUE;
}

//6、光幕自检
BOOL MVolume::volume_check(HANDLE hCom)
{
	//被调用的类
	MCom_port com_port;
	//光幕自检，发送数据
	BOOL bl_1 = com_port.send_date(hCom, volume_check_send, 6, 2000, _T("光幕自检--发送数据"));
	if (bl_1 == FALSE)
	{
		AfxMessageBox(_T("光幕自检 -- 发送数据：失败"));
		return FALSE;
	}
	//光幕自检，接收返回数据
	BOOL bl_2 = com_port.receive_for_judge(hCom, volume_check_right, 6, 2000, _T("光幕自检----接收返回数据"));
	if (bl_2 == FALSE)
	{
		AfxMessageBox(_T("光幕自检 -- 接收返回数据：失败,光幕自检通不过"));
		return FALSE;
	}

	return TRUE;
}

//7、清洁光幕
BOOL MVolume::clean_screen(HANDLE hCom)
{
	//1、被调用的类
	MCom_port com_port;
	//2、发送数据，清洁光幕
	BOOL bl_1 = com_port.send_date(hCom, clean_work, 6, 2000, _T("发送数据 -- 清洁光幕"));
	if (bl_1 == FALSE)
	{
		AfxMessageBox(_T("警告：发送数据，清洁光幕：失败"));
		return FALSE;
	}
	//3、清洁光幕，接收返回数据
	BOOL bl_2 = com_port.receive_for_judge(hCom, clean_return, 6, 22000, _T("清洁光幕 -- 接收返回数据"));
	if (bl_2 == FALSE)
	{
		AfxMessageBox(_T("警告：清洁光幕，接收返回数据：失败"));
		return FALSE;
	}

	return TRUE;
}

//8、重新要求光幕发送包裹数据给上位机
BOOL MVolume::send_date_again(HANDLE hCom)
{
	//1、被调用的类
	MCom_port com_port;
	//2、发送数据，重新发送数据
	BOOL bl_1 = com_port.send_date(hCom, send_date_second, 6, 500, _T("发送数据 -- 清洁光幕"));
	if (bl_1 == FALSE)
	{
		AfxMessageBox(_T("警告：发送数据，清洁光幕：失败"));
		return FALSE;
	}

	return TRUE;
}


//***********************************************************灯泡*******************************************************
//1、灯泡绿色，随时准备工作
 BOOL MVolume::bulb_green(HANDLE hCom)
 {
	 //1、被调用的类
	 MCom_port com_port;
	 //2、发送数据，灯泡变绿
	 BOOL bl_1 = com_port.send_date(hCom, bulb_green_send, 6, 1000, _T("发送数据 -- 灯泡变绿"));
	 if (bl_1 == FALSE)
	 {
		 AfxMessageBox(_T("警告：发送数据，灯泡变绿：失败"));
		 return FALSE;
	 }
	 //3、灯泡变绿，接收返回数据
	 BOOL bl_2 = com_port.receive_for_judge(hCom, bulb_green_receive, 6, 1000, _T("灯泡变绿 -- 接收返回数据"));
	 if (bl_2 == FALSE)
	 {
		 AfxMessageBox(_T("警告：灯泡变绿，接收返回数据：失败"));
		 return FALSE;
	 }

	 return TRUE;
 }

 //2、灯泡黄色，工作正在进行中
 BOOL MVolume::bulb_yellow(HANDLE hCom)
 {
	 //1、被调用的类
	 MCom_port com_port;
	 //2、发送数据，灯泡变黄
	 BOOL bl_1 = com_port.send_date(hCom, bulb_yellow_send, 6, 1000, _T("发送数据 -- 灯泡变黄"));
	 if (bl_1 == FALSE)
	 {
		 AfxMessageBox(_T("警告：发送数据，灯泡变黄：失败"));
		 return FALSE;
	 }
	 //3、灯泡变黄，接收返回数据
	 BOOL bl_2 = com_port.receive_for_judge(hCom, bulb_yellow_receive, 6, 1000, _T("灯泡变黄 -- 接收返回数据"));
	 if (bl_2 == FALSE)
	 {
		 AfxMessageBox(_T("警告：灯泡变黄，接收返回数据：失败"));
		 return FALSE;
	 }

	 return TRUE;
 }

 //3、灯泡红色，工作异常
 BOOL MVolume::bulb_red(HANDLE hCom)
 {
	 //1、被调用的类
	 MCom_port com_port;
	 //2、发送数据，灯泡变红
	 BOOL bl_1 = com_port.send_date(hCom, bulb_red_send, 6, 1000, _T("发送数据 -- 灯泡变红"));
	 if (bl_1 == FALSE)
	 {
		 AfxMessageBox(_T("警告：发送数据，灯泡变红：失败"));
		 return FALSE;
	 }
	 //3、灯泡变红，接收返回数据
	 BOOL bl_2 = com_port.receive_for_judge(hCom, bulb_red_receive, 6, 1000, _T("灯泡变红 -- 接收返回数据"));
	 if (bl_2 == FALSE)
	 {
		 AfxMessageBox(_T("警告：灯泡变红，接收返回数据：失败"));
		 return FALSE;
	 }

	 return TRUE;
 }

 //4、关闭灯泡
 BOOL MVolume::bulb_close(HANDLE hCom)
 {
	 //1、被调用的类
	 MCom_port com_port;
	 //2、发送数据，关闭灯泡
	 BOOL bl_1 = com_port.send_date(hCom, bulb_close_send, 6, 1000, _T("发送数据 -- 关闭灯泡"));
	 if (bl_1 == FALSE)
	 {
		 AfxMessageBox(_T("警告：发送数据，关闭灯泡：失败"));
		 return FALSE;
	 }
	 //3、关闭灯泡，接收返回数据
	 BOOL bl_2 = com_port.receive_for_judge(hCom, bulb_close_receive, 6, 1000, _T("关闭灯泡 -- 接收返回数据"));
	 if (bl_2 == FALSE)
	 {
		 AfxMessageBox(_T("警告：关闭灯泡，接收返回数据：失败"));
		 return FALSE;
	 }

	 return TRUE;
 }

//***********************************************************电机*******************************************************
//1、电机启动
BOOL MVolume::machine_work(HANDLE hCom)
{
	//被调用的类
	MCom_port com_port;
	//发送握手数据
	BOOL bl_1 = com_port.send_date(hCom, machine_work_send, 6, 2000, _T("发送数据，为了启动电机"));
	if (bl_1 == FALSE)
	{
		AfxMessageBox(_T("警告：发送数据，为了启动电机 ---- 数据发送失败"));
		return FALSE;
	}

	//接收开始工作的握手返回数据
	BOOL bl_2 = com_port.receive_for_judge(hCom, machine_work_right, 6, 2000, _T("接收数据，为了判断电机是否正常启动"));
	if (bl_2 == FALSE)
	{
		AfxMessageBox(_T("警告：接收数据，为了判断电机是否正常启动 ---- 接收数据失败"));
		return FALSE;
	}

	machine_state = 1;
	return TRUE;
}

//2、电机暂停
BOOL MVolume::machine_close(HANDLE hCom)
{
	//1、被调用的类
	MCom_port com_port;
	//2、发送数据，关闭电机
	BOOL bl_1 = com_port.send_date(hCom, machine_suspend_send, 6, 1000, _T("发送数据 -- 关闭电机"));
	if (bl_1 == FALSE)
	{
		AfxMessageBox(_T("警告：发送数据，关闭电机：失败"));
		return FALSE;
	}

	//3、关闭电机，接收返回数据
	BOOL bl_2 = com_port.receive_for_judge(hCom, machine_suspend_right, 6, 1000, _T("关闭电机 -- 接收返回数据"));
	if (bl_2 == FALSE)
	{
		AfxMessageBox(_T("警告：关闭电机 -- 接收返回数据：失败"));
		return FALSE;
	}

	machine_state = 0;
	return TRUE;
}

//3、设置电机的速度 ---- 通过设置变频器的频率
BOOL MVolume::make_hz(HANDLE hCom)
{
	//1、被调用的类
	MCom_port com_port;
	//2、发送握手数据
	BOOL bl_1 = com_port.send_date(hCom, machine_spend_send, 6, 2000, _T("发送数据，为了设置变频器频率"));
	if (bl_1 == FALSE)
	{
		AfxMessageBox(_T("警告：发送数据，为了设置变频器 ---- 数据发送失败"));
		return FALSE;
	}
	//3、接收开始工作的握手返回数据
	BOOL bl_2 = com_port.receive_for_judge(hCom, machine_spend_right, 6, 2000, _T("接收数据，为了判断变频器设置是非返回预定数据"));
	if (bl_2 == FALSE)
	{
		AfxMessageBox(_T("警告：接收数据，为了判断变频器设置是否返回预定数据 ---- 接收数据失败"));
		return FALSE;
	}

	return TRUE;
}

//**********************************************************编码器******************************************************
//1、光幕检查
BOOL MVolume::encoder_check(HANDLE hCom)
{
	//被调用的类
	MCom_port com_port;
	//编码器自检，发送数据
	BOOL bl_1 = com_port.send_date(hCom, encoder_send, 6, 2000, _T("编码器自检--发送数据"));
	if (bl_1 == FALSE)
	{
		AfxMessageBox(_T("编码器自检 -- 发送数据：失败"));
		return FALSE;
	}
	//编码器自检，接收返回数据
	BOOL bl_2 = com_port.receive_for_judge(hCom, encoder_receive, 6, 4000, _T("编码器自检----接收返回数据"));
	if (bl_2 == FALSE)
	{
		AfxMessageBox(_T("编码器自检 -- 接收返回数据：失败,编码器自检通不过"));
		return FALSE;
	}

	return TRUE;
}














//***********************************************************体积*******************************************************
//1、解析包裹数据
void MVolume::parse_date()
{
	/*
	1、解析出长、宽数组
	2、解析出高的数组
	3、解析出输送带行走脉冲数的数组 ---- 用来计算包裹的总长度信息
	*/
	int n = cycle_counts;
	//*************************************************光幕***************************************************
	//光幕左右2边的点
	for (int i = 0; i < n; i++)
	{
		len_width_begin[i] = (int)(256 * box_date[2 * i]) + (int)box_date[2 * i + 1];
		//MDate::message(len_width_begin[i]);
	}
	for (int i = 0; i < n; i++)
	{
		len_width_end[i] = (int)(256 * box_date[2 * n + 2 + 2 * i]) + (int)box_date[2 * n + 2 + 2 * i + 1];
	}
	//光幕上下的点
	for (int i = 0; i < n; i++)
	{
		height_up_begin[i] = (int)(256 * box_date[4 * n + 4 + 2 * i]) + (int)box_date[4 * n + 4 + 2 * i + 1];
	}
	for (int i = 0; i < n; i++)
	{
		height_down_end[i] = (int)(256 * box_date[6 * n + 6 + 2 * i]) + (int)box_date[6 * n + 6 + 2 * i + 1];
	}

	//*************************************************编码器*************************************************
	//前面的输送带脉冲数量
	for (int i = 0; i < n; i++)
	{
		//前面的输送带脉冲数量 ---- 因为数据是BYTE，要转换成int， 所以数据的高位要*256
		pulse_front[i] = (int)box_date[8 * n + 8 + i];
	}
	//后面的输送带脉冲数量
	for (int i = 0; i < n; i++)
	{
		//前面的输送带脉冲数量 ---- 因为数据是BYTE，要转换成int， 所以数据的高位要*256
		pulse_behind[i] = (int)box_date[9 * n + 10 + i];
	}
}

//2、将2个编码器的数据合二为一；对数组pulse[]赋值
void MVolume::get_encoder_date()
{
	/*
	前面编码器一半的数据 + 后面编码器一半的数据 = 编码器合成的数据
	*/
	//编码器第一个数据不一定是0，看如何定义
	int n = cycle_counts / 2;//以一半的扫描周期作为临界点
	for (int i = 0; i < cycle_counts; i++)
	{
		if (i == 0)
		{
			pulse[i] = pulse_front[i];
		}
		else
		{
			if (i <= n)
			{
				pulse[i] = pulse[i-1] + pulse_front[i];
			}
			else
			{
				pulse[i] = pulse[i - 1] + pulse_behind[i];
			}
		}
	}
}

//3、编码器数据异常的判断
BOOL MVolume::filter_encoder()
{
	//1、包裹总长度不能＞3m
	return TRUE;
}

//4、得到最小外接矩形的参数
RotatedRect MVolume::get_min_rect()
{
	/*
	1、第一次计算包裹长宽，对目的是为了切角用（切角的时候需要这个函数输出的结果）,所以对精度要求不高
	*/
	//vector是opencv官方教程的数据格式，
	vector<Point> points;
	//x轴移动的距离
	int x_run = 0;
	//定义一个点变量--包裹扫码一次上下（或者说左右）的2个点的变量
	Point point_begin;
	Point point_end;

	// ---- 把包裹所有的点压入points中
	for (int i = 0; i < cycle_counts; i++)
	{
		//这个数据不压栈
		if (len_width_end[i] == 0 && len_width_begin[i] == 0)
		{
			continue;
		}

		//计算在x轴行走的距离
		x_run = (int)(pulse[i] * pulse_length);

		//开始压入数据
		point_begin.x = 100 + x_run;		//这个100是相对于mat的二位矩阵（0，0）的偏移，矩阵一个像素相当于1mm
		point_begin.y = 100 + len_width_begin[i] * 3;
		points.push_back(point_begin);
		//压入下面的点
		point_end.x = 100 + x_run;
		point_end.y = 100 + len_width_end[i] * 3;
		points.push_back(point_end);
	}

	//计算最小外接矩形 ---- 这个是返回值
	RotatedRect out_rectPoint = minAreaRect(points);

	return out_rectPoint;
}

//5、对图像的4个角的数据进行切除，并改变原始数据
void MVolume::cut_corner()
{
	//1、找到4个对应的顶点
	RotatedRect cut_rectPoint = get_min_rect();//得到最小外接矩形
	//将rectPoint变量中存储的坐标值放到 fourPoint的数组中  
	Point2f fourPoint2f[4];
	cut_rectPoint.points(fourPoint2f);

	//2、对切角进行判断
	//cut_date
	//float length_f = (cut_rectPoint.size.height > cut_rectPoint.size.width) ? cut_rectPoint.size.height : cut_rectPoint.size.width;
	int width_i = (int)((cut_rectPoint.size.height > cut_rectPoint.size.width) ? cut_rectPoint.size.width : cut_rectPoint.size.height);
	if (width_i <= 100)
	{
		cut_date = 20;
	}
	else if (width_i > 100 && width_i <= 200)
	{
		cut_date = 40;
	}
	else if (width_i > 200 && width_i <= 300)
	{
		cut_date = 60;
	}
	else
	{
		cut_date = 80;
	}


	//3、做4次循环，分别进行切角
	for (int i = 0; i < 4; i++)
	{
		//1、得到参考点和对应的2个相邻的点
		CPoint p_reference;//参考点
		CPoint p_neighbor1, p_neighbor2;//相邻的点

		p_reference.x = (int)fourPoint2f[i].x;
		p_reference.y = (int)fourPoint2f[i].y;

		//相邻点的数组标号
		int j, f;
		j = i + 1;//顺时针相邻的点 ---- p_neighbor1
		f = i - 1;//逆时针相邻的点 ---- p_neighbor2

		if (j == 4)
		{
			j = 0;
		}
		if (f == -1)
		{
			f = 3;
		}
		//顺时针相邻的点
		p_neighbor1.x = (int)fourPoint2f[j].x;
		p_neighbor1.y = (int)fourPoint2f[j].y;
		//逆时针相邻的点
		p_neighbor2.x = (int)fourPoint2f[f].x;
		p_neighbor2.y = (int)fourPoint2f[f].y;


		//2、通过3个点得到一次函数的直线上面的2个点
		MAlgorithm alg;
		CPoint line_1, line_2;//一次函数的2个点
		line_1 = alg.get_point(p_reference, p_neighbor1, cut_date);//顺时针对应直线的点
		line_2 = alg.get_point(p_reference, p_neighbor2, cut_date);//逆时针对应直线的点

																   //3、通过2个点得到一条直线
		MPoint line;
		line = alg.get_line(line_1, line_2);

		//4、参考点，2个相邻点x轴上的，最大值和最小值，计算出来
		int x_min, x_max;
		//第一次比较
		if (p_reference.x >= p_neighbor1.x)
		{
			x_min = p_neighbor1.x;
			x_max = p_reference.x;
		}
		else
		{
			x_min = p_reference.x;
			x_max = p_neighbor1.x;
		}
		//第二次比较出最小值
		if (x_min >= p_neighbor2.x)
		{
			x_min = p_neighbor2.x;
		}
		else
		{
			//不做处理
		}
		//第二次比较出最大值
		if (x_max <= p_neighbor2.x)
		{
			x_max = p_neighbor2.x;
		}
		else
		{
			//不做处理
		}

		//5、开始切角
		for (int i = 0; i < cycle_counts; i++)
		{
			//1、计算出当前周期2个点的坐标
			//一次扫描周期上的2个点
			CPoint p_begin, p_end;
			//当前点X轴的坐标----计算在x轴行走的距离
			int x = (int)(pulse[i] * pulse_length) + 100;

			//2、如果x比min还小，比max还大。我们就不做处理提供执行效率
			if (x < (x_min - 2))//-1/-2 都可以，保险起见-2
			{
				continue;
			}
			if (x > (x_max + 2))
			{
				continue;
			}

			//3、判断2个点和参考点的关系
			//开始的点、结束的点
			p_begin.x = x;
			p_begin.y = 100 + len_width_begin[i] * 3;
			//结束的点
			p_end.x = x;
			p_end.y = 100 + len_width_end[i] * 3;

			BOOL bl_1 = alg.judge_boundary(p_begin, p_reference, line);
			BOOL bl_2 = alg.judge_boundary(p_end, p_reference, line);
			if (bl_1 == TRUE && bl_2 == TRUE)
			{
				len_width_begin[i] = 0;
				len_width_end[i] = 0;
			}
			else
			{
				//当直线的没有斜率的时候，要么都是返回true，要么都是返回false。当没有斜率的时候，这里地方都是false，可以不要做处理
				if (bl_1 == TRUE)
				{
					//X轴上对应Y轴上的点就在直线上
					int y_begin = (int)(line.k*x + line.b);
					len_width_begin[i] = (int)((y_begin - 100) / 3);
				}
				if (bl_2 == TRUE)
				{
					int y_end = (int)(line.k*x + line.b);
					len_width_end[i] = (int)((y_end - 100) / 3);
				}
			}
		}
	}
}

//6、切角后，原始数据不变，得到切角后的数据
void MVolume::cut_corner_date()
{
	/*
	1.用原始数据画图，切角数据计算长宽高
	*/
	//1、把原始数据赋值给切角的数组
	for (int i = 0; i < cycle_counts; i++)
	{
		cut_len_width_begin[i] = len_width_begin[i];
		cut_len_width_end[i] = len_width_end[i];
	}

	//2、找到4个对应的顶点
	RotatedRect cut_rectPoint = get_min_rect();//得到最小外接矩形
	//将rectPoint变量中存储的坐标值放到 fourPoint的数组中  
	Point2f fourPoint2f[4];
	cut_rectPoint.points(fourPoint2f);

	//2、对切角进行判断 ---- 就是要切多大的角
	//float length_f = (cut_rectPoint.size.height > cut_rectPoint.size.width) ? cut_rectPoint.size.height : cut_rectPoint.size.width;
	int width_i = (int)((cut_rectPoint.size.height > cut_rectPoint.size.width) ? cut_rectPoint.size.width : cut_rectPoint.size.height);
	if (width_i <= 100)
	{
		cut_date = 20;
	}
	else if (width_i > 100 && width_i <= 200)
	{
		cut_date = 40;
	}
	else if (width_i > 200 && width_i <= 300)
	{
		cut_date = 60;
	}
	else
	{
		cut_date = 80;
	}

	//3、做4次循环，分别进行切角
	for (int i = 0; i < 4; i++)
	{
		//1、得到参考点和对应的2个相邻的点
		CPoint p_reference;//参考点
		CPoint p_neighbor1, p_neighbor2;//相邻的点

		p_reference.x = (int)fourPoint2f[i].x;
		p_reference.y = (int)fourPoint2f[i].y;

		//相邻点的数组标号
		int j, f;
		j = i + 1;//顺时针相邻的点 ---- p_neighbor1
		f = i - 1;//逆时针相邻的点 ---- p_neighbor2

		if (j == 4)
		{
			j = 0;
		}
		if (f == -1)
		{
			f = 3;
		}
		//顺时针相邻的点
		p_neighbor1.x = (int)fourPoint2f[j].x;
		p_neighbor1.y = (int)fourPoint2f[j].y;
		//逆时针相邻的点
		p_neighbor2.x = (int)fourPoint2f[f].x;
		p_neighbor2.y = (int)fourPoint2f[f].y;


		//2、通过3个点得到一次函数的直线上面的2个点
		MAlgorithm alg;
		CPoint line_1, line_2;//一次函数的2个点
		line_1 = alg.get_point(p_reference, p_neighbor1, cut_date);//顺时针对应直线的点
		line_2 = alg.get_point(p_reference, p_neighbor2, cut_date);//逆时针对应直线的点

		cut_point[i][0][0] = line_1.x;
		cut_point[i][0][1] = line_1.y;

		cut_point[i][1][0] = line_2.x;
		cut_point[i][1][1] = line_2.y;

		//3、通过2个点得到一条直线
		MPoint line;
		line = alg.get_line(line_1, line_2);

		//4、参考点，2个相邻点x轴上的，最大值和最小值，计算出来
		int x_min, x_max;
		//第一次比较
		if (p_reference.x >= p_neighbor1.x)
		{
			x_min = p_neighbor1.x;
			x_max = p_reference.x;
		}
		else
		{
			x_min = p_reference.x;
			x_max = p_neighbor1.x;
		}
		//第二次比较出最小值
		if (x_min >= p_neighbor2.x)
		{
			x_min = p_neighbor2.x;
		}
		else
		{
			//不做处理
		}
		//第二次比较出最大值
		if (x_max <= p_neighbor2.x)
		{
			x_max = p_neighbor2.x;
		}
		else
		{
			//不做处理
		}

		//5、开始切角
		for (int i = 0; i < cycle_counts; i++)
		{
			//1、计算出当前周期2个点的坐标
			//一次扫描周期上的2个点
			CPoint p_begin, p_end;
			//当前点X轴的坐标----计算在x轴行走的距离
			int x = (int)(pulse[i] * pulse_length) + 100;

			//2、如果x比min还小，比max还大。我们就不做处理提供执行效率
			if (x < (x_min - 2))//-1/-2 都可以，保险起见-2
			{
				continue;
			}
			if (x > (x_max + 2))
			{
				continue;
			}

			//3、判断2个点和参考点的关系
			//开始的点、结束的点
			p_begin.x = x;
			p_begin.y = 100 + len_width_begin[i] * 3;
			//结束的点
			p_end.x = x;
			p_end.y = 100 + len_width_end[i] * 3;

			BOOL bl_1 = alg.judge_boundary(p_begin, p_reference, line);
			BOOL bl_2 = alg.judge_boundary(p_end, p_reference, line);
			if (bl_1 == TRUE && bl_2 == TRUE)
			{
				cut_len_width_begin[i] = 0;
				cut_len_width_end[i] = 0;
			}
			else
			{
				//当直线的没有斜率的时候，要么都是返回true，要么都是返回false。当没有斜率的时候，这里地方都是false，可以不要做处理
				if (bl_1 == TRUE)
				{
					//X轴上对应Y轴上的点就在直线上
					int y_begin = (int)(line.k*x + line.b);
					cut_len_width_begin[i] = (int)((y_begin - 100) / 3);
				}
				if (bl_2 == TRUE)
				{
					int y_end = (int)(line.k*x + line.b);
					cut_len_width_end[i] = (int)((y_end - 100) / 3);
				}
			}
		}
	}
}

//7、计算出包裹的高度信息
void MVolume::get_height()
{
	//1、高的最大值 ---- 最后返回的数据
	int height_max = 0;

	//开始划图像 ---- 长宽的图像
	int height_max_count = 0;
	for (int i = 0; i < cycle_counts; i++)
	{
		//包裹高的值
		int height_1 = (height_down_end[i] - height_up_begin[i]) * 3;
		//计算出高的最大值
		if (height_max < height_1)
		{
			height_max = height_1;
			height_max_count = 1;
		}
		if (height_max == height_1)
		{
			height_max_count++;
		}
	}

	//对高的数据，进行进位处理
	if (height_max_count <= 40)
	{
		height_max = height_max - 3;
		//height_max = height_max;
	}
	if (height_max_count <= 60 && height_max_count > 40)
	{
		height_max = height_max - 2;
		//height_max = height_max;
	}
	if (height_max_count <= 80 && height_max_count > 60)
	{
		height_max = height_max - 1;
		//height_max = height_max;
	}

	height = height_max;
}

//8、计算出包裹长宽的信息/切角后的
void MVolume::get_length_width_cut()
{
	//1、压入数据，得到最小外接矩形的数据
	//vector是opencv官方教程的数据格式，
	vector<Point> points;
	//定义一个点变量----包裹扫描一次上下（或者说左右）的2个点的变量
	Point point_begin;
	Point point_end;
	//x轴移动的距离
	int x_run = 0;

	//把包裹所有的点压入points中
	for (int i = 0; i < cycle_counts; i++)
	{
		//1、这个数据不压栈
		if (cut_len_width_begin[i] == 0 && cut_len_width_end[i] == 0)
		{
			continue;
		}
		//2、计算在x轴行走的距离
		x_run = (int)(pulse[i] * pulse_length);
		//3、开始压入数据
		point_begin.x = 100 + x_run;		//这个100是相对于mat的二位矩阵（0，0）的偏移，矩阵一个像素相当于1mm
		point_begin.y = 100 + cut_len_width_begin[i] * 3;
		points.push_back(point_begin);
		//压入下面的点
		point_end.x = 100 + x_run;
		point_end.y = 100 + cut_len_width_end[i] * 3;
		points.push_back(point_end);
	}
	//计算最小外接矩形
	RotatedRect rectPoint = minAreaRect(points);
	//给其他函数用的变量
	cut_rectPoint = rectPoint;

	//显示出包裹长宽的数据--以全局变量的形式返回长宽数据
	float length_f = (rectPoint.size.height > rectPoint.size.width) ? rectPoint.size.height : rectPoint.size.width;
	float width_f = (rectPoint.size.height > rectPoint.size.width) ? rectPoint.size.width : rectPoint.size.height;

	length = (int)length_f;
	width = (int)width_f;
}

//9、画图像/用切角后的数据
void  MVolume::paint_image(CPaintDC& dc)
{
	//1、坐标申明（用于划线的坐标点）
	int start_x, start_y;
	int end_x, end_y;
	//2、定义画笔
	CPen pn(PS_SOLID, 1, RGB(220, 220, 220));
	CPen *pOldPen = dc.SelectObject(&pn);
	//3、获取画图的参考坐标点
	MCoordinate layer;
	//第一个，和第二个数据长宽的参考点， 第三和第四个是高的参考点
	int reference[4] = { 0,0,0,0 };
	//通过函数给reference赋值
	layer.set_reference(reference);
	//4.画:长/宽/高
	//x轴方向坐标（同一个周期x方向的值是一样的）
	int x_mm;
	//y轴方向坐标
	int add1;//开始的点
	int add2;//结束的点
	//包裹移动一半的时候所扫描的次数
	//int n = cycle_counts / 2;

	//循环的次数，是包裹扫描的次数。
	//★★★ ---- 包裹实际的长度，也是X轴实际运动的距离	，x轴总的像素的数量（包裹x轴运行的距离）
	int n_pixel = (int)(pulse[cycle_counts - 1] * pulse_length);
	//等比例缩放
	n_pixel = (int)((584 / 840.0)*n_pixel);
	
	//开始划图像 ---- 长宽的图像
	for (int i = 0; i < cycle_counts; i++)
	{
		//x轴方向的位移
		x_mm = (int)(pulse[i] * pulse_length);
		//y轴方向的位移
		add1 = len_width_begin[i] * 3;	//一个点对应的距离是3mm（由硬件决定的）
		add2 = len_width_end[i] * 3;	//一个点对应的距离是3mm（由硬件决定的）

		//缩放后的数据
		add1 = (int)((584 / 840.0)*add1);
		add2 = (int)((584 / 840.0)*add2);
		x_mm = (int)((584 / 840.0)*x_mm);
		//--------------------画长宽--------------------
		//对画线对应的坐标赋值
		start_x = (reference[0] - n_pixel) + x_mm;
		start_y = reference[1] + add1 + 3;
		end_x = (reference[0] - n_pixel) + x_mm;
		end_y = reference[1] + add2 + 3;

		//开始划线 ---- 长，宽
		dc.MoveTo(start_x, start_y);
		dc.LineTo(end_x, end_y);

		//--------------------画高--------------------
		//包裹高的值
		int height_1 = (height_down_end[i] - height_up_begin[i]) * 3;
		//长宽分配的像素1140*(840/(840+800)) = 584
		//高分配的像素1140*(800/(840+800)) = 556
		//因为现实高的区域没有800个像素，我们将高的区域要分成800个像素，将height_1分成800个单位
		//height_pixel就变成实际所拥有的像素点
		int height_pixel = (int)((556.0 / 800.0)*height_1);

		//给坐标赋值，起点是最下面的线条。
		//start_x值不变
		start_y = reference[3];
		//end_x值不变
		end_y = reference[3] - height_pixel;

		//开始划线 ---- 高
		dc.MoveTo(start_x, start_y);
		dc.LineTo(end_x, end_y);
	}
}

//10、画包裹长、宽、高的外框/用切角后的数据
void  MVolume::paint_outline(CPaintDC& dc)
{
	//1、定义基本变量
	//获取参考点的类
	MCoordinate layer;
	//定义一个存储以上四个点的坐标的变量  ，这里一定是浮点型变量。
	Point2f fourPoint2f[4];
	//将rectPoint变量中存储的坐标值放到 fourPoint的数组中  
	cut_rectPoint.points(fourPoint2f);

	//2、定义画笔，开始画外接矩形和高
	CPen pn1(PS_SOLID, 2, RGB(255, 0, 0));
	CPen *pOldPen1 = dc.SelectObject(&pn1);

	//*********************************************开始画高*********************************************
	int h_startx, h_starty;
	int h_endx, h_endy;
	//第一个，和第二个数据长宽的参考点， 第三和第四个是高的参考点
	int reference[4] = { 0,0,0,0 };
	//通过函数给reference赋值
	layer.set_reference(reference);
	//长宽分配的像素1140*(840/(840+800)) = 584
	//高分配的像素1140*(800/(840+800)) = 556
	//height_1是显示高所拥有的像素
	int height_1 = 556;
	//因为现实高的区域没有800个像素，我们将高的区域要分成800个像素，将height_1分成800个单位
	int height_pixel = (int)((556.0 / 800.0)*height);
	//起点
	h_startx = reference[2];
	h_starty = reference[3] - height_pixel;
	//终点
	//x轴总的像素的数量
	int n_pixel = (int)(pulse[cycle_counts - 1] * pulse_length);
	n_pixel = (int)((584 / 840.0)*n_pixel);

	h_endx = h_startx - n_pixel;//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
	h_endy = h_starty;
	//画图形
	dc.MoveTo(h_startx, h_starty);
	dc.LineTo(h_endx, h_endy);
	//*********************************************开始画最小外接矩形*********************************************
	//位移量的补偿
	//x ---- 偏移的像素
	int vx = reference[0] - n_pixel - (int)((584 / 840.0) * 100);
	//y ---- 偏移的像素
	//-100+49+3
	int vy = -(int)((584 / 840.0) * 100) + 49 + 3;

	//将尺寸变换成对应的像素值
	int fx = GetSystemMetrics(SM_CXSCREEN);
	for (int i = 0; i < 4; i++)
	{
		//fourPoint2f[i].x = (float)((fourPoint2f[i].x * fx) / 1920.0);
		//fourPoint2f[i].y = (float)((fourPoint2f[i].y * fx) / 1920.0);
		fourPoint2f[i].x = (float)((584.0 / 840)*fourPoint2f[i].x);
		fourPoint2f[i].y = (float)((584.0 / 840)*fourPoint2f[i].y);
	}
	//开始画框框
	for (int i = 0; i < 3; i++)
	{
		dc.MoveTo((int)fourPoint2f[i].x + vx, (int)fourPoint2f[i].y + vy);
		dc.LineTo((int)fourPoint2f[i + 1].x + vx, (int)fourPoint2f[i + 1].y + vy);
	}
	dc.MoveTo((int)fourPoint2f[0].x + vx, (int)fourPoint2f[0].y + vy);
	dc.LineTo((int)fourPoint2f[3].x + vx, (int)fourPoint2f[3].y + vy);

	//画切角
	if (MDate::cut_corner == 1)
	{
		CPen pn2(PS_SOLID, 3, RGB(255, 0, 0));
		dc.SelectObject(&pn2);
		CPoint point_1, point_2;
		for (int i = 0; i < 4; i++)
		{
			point_1.x = (int)((584.0 / 840)*cut_point[i][0][0]);
			point_1.y = (int)((584.0 / 840)*cut_point[i][0][1]);

			point_2.x = (int)((584.0 / 840)*cut_point[i][1][0]);
			point_2.y = (int)((584.0 / 840)*cut_point[i][1][1]);

			dc.MoveTo(point_1.x + vx, point_1.y + vy);
			dc.LineTo(point_2.x + vx, point_2.y + vy);
		}
	}
	else
	{
		//不做操作
	}
}






























//5、画出包裹长、宽、高的图像,并且返回高的最大值
void MVolume::show_image(CPaintDC& dc)
{
	//1、高的最大值 ---- 最后返回的数据
	int height_max = 0;

	//2、坐标申明（用于划线的坐标点）
	int start_x, start_y;
	int end_x, end_y;
	//---- 定义画笔
	CPen pn(PS_SOLID, 1, RGB(220, 220, 220));
	CPen *pOldPen = dc.SelectObject(&pn);
	//3、获取画图的参考坐标点
	MCoordinate layer;
	//第一个，和第二个数据长宽的参考点， 第三和第四个是高的参考点
	int reference[4] = { 0,0,0,0 };
	//通过函数给reference赋值
	layer.set_reference(reference);

	//4.画----长----宽----高
	//x轴方向坐标（同一个周期x方向的值是一样的）
	int x_mm;
	//y轴方向坐标
	int add1;//开始的点
	int add2;//结束的点
	//包裹移动一半的时候所扫描的次数
	int n = cycle_counts / 2;

	//循环的次数，是包裹扫描的次数。
	//★★★ ---- 包裹实际的长度，也是X轴实际运动的距离	，x轴总的像素的数量（包裹x轴运行的距离）
	int n_pixel = (int)(pulse[cycle_counts - 1] * pulse_length);





	n_pixel = (int)((584 / 840.0)*n_pixel);
	//高的数据，符合最高点的数量 ---- 目的是对高进行取舍操作用的
	int height_max_count = 0;

	//开始划图像 ---- 长宽的图像
	for (int i = 0; i < cycle_counts; i++)
	{
		//x轴方向的位移
		x_mm = (int)(pulse[i] * pulse_length);
		//y轴方向的位移
		add1 = len_width_begin[i] * 3;	//一个点对应的距离是3mm（由硬件决定的）
		add2 = len_width_end[i] * 3;	//一个点对应的距离是3mm（由硬件决定的）

		//缩放后的数据
		add1 = (int)((584 / 840.0)*add1);
		add2 = (int)((584 / 840.0)*add2);
		x_mm = (int)((584 / 840.0)*x_mm);
		//--------------------画长宽--------------------
		//对画线对应的坐标赋值
		start_x = (reference[0] - n_pixel) + x_mm;
		start_y = reference[1] + add1 + 3;
		end_x = (reference[0] - n_pixel) + x_mm;
		end_y = reference[1] + add2 + 3;

		//等比例缩小

		//长宽分配的像素1140*(840/(840+800)) = 584
		//高分配的像素1140*(800/(840+800)) = 556
		//int height_pixel = (int)((height_1 / 800.0)*height);
		//start_x = (int)((584 / 840.0)*start_x);
		//start_y = (int)((584 / 840.0)*start_y);
		//end_x   = (int)((584 / 840.0)*end_x);
		//end_y   = (int)((584 / 840.0)*end_y);

		//开始划线 ---- 长，宽
		dc.MoveTo(start_x, start_y);
		dc.LineTo(end_x, end_y);

		//--------------------画高--------------------
		//包裹高的值
		int height_1 = (height_down_end[i] - height_up_begin[i]) * 3;
		//计算出高的最大值
		if (height_max<height_1)
		{
			height_max = height_1;
			height_max_count = 1;
		}
		if (height_max == height_1)
		{
			height_max_count++;
		}

		//长宽分配的像素1140*(840/(840+800)) = 584
		//高分配的像素1140*(800/(840+800)) = 556
		//因为现实高的区域没有800个像素，我们将高的区域要分成800个像素，将height_1分成800个单位
		//height_pixel就变成实际所拥有的像素点
		int height_pixel = (int)((556.0 / 800.0)*height_1);

		//给坐标赋值，起点是最下面的线条。
		//start_x值不变
		start_y = reference[3];
		//end_x值不变
		end_y = reference[3] - height_pixel;

		//开始划线 ---- 高
		dc.MoveTo(start_x, start_y);
		dc.LineTo(end_x, end_y);
	}

	//对高的数据，进行进位处理
	if (height_max_count <= 40)
	{
		height_max = height_max - 3;
		//height_max = height_max;
	}
	if (height_max_count <= 60 && height_max_count > 40)
	{
		height_max = height_max - 2;
		//height_max = height_max;
	}
	if (height_max_count <= 80 && height_max_count > 60)
	{
		height_max = height_max - 1;
		//height_max = height_max;
	}

	height = height_max;
}

//6、计算出包裹的长宽高、并画出最小外接矩形
void MVolume::shou_outline(CPaintDC& dc)
{
	//获取参考点的类
	MCoordinate layer;

	//1、压入数据，得到最小外接矩形的数据
	//vector是opencv官方教程的数据格式，
	vector<Point> points;
	//定义一个点变量----包裹扫描一次上下（或者说左右）的2个点的变量
	Point point_begin;
	Point point_end;
	//x轴移动的距离
	int x_run = 0;

	//---- 把包裹所有的点压入points中
	for (int i = 0; i < cycle_counts; i++)
	{
		//1、这个数据不压栈
		if (len_width_begin[i] == 0 && len_width_end[i] == 0)
		{
			continue;
		}
		//2、计算在x轴行走的距离
		x_run = (int)(pulse[i] * pulse_length);
		//3、开始压入数据
		point_begin.x = 100 + x_run;		//这个100是相对于mat的二位矩阵（0，0）的偏移，矩阵一个像素相当于1mm
		point_begin.y = 100 + len_width_begin[i] * 3;
		points.push_back(point_begin);
		//压入下面的点
		point_end.x = 100 + x_run;
		point_end.y = 100 + len_width_end[i] * 3;
		points.push_back(point_end);
	}
	//计算最小外接矩形-----------------------------------------------------
	RotatedRect rectPoint = minAreaRect(points);
	cut_rectPoint = rectPoint;//给其他函数用的变量
	//定义一个存储以上四个点的坐标的变量  ，这里一定是浮点型变量。
	Point2f fourPoint2f[4];
	//将rectPoint变量中存储的坐标值放到 fourPoint的数组中  
	rectPoint.points(fourPoint2f);


	//2、定义画笔，开始画外接矩形和高
	CPen pn1(PS_SOLID, 1, RGB(255, 0, 0));
	CPen *pOldPen1 = dc.SelectObject(&pn1);

	//--------------------开始画高----------------------------
	int h_startx, h_starty;
	int h_endx, h_endy;
	//第一个，和第二个数据长宽的参考点， 第三和第四个是高的参考点
	int reference[4] = { 0,0,0,0 };
	//通过函数给reference赋值
	layer.set_reference(reference);
	//长宽分配的像素1140*(840/(840+800)) = 584
	//高分配的像素1140*(800/(840+800)) = 556
	//height_1是显示高所拥有的像素
	int height_1 = 556;
	//因为现实高的区域没有800个像素，我们将高的区域要分成800个像素，将height_1分成800个单位
	//eight_pixel就变成实际所拥有的像素点
	//int height_pixel = (int)((height_1 / 800.0)*height);
	int height_pixel = (int)((556.0 / 800.0)*height);//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
	//int height_pixel = (int)((556.0 / 800.0)*height_1);
	//起点
	h_startx = reference[2];
	h_starty = reference[3] - height_pixel;
	//终点
	//x轴总的像素的数量
	int n_pixel = (int)(pulse[cycle_counts - 1] * pulse_length);
	n_pixel = (int)((584 / 840.0)*n_pixel);

	h_endx = h_startx - n_pixel;//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
	h_endy = h_starty;
	//画图形
	dc.MoveTo(h_startx, h_starty);
	dc.LineTo(h_endx, h_endy);
	//--------------------画最小外接矩形--------------------
	//位移量的补偿
	//x ---- 偏移的像素
	int vx = reference[0] - n_pixel - (int)((584 / 840.0) * 100);
	//y ---- 偏移的像素
	//-100+49+3
	int vy = -(int)((584 / 840.0) * 100) + 49 + 3;

	//将尺寸变换成对应的像素值
	int fx = GetSystemMetrics(SM_CXSCREEN);
	for (int i = 0; i < 4; i++)
	{
		//fourPoint2f[i].x = (float)((fourPoint2f[i].x * fx) / 1920.0);
		//fourPoint2f[i].y = (float)((fourPoint2f[i].y * fx) / 1920.0);
		fourPoint2f[i].x = (float)((584.0 / 840)*fourPoint2f[i].x);
		fourPoint2f[i].y = (float)((584.0 / 840)*fourPoint2f[i].y);
	}
	//开始画框框 ---- 在显示屏上画
	for (int i = 0; i < 3; i++)
	{
		dc.MoveTo((int)fourPoint2f[i].x + vx, (int)fourPoint2f[i].y + vy);
		dc.LineTo((int)fourPoint2f[i + 1].x + vx, (int)fourPoint2f[i + 1].y + vy);
	}
	dc.MoveTo((int)fourPoint2f[0].x + vx, (int)fourPoint2f[0].y + vy);
	dc.LineTo((int)fourPoint2f[3].x + vx, (int)fourPoint2f[3].y + vy);


	//画切角
	if (MDate::cut_corner == 1)
	{
		/*
		p_test1.x = (int)((584.0 / 840)*p_test1.x);
		p_test1.y = (int)((584.0 / 840)*p_test1.y);

		p_test2.x = (int)((584.0 / 840)*p_test2.x);
		p_test2.y = (int)((584.0 / 840)*p_test2.y);

		dc.MoveTo(p_test1.x + vx, p_test1.y + vy);
		dc.LineTo(p_test2.x + vx, p_test2.y + vy);
		*/
	}
	else
	{

	}
	

	//显示出包裹长宽的数据--以全局变量的形式返回长宽数据
	float length_f = (rectPoint.size.height > rectPoint.size.width) ? rectPoint.size.height : rectPoint.size.width;
	float width_f = (rectPoint.size.height > rectPoint.size.width) ? rectPoint.size.width : rectPoint.size.height;

	length = (int)length_f;
	width = (int)width_f;
}

//7、计算出包裹切角后的长宽高、并画出最小外接矩形
void MVolume::cut_shou_outline(CPaintDC& dc)
{
	//获取参考点的类
	MCoordinate layer;

	//1、压入数据，得到最小外接矩形的数据
	//vector是opencv官方教程的数据格式，
	vector<Point> points;
	//定义一个点变量----包裹扫描一次上下（或者说左右）的2个点的变量
	Point point_begin;
	Point point_end;
	//x轴移动的距离
	int x_run = 0;

	//把包裹所有的点压入points中
	for (int i = 0; i < cycle_counts; i++)
	{
		//1、这个数据不压栈
		if (cut_len_width_begin[i] == 0 && cut_len_width_end[i] == 0)
		{
			continue;
		}
		//2、计算在x轴行走的距离
		x_run = (int)(pulse[i] * pulse_length);
		//3、开始压入数据
		point_begin.x = 100 + x_run;		//这个100是相对于mat的二位矩阵（0，0）的偏移，矩阵一个像素相当于1mm
		point_begin.y = 100 + cut_len_width_begin[i] * 3;
		points.push_back(point_begin);
		//压入下面的点
		point_end.x = 100 + x_run;
		point_end.y = 100 + cut_len_width_end[i] * 3;
		points.push_back(point_end);
	}
	//计算最小外接矩形-----------------------------------------------------
	RotatedRect rectPoint = minAreaRect(points);
	cut_rectPoint = rectPoint;//给其他函数用的变量
	//定义一个存储以上四个点的坐标的变量  ，这里一定是浮点型变量。
	Point2f fourPoint2f[4];
	//将rectPoint变量中存储的坐标值放到 fourPoint的数组中  
	rectPoint.points(fourPoint2f);

	//2、定义画笔，开始画外接矩形和高
	CPen pn1(PS_SOLID, 2, RGB(255, 0, 0));
	CPen *pOldPen1 = dc.SelectObject(&pn1);

	//--------------------开始画高----------------------------
	int h_startx, h_starty;
	int h_endx, h_endy;
	//第一个，和第二个数据长宽的参考点， 第三和第四个是高的参考点
	int reference[4] = { 0,0,0,0 };
	//通过函数给reference赋值
	layer.set_reference(reference);
	//长宽分配的像素1140*(840/(840+800)) = 584
	//高分配的像素1140*(800/(840+800)) = 556
	//height_1是显示高所拥有的像素
	int height_1 = 556;
	//因为现实高的区域没有800个像素，我们将高的区域要分成800个像素，将height_1分成800个单位
	//eight_pixel就变成实际所拥有的像素点
	//int height_pixel = (int)((height_1 / 800.0)*height);
	int height_pixel = (int)((556.0 / 800.0)*height);//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
	//int height_pixel = (int)((556.0 / 800.0)*height_1);
	//起点
	h_startx = reference[2];
	h_starty = reference[3] - height_pixel;
	//终点
	//x轴总的像素的数量
	int n_pixel = (int)(pulse[cycle_counts - 1] * pulse_length);
	n_pixel = (int)((584 / 840.0)*n_pixel);

	h_endx = h_startx - n_pixel;//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
	h_endy = h_starty;
	//画图形
	dc.MoveTo(h_startx, h_starty);
	dc.LineTo(h_endx, h_endy);
	//--------------------画最小外接矩形--------------------
	//位移量的补偿
	//x ---- 偏移的像素
	int vx = reference[0] - n_pixel - (int)((584 / 840.0) * 100);
	//y ---- 偏移的像素
	//-100+49+3
	int vy = -(int)((584 / 840.0) * 100) + 49 + 3;

	//将尺寸变换成对应的像素值
	int fx = GetSystemMetrics(SM_CXSCREEN);
	for (int i = 0; i < 4; i++)
	{
		//fourPoint2f[i].x = (float)((fourPoint2f[i].x * fx) / 1920.0);
		//fourPoint2f[i].y = (float)((fourPoint2f[i].y * fx) / 1920.0);
		fourPoint2f[i].x = (float)((584.0 / 840)*fourPoint2f[i].x);
		fourPoint2f[i].y = (float)((584.0 / 840)*fourPoint2f[i].y);
	}
	//开始画框框
	for (int i = 0; i < 3; i++)
	{
		dc.MoveTo((int)fourPoint2f[i].x + vx, (int)fourPoint2f[i].y + vy);
		dc.LineTo((int)fourPoint2f[i + 1].x + vx, (int)fourPoint2f[i + 1].y + vy);
	}
	dc.MoveTo((int)fourPoint2f[0].x + vx, (int)fourPoint2f[0].y + vy);
	dc.LineTo((int)fourPoint2f[3].x + vx, (int)fourPoint2f[3].y + vy);

	//画切角
	if (MDate::cut_corner == 1)
	{
		CPen pn2(PS_SOLID, 3, RGB(255, 0, 0));
		dc.SelectObject(&pn2);
		CPoint point_1, point_2;
		for (int i = 0; i < 4; i++)
		{
			point_1.x = (int)((584.0 / 840)*cut_point[i][0][0]);
			point_1.y = (int)((584.0 / 840)*cut_point[i][0][1]);

			point_2.x = (int)((584.0 / 840)*cut_point[i][1][0]);
			point_2.y = (int)((584.0 / 840)*cut_point[i][1][1]);

			dc.MoveTo(point_1.x + vx, point_1.y + vy);
			dc.LineTo(point_2.x + vx, point_2.y + vy);
		}
	}
	else
	{
		//不做操作
	}


	//显示出包裹长宽的数据--以全局变量的形式返回长宽数据
	float length_f = (rectPoint.size.height > rectPoint.size.width) ? rectPoint.size.height : rectPoint.size.width;
	float width_f = (rectPoint.size.height > rectPoint.size.width) ? rectPoint.size.width : rectPoint.size.height;

	length = (int)length_f;
	width = (int)width_f;
}

//8、计算体积，画出图像
void  MVolume::show_volume(CPaintDC& dc, CString str_path)
{
	//画包裹图像，计算出高的值
	show_image(dc);

	//如果要画出切角后的最小外接矩形
	if (MDate::cut_outline == 1)
	{
		//画包裹边缘的线条，计算出长宽的值
		cut_shou_outline(dc);
	}
	else//如果不要画出切角后的最小外接矩形
	{
		//画包裹边缘的线条，计算出长宽的值
		shou_outline(dc);
	}

	//保存图像在文件夹里面
	//save_picture(str_path);
}

//**********************************************************************************************************************

//***********************************************************算法*******************************************************
/*
计算一个脉冲对应的运行距离
1、int p_lenth, int p_width  标定长方形的长，和宽。
2、原理：通过设置多个参数，比较不同参数的计算出来的，长和宽。 和标定长方形的长和宽进行比较。最接近标定板的长和宽就是我们需要的参数
*/
void MVolume::pulse_lenth_distance(int p_lenth, int p_width)
{
	//编码器一个脉冲对应的距离，以mm为单位
	//double pulse_length = 0.09841415;
	//我们做一个约定：第一次  最佳值应该在 0.05 ~ 0.15之间
	//在第一个的基础上，在下面的小数位去找最佳值

	//1、给脉冲的距离赋初值
	pulse_length = 0.05;//每次+0.0001  0.100217
	float sum = 1000.0;//定义一个比长宽都大很多的值
	double pulse_length_out = 0.0;//我们要求获取到的数据

	//RotatedRect out_rectPoint = get_min_rect();

	for (int i = 0; i < 100000; i++)
	{
		//2、求出长和宽
		//定义矩形变量
		RotatedRect out_rectPoint = get_min_rect();
		//显示出包裹长宽的数据--以全局变量的形式返回长宽数据
		float length_f = (out_rectPoint.size.height > out_rectPoint.size.width) ? out_rectPoint.size.height : out_rectPoint.size.width;
		float width_f = (out_rectPoint.size.height > out_rectPoint.size.width) ? out_rectPoint.size.width : out_rectPoint.size.height;

		//3、求出绝对值的和
		float subtract_length = p_lenth - length_f;
		if (subtract_length < 0)
		{
			subtract_length = subtract_length * (-1);
		}

		float subtract_width = p_width - width_f;
		if (subtract_width < 0)
		{
			subtract_width = subtract_width * (-1);
		}

		float sum_begin = subtract_length + subtract_width;

		//4、计算出绝对值的和最小值
		if (sum_begin < sum)
		{
			sum = sum_begin;
			pulse_length_out = pulse_length;
		}

		pulse_length = pulse_length + 0.000001;
	}

	//最终得到的值
	MDate::message(pulse_length_out);
}




//求一个脉冲对应输送带运动的距离
//原理：通过计算没有被最小外界矩形的面积来判断 ---- 面积最小的情况下，就是最佳值
void MVolume::pulse_lenth_area()
{


}

//**********************************************************************************************************************


//*******************************************************图像处理*******************************************************
//1、保存图像 ---- 取证用的
BOOL MVolume::save_picture(CString str_path)
{
	/*
	画切角代码注销了，代码还在里面
	*/
	//1、定义相关变量
	//拍照取证定义的类
	Mat picture(1000, 2000, CV_8UC3, Scalar(0, 0, 0));
	//像素颜色 ---- 黑色
	Vec3b pixel;
	pixel[0] = 255;
	pixel[1] = 255;
	pixel[2] = 255;
	//像素颜色 ---- 红色
	Vec3b pixel_red;
	pixel_red[0] = 0;
	pixel_red[1] = 0;
	pixel_red[2] = 255;
	//在中间画一条中间线，左边图像是长宽的图像，右边是高的图像
	Point middle_begin, middlet_end;
	middle_begin.x = 1000;
	middle_begin.y = 0;
	middlet_end.x = 1000;
	middlet_end.y = 1000;
	line(picture, middle_begin, middlet_end, pixel_red);

	//2、画高的最高线条
	Point height_1;
	height_1.x = 999 + 100 + (int)(pulse[0] * pulse_length);
	height_1.y = 999 - height;
	Point height_2;
	height_2.x = 999 + 100 + (int)(pulse[cycle_counts - 1] * pulse_length);
	height_2.y = 999 - height;
	line(picture, height_1, height_2, pixel_red);

	//4、画最小外接矩形
	//定义一个存储以上四个点的坐标的变量  ，这里一定是浮点型变量。
	Point2f fourPoint2f[4];
	//将rectPoint变量中存储的坐标值放到 fourPoint的数组中
	cut_rectPoint.points(fourPoint2f);
	//保存图片到文件里面
	for (int i = 0; i < 3; i++)
	{
		Point pt1;
		pt1.x = (int)fourPoint2f[i].x;
		pt1.y = (int)fourPoint2f[i].y;
		Point pt2;
		pt2.x = (int)fourPoint2f[i + 1].x;
		pt2.y = (int)fourPoint2f[i + 1].y;
		//开始画线条
		line(picture, pt1, pt2, pixel_red);
	}
	Point pt1;
	pt1.x = (int)fourPoint2f[3].x;
	pt1.y = (int)fourPoint2f[3].y;
	Point pt2;
	pt2.x = (int)fourPoint2f[0].x;
	pt2.y = (int)fourPoint2f[0].y;
	//开始画线条
	line(picture, pt1, pt2, pixel_red);

	//5、画切角 ---- 解除这个代码的注销，有明显的图像切角效果
	/*
	for (int i = 0; i < 4; i++)
	{
		Point cut_1, cut_2;
		cut_1.x = cut_point[i][0][0];
		cut_1.y = cut_point[i][0][1];
		cut_2.x = cut_point[i][1][0];
		cut_2.y = cut_point[i][1][1];
		line(picture, cut_1, cut_2, pixel_red);
	}
	*/

	//6、画长宽、高的图像
	//定义一个点变量----包裹扫描一次上下（或者说左右）的2个点的变量
	Point point_begin, point_end;
	//高上下的2个点
	Point point_down_end, point_up_begin;
	//x轴移动的距离
	int x_run = 0;
	//将点压入图像中 ---- 用原始数据压入
	for (int i = 0; i < cycle_counts; i++)
	{
		//1、这个数据不压栈
		if (len_width_begin[i] == 0 && len_width_end[i] == 0)
		{
			continue;
		}
		//2、计算在x轴行走的距离
		x_run = (int)(pulse[i] * pulse_length);
		//3、开始压入数据
		point_begin.x = 100 + x_run;		//这个100是相对于mat的二位矩阵（0，0）的偏移，矩阵一个像素相当于1mm
		point_begin.y = 100 + len_width_begin[i] * 3;
		//把点压入拍照取证的图片中
		picture.at<Vec3b>(point_begin.y, point_begin.x) = pixel;

		//压入下面的点
		point_end.x = 100 + x_run;
		point_end.y = 100 + len_width_end[i] * 3;
		//把点压入拍照取证的图片中
		picture.at<Vec3b>(point_end.y, point_end.x) = pixel;

		//开始画高
		int height_1 = (height_down_end[i] - height_up_begin[i]) * 3;
		point_up_begin.x = 999 + 100 + x_run;
		point_up_begin.y = 999 - height_1;
		picture.at<Vec3b>(point_up_begin.y, point_up_begin.x) = pixel;
	}

	//7、保存图片
	//系统函数获取时间
	CTime m_time;
	m_time = CTime::GetCurrentTime();				//获取当前时间日期  
	//得到：时 - 分 - 秒
	CString ss_time = m_time.Format(_T("%H")) + _T(".") + m_time.Format(_T("%M")) + _T(".") + m_time.Format(_T("%S"));
	CString cs_save = str_path + _T("-") + ss_time + _T(".png");

	//AfxMessageBox(cs_save);
	//照片文件全名
	String save_p = toString(cs_save);
	//String save_p = "date\\picture\\save_picture.png";
	//BOOL bl_write = imwrite("save_picture.png", picture);
	BOOL bl_write = imwrite(save_p, picture);
	if (bl_write == FALSE)
	{
		//AfxMessageBox(_T("图像保持失败"));
		return FALSE;
	}
	else
	{
		//AfxMessageBox(_T("图像保持成功"));
	}

	return TRUE;
}

//**********************************************************************************************************************

String MVolume::toString(CString cs) {
#ifdef _UNICODE

	//如果是unicode工程
	USES_CONVERSION;
	std::string str(W2A(cs));
	return str;
#else
	//如果是多字节工程 
	std::string str(cs.GetBuffer());
	cs.ReleaseBuffer();
	return str;

#endif // _UNICODE 
}

