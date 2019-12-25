//------------------有待解决的问题------------------
//1、
//--------------------------------------------------





#include "stdafx.h"
#include "MCom_port.h"
#include "MDate.h"


MCom_port::MCom_port()
{
}


MCom_port::~MCom_port()
{
}


/*
该方法在 1ms内即可完成查找；同时也可解决 usb 转串口设备的问题，比较实用，唯一缺点是，
如果用户在装某些软硬件时在注册表中注册了虚拟串口之类的，用此法枚举得到的该类串口实际
上是不能当串口用的。
*/
BOOL MCom_port::find_comms(int ports[512])
{
	//当前串口的状态
	LONG Status;
	DWORD dwIndex = 0;
	TCHAR Name[255];//???
	DWORD dwName;
	DWORD Type;
	UCHAR szPortName[255];
	DWORD dwSizeofPortName;
	int i = 0;

	dwName = sizeof(Name);
	dwSizeofPortName = sizeof(szPortName);

	/*
	HKEY是主根键名称。在它下面是最大的几个键，也就是所谓根键。
	HKLM=HKEY_LOCAL_MACHINE，是机器软硬件信息的集散地。
	HKCU=HKEY_CURRENT_USER，是当前用户所用信息储存地。
	除此以外，还有：
	HKEY_CLASSES_ROOT：文件关联相关信息。
	HKEY_USERS：所有用户信息。
	HKEY_CURRENT_CONFIG：当前系统配置。
	*/
	HKEY hKey;
	
	//保存串口的变量
	//CString strSerialList[256];  // 临时定义 256 个字符串组，因为系统最多也就 256 个 

	LPCTSTR data_Set = _T("HARDWARE\\DEVICEMAP\\SERIALCOMM\\");
	//打开一个制定的注册表键,成功返回ERROR_SUCCESS即“0”值
	long ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, data_Set, 0, KEY_READ, &hKey); 
	if (ret == ERROR_SUCCESS)
	{
		do
		{
			//读取键值 
			Status = RegEnumValue(	hKey,						//打开注册表项的句柄。
									dwIndex++, //要检索的值的索引。对于第一次调用RegEnumValue函数，此参数应为零 ，然后为后续调用递增。
									Name, 
									&dwName, 
									NULL, 
									&Type, 
									szPortName, 
									&dwSizeofPortName);
			//如果lpData(szPortName)缓冲区太小而无法接收该值，则该函数返回ERROR_MORE_DATA。
			if ((Status == ERROR_SUCCESS) || (Status == ERROR_MORE_DATA))
			{
				CString ss;
				for (int j = 0; j < (int)dwSizeofPortName; j++)
				{
					CString strPort;
					strPort += szPortName[j];//szPortName里面有很多空格字符
					if (strPort == _T(""))
					{
					}
					else
					{
						ss += strPort;
					}
				}
				ss.Delete(0,3);
				int  intResult;
				intResult = _ttoi(ss);
				//MDate::message(intResult);
				ports[i * 2] = intResult;
				ports[i * 2+1] = 0;

				i++;// 串口计数 
			}
			//每读取一次dwName和dwSizeofPortName都会被修改 
			//注意一定要重置,否则会出现很离奇的错误,本人就试过因没有重置,出现先插入串口号大的（如COM4）,再插入串口号小的（如COM3），此时虽能发现两个串口，但都是同一串口号（COM4）的问题，同时也读不了COM大于10以上的串口 
			dwName = sizeof(Name);
			dwSizeofPortName = sizeof(szPortName);
		} while ((Status == ERROR_SUCCESS) || (Status == ERROR_MORE_DATA));


		RegCloseKey(hKey);	

		//判断串口的数量和要得到的是否一致
		if (MDate::comm_count > i)
		{
			AfxMessageBox(_T("寻找到的串口数量 < 需要的串口数量"));
			return FALSE;
		}
		
		return TRUE;
	}
	else
	{
		MDate::message(_T("无法打开注册表的KEY"));
		return FALSE;
	}
}


BOOL MCom_port::create_port(HANDLE& hCom, int port_number, int parameter[5], CString ss)
{
	//1、格式化串口号
	CString str;
	if (port_number < 10)
	{
		str.Format(_T("COM%d"), port_number);
	}
	else
	{
		str.Format(_T("\\\\.\\COM%d"), port_number);
	}

	//2、创建串口对象	
	hCom = CreateFile(str,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		NULL);

	//3、如果创建失败
	if (hCom == INVALID_HANDLE_VALUE)
	{
		//----这里有一个疑问：创建失败了就不需要CloseHandle(hCom)？？？
		ss = ss + _T("----串口创建失败");
		MDate::message(ss);
		return FALSE;
	}

	//4、设置输入输出缓冲区大小
	if (!SetupComm(hCom, parameter[0], parameter[1]))
	{
		ss = ss + _T("----串口配置参数失败");
		MDate::message(ss);
		CloseHandle(hCom);
		return FALSE;
	}

	//5、设置读超时
	COMMTIMEOUTS timeouts;
	//----这里好像可以不要调用Get函数
	GetCommTimeouts(hCom, &timeouts);
	timeouts.ReadIntervalTimeout = 0;				//以上各个成员设为0表示未设置对应超时。
	timeouts.ReadTotalTimeoutMultiplier = 5;
	timeouts.ReadTotalTimeoutConstant = parameter[3];
	timeouts.WriteTotalTimeoutMultiplier = 5;
	timeouts.WriteTotalTimeoutConstant = parameter[4];
	SetCommTimeouts(hCom, &timeouts);

	//6、设计串口参数
	DCB dcb;
	if (!GetCommState(hCom, &dcb))
	{
		ss = ss + _T("----获取串口状态参数失败");
		MDate::message(ss);
		CloseHandle(hCom);
		return FALSE;
	}
	dcb.BaudRate = parameter[2];//波特率为9600/115200
	dcb.ByteSize = 8;//数据位为8位
	dcb.Parity = 0;//校验方式为无校验
	dcb.StopBits = ONESTOPBIT;//停止位为1位
	dcb.fBinary = true;
	if (!SetCommState(hCom, &dcb))
	{
		ss = ss + _T("----设置串口状态参数失败");
		MDate::message(ss);
		CloseHandle(hCom);
		return FALSE;
	}

	//7、创建串口的时候，做一个清空。
	//清空缓冲
	PurgeComm(hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);
	//PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	//8、清除错误
	DWORD dwError;
	COMSTAT cs;
	if (!ClearCommError(hCom, &dwError, &cs))
	{
		ss = ss + _T("----串口清除错误失败");
		MDate::message(ss);
		CloseHandle(hCom);
		return FALSE;
	}

	return TRUE;
}


//向下位机发送数据(是一个数组)
//BYTE dateParam[] ----待发送的数据
//int n ----发送数据的长度
BOOL MCom_port::send_date(HANDLE& hCom, BYTE dateParam[], int n, int wait_time, CString ss)
{
	//1、检测数据的长度
	if (n > 20)
	{
		ss = ss + _T("--警告--发送数据长度超过20");
		MDate::message(ss);
		return FALSE;
	}

	//2、因为是异步处理串口数据，所以在WriteFile（）函数里面要OVERLAPPED（）结构做为参数
	OVERLAPPED  m_osWrite;
	// OVERLAPPED初始化0
	memset(&m_osWrite, 0, sizeof(OVERLAPPED));
	//overlapped结构里面的hevent需要用户Create一个对象
	m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	//3、发送二进制数据？？？？？？？？？？？？？？？？？？？
	BYTE date[20] = { 0 };
	//做一个循环来给数组赋值----使用参数
	for (int i = 0; i < n; i++)
	{
		date[i] = dateParam[i];
	}

	//4、发送数据之前清空缓冲区(只要清空输出，不要清空输入缓冲区)
	PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR);
	//清楚串口错误，为输出数据做好准备
	COMSTAT ComStat;
	DWORD dwErrorFlags;
	ClearCommError(hCom, &dwErrorFlags, &ComStat);

	//5、开始输出数据
	DWORD dwBytesWrite;
	BOOL bWriteStat;
	bWriteStat = WriteFile(hCom,
		date,
		n,//发送数据的个数
		&dwBytesWrite,
		&m_osWrite);

	//6、注意这里是异步处理串口数据
	if (!bWriteStat)
	{
		if (GetLastError() == ERROR_IO_PENDING)
		{
			DWORD error = WaitForSingleObject(m_osWrite.hEvent, wait_time);
			switch (error)
			{
			case WAIT_OBJECT_0:
				//所代表的进程在规定秒内结束，关键这个语句可以返回实际收到的数据的数量  
				if (!GetOverlappedResult(hCom, &m_osWrite, &dwBytesWrite, false))
				{
					ss = ss + _T("--发送数据：GetOverlappedResult错误 & 重新启动软件");
					MDate::message(ss);
					return FALSE;
				}
				//返回值成功，break退出switch语句
				break;
			case WAIT_TIMEOUT:
				// 等待时间超时
				ss = ss + _T("--发送数据：超时 & 重新启动软件");
				MDate::message(ss);
				return FALSE;

			case WAIT_FAILED:
				// 函数调用失败，比如传递了一个无效的句柄   
				ss = ss + _T("--发送数据：wait_failed & 重新启动软件");
				MDate::message(ss);
				return FALSE;
			}
		}
		else
		{
			ss = ss + _T("-- GetLastError() != ERROR_IO_PENDING & 重新启动软件");
			MDate::message(ss);
			return FALSE;
		}
	}

	//实际发送的数据和需要发送的数据量是一直的时候
	if ((int)dwBytesWrite != n)
	{
		ss = ss + _T("--发送数据的数量和需要发送的数量不一致 & 重新启动软件");
		MDate::message(ss);
		return FALSE;
	}

	return TRUE;
}


BOOL  MCom_port::receive_date(HANDLE& hCom, BYTE out_date[], int n, int wait_time, CString ss)
{
	//1、----为接收数据定义相关变量
	//因为是异步处理串口数据，所以在ReadFile（）函数里面要OVERLAPPED（）结构做为参数
	OVERLAPPED ol_Read;
	memset(&ol_Read, 0, sizeof(OVERLAPPED));
	ol_Read.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	//2、定义一个comstat结构，为clearcommerror（）函数做准备
	COMSTAT ComStat;
	DWORD dwErrorFlags;
	//清楚串口可能出现的错误，保证程序不会因为错误而不能进行，获取缓冲区的数据大小
	ClearCommError(hCom, &dwErrorFlags, &ComStat);

	//3、----开始读取数据
	//readfile（）函数做参数准备
	BYTE date[1024];
	memset(date, 0, sizeof(date));
	DWORD dwBytesRead = 0;//实际读取的字节数
	BOOL bReadStat;
	//开始读取数据
	bReadStat = ReadFile(hCom,
		date,
		n,				//ComStat.cbInQue不要用这个变量，可能ComStat.cbInQue比n要大，但是我不需要所有的数据，我只要n个数据
		&dwBytesRead,
		&ol_Read);

	//4、注意这里是异步处理串口数据
	if (!bReadStat)
	{
		if (GetLastError() == ERROR_IO_PENDING)	//GetLastError()函数返回ERROR_IO_PENDING,表明串口正在进行读操作
		{
			//使用WaitForSingleObject函数等待，直到读操作完成或延时已达到wait_time秒钟
			DWORD error = WaitForSingleObject(ol_Read.hEvent, wait_time);	//INFINITE	--wait_time
			switch (error)
			{
			case WAIT_OBJECT_0:
				//所代表的进程在6秒内结束，关键这个语句可以返回实际收到的数据的数量 
				//如果此参数为FALSE且操作仍处于暂挂状态，则该函数返回FALSE， GetLastError函数返回ERROR_IO_INCOMPLETE。
				if (!GetOverlappedResult(hCom, &ol_Read, &dwBytesRead, false))
				{
					ss = ss + _T("--发送数据：GetOverlappedResult错误 & 重新启动软件");
					MDate::message(ss);
					return FALSE;
				}
				//返回值成功，break退出switch语句
				break;

			case WAIT_TIMEOUT:
				// 等待时间超时
				ss = ss + _T("--发送数据：超时 & 重新启动软件");
				MDate::message(ss);
				//AfxMessageBox(_T("2"));
				return FALSE;

			case WAIT_FAILED:
				// 函数调用失败，比如传递了一个无效的句柄   
				ss = ss + _T("--发送数据：wait_failed & 重新启动软件");
				MDate::message(ss);
				return FALSE;
			}
		}
		else
		{
			ss = ss + _T("-- GetLastError() != ERROR_IO_PENDING & 重新启动软件");
			MDate::message(ss);
			return FALSE;
		}
	}
	
	//5、----接收数据成功给返回指针赋值
	if (dwBytesRead == n)
	{
		for (int i = 0; i < n; i++)
		{
			out_date[i] = date[i];
		}
	}
	else
	{
		ss = ss + _T("-- 接收到的数据数量和预期不一致");
		MDate::message(ss);
		return FALSE;
	}

	//6、----读取问之后，就清空缓冲区
	PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	return TRUE;
}


// 为串口配置参数----param[0]:输入缓冲区; param[1]:输出缓冲区; param[2]:波特率
//给find_port()函数使用
//配置完后，清空缓冲区，清除错误
BOOL  MCom_port::config_param(HANDLE& hCom, int param[5], CString ss)
{
	//hCom ---- 这个串口号是已经创建好了的，不需要在创建了，只要对他配置参数
	//设置输入输出缓冲区大小
	if (!SetupComm(hCom, param[0], param[1]))
	{
		ss = ss + _T("----串口配置参数失败");
		MDate::message(ss);
		//关掉串口最好在创建这个串口的函数里面操作，不要在被调函数里面做操作
		//CloseHandle(hCom);
		return FALSE;
	}

	//设置读超时
	COMMTIMEOUTS timeouts;
	GetCommTimeouts(hCom, &timeouts);//这里好像可以不要调用Get函数？？？？？？？？？？						 
	timeouts.ReadIntervalTimeout = 0;//这里组好赋值为0.
	timeouts.ReadTotalTimeoutMultiplier = 5;
	timeouts.ReadTotalTimeoutConstant = param[3];
	timeouts.WriteTotalTimeoutMultiplier = 5;
	timeouts.WriteTotalTimeoutConstant = param[4];
	SetCommTimeouts(hCom, &timeouts);

	//设计串口参数
	DCB dcb;
	if (!GetCommState(hCom, &dcb))
	{
		ss = ss + _T("----获取串口状态参数失败");
		MDate::message(ss);
		//CloseHandle(hCom);
		return FALSE;
	}
	dcb.BaudRate = param[2];//波特率为
	dcb.ByteSize = 8;//数据位为8位
	dcb.Parity = 0;//校验方式为无校验
	dcb.StopBits = ONESTOPBIT;//停止位为1位
	dcb.fBinary = true;
	if (!SetCommState(hCom, &dcb))
	{
		ss = ss + _T("----设置串口状态参数失败");
		MDate::message(ss);
		//CloseHandle(hCom);
		return FALSE;
	}

	//配置好之后，做一次清理工作
	//清空缓冲
	PurgeComm(hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);
	//清除错误
	DWORD dwError;
	COMSTAT cs;
	if (!ClearCommError(hCom, &dwError, &cs))
	{
		ss = ss + _T("----串口清除错误失败");
		MDate::message(ss);
		//CloseHandle(hCom);
		return FALSE;
	}

	return TRUE;
}


//接收数据 并且 和期望的数据进行对比
//n 是接收数据的长度
BOOL MCom_port::receive_for_judge(HANDLE& hCom, BYTE dateParam[], int n, int wait_time, CString ss)
{
	//1、接收数据
	BYTE date[20] = { 0x00 };
	BOOL bl = receive_date(hCom, date, n, wait_time, ss);
	if (bl == FALSE)
	{
		ss = ss + _T("receive_for_judge()函数，接收数据失败");
		MDate::message(ss);
		return FALSE;
	}

	//2、date是我们接收到的数据，dateParam[]是我们期望收到的数据，在这里我们做一个比较
	for (int i = 0; i < n; i++)
	{
		if (date[i] != dateParam[i])
		{
			ss = ss + _T("receive_for_judge()函数，接收到了数据，但是数据和期望值不一致");
			MDate::message(ss);
			return FALSE;
		}
	}

	return TRUE;
}


//串口握手的时候，输入数据后，判断输出数据是否是自己预期的数据
BOOL MCom_port::judge_port(HANDLE& hCom,						//串口句柄
							BYTE date1[],						//发送的数据
							int n1,								//发送数据的长度
							int wait_time1,						//串口发送数据等待的时间
							BYTE date2[],						//接收到的数据
							int n2,								//约定接收到数据的长度
							int wait_time2)						//串口接收数据等待的时间
{
	BOOL bl_send = send_date(hCom, date1, n1, wait_time1, _T("judge_port（）函数"));
	if (bl_send == FALSE)
	{
		return FALSE;
	}

	//发送完数据后，等一会儿，就开始接收数据，这样思路上回更好一些
	BOOL bl_receive = receive_for_judge(hCom, date2, n2, wait_time2,_T("judge_port（）函数"));
	if (bl_receive == FALSE)
	{
		return FALSE;	
	}

	return TRUE;
}


//对下位机1个串口自动寻址
//返回串口号，如果返回值是0：说明没有找到对应的串口
int MCom_port::find_port(BYTE shake_date[],				//发送出去的握手数据
							int n1,						//BYTE shake_date[] 数据的长度
							int wait_time1,				//串口发送接收等待时间
							BYTE shake_return[],		//接收回来的握手数据	
							int n2,						//BYTE shake_return[] 数据的长度
							int wait_time2,				//串口接收数据的等待时间
							int param[],				//param[]是串口配置的参数
							int ports[],				//有效串口的数组，从注册表中找到的所有有效串口
							CString ss)					//提示语
{
	//1、寻找到注册表中串口的数量
	int count = 0;
	for (int i = 0; i < 256; i++)
	{
		if (ports[i * 2] != 0)
		{
			count++;
		}
		else
		{
			break;//如果记录串口编号的数据是0，说明没有串口了
		}
	}
	
	//2、定义串口句柄。
	HANDLE handle;
	int port_number = 0;
	
	//3、循环寻址
	for (int i = 0; i < count; i++)
	{
		//1、判断串口是否是已经寻址过了的串口，如果是，我们在下面的语句中会自定义ports[2 * i + 1] = 1（本函数最后面会有这个语句）																																				
		if (ports[2 * i + 1] != 0)
		{
			MDate::message(ports[2 * i], _T("这串口号 ---- 已经被占用了"));
			continue;	                                                                  
		}
		MDate::message(ports[2 * i],_T("遍历了一次这个串口号"));

		//2、串口号转换成字符串
		CString comm = _T("COM");
		CString numb;
		numb.Format(_T("%d"), ports[2*i]);
		comm = comm + numb;
		if (ports[2 * i] >= 10)
		{
			comm = _T("\\\\.\\") + comm;
		}

		//3、这里的CreateFile函数起了很大的作用，可以用来创建系统设备文件，如果该设备不存在或者被占用，则会返回一个错误，即下面的 INVALID_HANDLE_VALUE ，据此可以判断可使用性。详细参见MSDN中的介绍。
		handle = CreateFile(comm,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,//串口是异步传输数据的方式
			NULL);

		//4、如果本次寻址不是有效地址就退出本次循环。疑问ports[]数组是注册表中搜到的有效串口数组，这个地方其实有点多余，但是为了保险起见，还是做这个操作
		//invalid 串口就是没有下位机的串口
		if (handle == INVALID_HANDLE_VALUE)     // 如果没有该设备，或者被其他应用程序在用 
		{
			CloseHandle(handle); // 关闭文件句柄
			continue;
		}
		else
		{
			//通过发送一个数据，和接收一个数据来判断是那一个下位机的串口号（就是通过定义的协议内容来判断）。
			//需要调用一个发送数据的函数，和接收数据的函数（不同的串口，握手的数据格式是不一样的）
			//4、给这个有效串口配置串口参数
			BOOL con_bl = config_param(handle, param, ss);
			if (con_bl == FALSE)
			{
				CloseHandle(handle);
				continue;
			}
			Sleep(30);

			//5、发送握手数据,检测返回值是否和协议规定的一致
			BOOL v_bl = judge_port(handle, shake_date, n1, wait_time1, shake_return, n2, wait_time2);
			if (v_bl == FALSE)
			{
				CloseHandle(handle); // 关闭文件句柄
				continue;
			}
			else
			{
				ss = ss + _T("----成功找到串口号");
				MDate::message(ss);
				port_number = ports[2 * i];
				//这个串口就已经被占用了 ---- 做一个标记
				ports[2 * i + 1] = 1;
				CloseHandle(handle); // 关闭文件句柄
				break;
			}
		}
	}

	//4、如果循环没有找到串口，返回0
	if (port_number == 0)
	{
		ss = ss + _T("----串口号没有被发现");
		MDate::message(ss);
		return 0;
	}

	//5、如果找到了，就返回串口号
	return port_number;
}

