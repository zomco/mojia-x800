#include "stdafx.h"
#include "MInternet.h"
#include "MDate.h"
#include <iostream>
#include <string>
#include "md5c.h"



MInternetParam::MInternetParam(CString length, CString width, CString  height, CString weight, CString orderNo)
{
	m_length = length;
	m_width = width;
	m_height = height;
	m_weight = weight;
	m_orderNo = orderNo;
}

MInternetParam::~MInternetParam()
{

}

MInternet::MInternet()
{
}


MInternet::~MInternet()
{
}





//将UTF-编码格式转换成Unicode，因为Unicode格式方便在电脑上显示不乱码，而UTF-8格式方便网络传输
CString UTF8toUnicode(const char* utf8Str, UINT length)
{
	CString unicodeStr;
	unicodeStr = _T("");

	if (!utf8Str)
		return unicodeStr;

	if (length == 0)
		return unicodeStr;


	WCHAR chr = 0;//一个中文字符  
	for (UINT i = 0; i<length;)
	{
		//UTF8的三种中文格式  
		if ((0x80 & utf8Str[i]) == 0) //只占用一个字节  
		{
			chr = utf8Str[i];
			i++;
		}
		else if ((0xE0 & utf8Str[i]) == 0xC0) //占用两个字节  
		{
			chr = (utf8Str[i + 0] & 0x3F) << 6;
			chr |= (utf8Str[i + 1] & 0x3F);
			i += 2;
		}
		else if ((0xF0 & utf8Str[i]) == 0xE0)//占用三个字节  
		{
			chr = (utf8Str[i + 0] & 0x1F) << 12;
			chr |= (utf8Str[i + 1] & 0x3F) << 6;
			chr |= (utf8Str[i + 2] & 0x3F);
			i += 3;
		}

		else
		{
			return unicodeStr;
		}
		unicodeStr.AppendChar(chr);
	}

	return unicodeStr;
}


//对要长传的变量进行赋值操作
void MInternet::init_http_date(CString length, CString width, CString height, CString weight, CString orderNo)
{
	MInternetParam param = MInternetParam(length, width, height, weight, orderNo);
	m_params.push_back(param);
}


//*******************************************************易集达*******************************************************
//这个函数值承担上传的任务，其他处理都不要在这个函数里面
BOOL MInternet::send_date_yijida()
{
	CString strUrl = _T("http://47.90.96.38:8080/ForeignTradeTwo/sysback/sm.htm?");

	for (int i = 0; i < m_params.size(); i++)
	{
		strUrl += _T("id=") + m_params[i].m_orderNo;																			// 条码
		strUrl += _T("&weight=") + m_params[i].m_weight;																		// 重量
		strUrl += _T("&volume=") + m_params[i].m_length + _T("*") + m_params[i].m_width + +_T("*") + m_params[i].m_height;		// 体积
	}

	//5、********************************************************************************************************
	//解析并获取服务器基本信息
	CString strServerName, strObjectName;
	INTERNET_PORT   nPort;
	DWORD dwServiceType;
	AfxParseURL(strUrl, dwServiceType, strServerName, strObjectName, nPort);

	//MFC网络传输用的对象
	CInternetSession session;
	CHttpConnection *pHttpConnection = NULL;
	CHttpFile* pFile = NULL;

	pHttpConnection = session.GetHttpConnection(strServerName, nPort);
	if (pHttpConnection == NULL)
	{
		session.Close();
		AfxMessageBox(_T("创建连接失败"));
		return FALSE;
	}
	pFile = pHttpConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET, strObjectName);
	if (pFile == NULL)
	{
		pHttpConnection->Close();
		delete pHttpConnection;
		session.Close();
		AfxMessageBox(_T("创建请求失败"));
		return FALSE;
	}
	CString szHeaders = L"Accept: audio/x-aiff, audio/basic, audio/midi,\
                     audio/mpeg, audio/wav, image/jpeg, image/gif, image/jpg, image/png,\
                     image/mng, image/bmp, text/plain, text/html, text/htm\r\n";
	pFile->AddRequestHeaders(szHeaders);
	pFile->SendRequest();

	DWORD dwRet;
	pFile->QueryInfoStatusCode(dwRet);
	if (dwRet != HTTP_STATUS_OK)
	{
		// 请求失败
		pFile->Close();
		pHttpConnection->Close();
		delete pFile;
		delete pHttpConnection;
		session.Close();

		AfxMessageBox(_T("请求失败"));
		return FALSE;
	}
	else
	{
		// 请求成功
		pFile->Close();
		pHttpConnection->Close();
		//指针是需要用户手动释放资源的
		delete pFile;
		delete pHttpConnection;
		session.Close();

		m_params.clear();
		return TRUE;
	}
}


BOOL MInternet::send_date_yijida_test()
{
	//1、********************************************************************************************************
	//解释说明
	//测试代码id=389&weight=200&volume=34*23*22 -------- 分别代表条码、重量、长/宽/高
	//http://47.90.96.38:8080/ForeignTradeTwo/sysback/sm.htm?number=1&id=3774&weight=1200&volume=74*55*36

	//2、********************************************************************************************************
	//生成上传到服务器的数据格式 ---- 发送数据前，最终要得到的数据格式
	//send_service = str_adress + str_date;
	CString send_service;
	//4********************************************************************************************************
	//生产最终发送的数据的字符窜
	//send_service = L"http://47.90.96.38:8080/ForeignTradeTwo/sysback/sm.htm?number=2&id=389&weight=200&volume=34*23*22&&weight=200&volume=34*23*22";
	//send_service = L"http://47.90.96.38:8080/ForeignTradeTwo/sysback/sm.htm?number=1&id=3774&weight=1200&volume=74*55*36";



	send_service = L"http://wsh.kingtrans.net/EquipInterfaceService?method=equipReceipt&orderNo=566000092040&length=100.5&width=100.5&height=100.5&weight=100.5";





	//http://wsh.kingtrans.net/EquipInterfaceService?method=equipReceipt&orderNo=566000092040&length=100.5&width=100.5&height=100.5&weight=100.5


	//3、********************************************************************************************************
	//解析并获取服务器基本信息
	CString defServerName, defObjectName;
	INTERNET_PORT   nPort;
	DWORD dwServiceType;
	AfxParseURL(send_service, dwServiceType, defServerName, defObjectName, nPort);

	//MFC网络传输用的对象
	CInternetSession session;
	CHttpConnection *pHttpConnection = NULL;
	CHttpFile* pFile = NULL;

	//构造一个CHttpConnection对象，建立HTTP连接
	pHttpConnection = session.GetHttpConnection(defServerName, nPort);

	// HTTP_VERB_GET---- HTTP 请求类型；
	// TEXT("/Practice/index.jsp")----指向一个包含指定的谓词的目标对象的字符串的指针。这通常是一个文件名、可执行模块或搜索说明符。
	pFile = pHttpConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET, defObjectName);

	//dwServiceType这个变量就是：CHttpConnection::HTTP_VERB_POST的意思
	//pHTTP = pHttpConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST, defObjectName);
	CString szHeaders = L"Accept: audio/x-aiff, audio/basic, audio/midi,\
                     audio/mpeg, audio/wav, image/jpeg, image/gif, image/jpg, image/png,\
                     image/mng, image/bmp, text/plain, text/html, text/htm, application/json\r\n";

	pFile->AddRequestHeaders(szHeaders);
	//发送请求 ---- 这里应该是一件发送了数据
	//在 CHttpFile::SendRequest() 之后，一定要用 CHttpFile::QueryInfoStatusCode() 来获得请求的状态码，从而判断是否正确获得了 http 数据
	try
	{
		pFile->SendRequest();
	}
	catch (CInternetException *pEx)
	{
		AfxMessageBox(_T("请求服务器抛出异常，请求失败"));
		pEx->Delete();
		
		pFile->Close();
		pHttpConnection->Close();
		//指针是需要用户手动释放资源的
		delete pFile;
		delete pHttpConnection;
		session.Close();

		return FALSE;
	}

	//获取状态码
	DWORD dwRet;
	pFile->QueryInfoStatusCode(dwRet);
	if (dwRet != HTTP_STATUS_OK)
	{
		//提示状态码
		CString errText;
		errText.Format(L"返回状态码错误：%d", dwRet);
		AfxMessageBox(errText);
		
		pFile->Close();
		pHttpConnection->Close();
		//指针是需要用户手动释放资源的
		delete pFile;
		delete pHttpConnection;
		session.Close();

		return FALSE;
	}
	else
	{
		/*
		//提示状态码
		CString errText;
		errText.Format(L"返回状态码：%d", dwRet);
		AfxMessageBox(errText);
		*/

		int numread = 0;
		char buf[2024] = { 0 };
		while ((numread = pFile->Read(buf, sizeof(buf) - 1)) > 0)
		{
			CString ccc;
			//Windows本地是使用Unicode编码显示的CString是用Unicode编码，
			//这里转换成Unicode编码
			ccc = UTF8toUnicode(buf, numread);
			AfxMessageBox(ccc);

			int index_n;
			//**********************************************************返回对/错**********************************************************
			/*
			数据格式：
			{"msg":"搜索失败，请稍后再试","statu":false}
			{"msg":"搜索成功","model":
			*/
			CString str1 = ccc.Mid(10, 2);
			if (str1 == _T("失败"))
			{
				AfxMessageBox(_T("服务器返回----搜索失败"));
				break;
			}
			else if (str1 == _T("成功"))
			{
				//AfxMessageBox(_T("数据成功"));
				//只有成功了，才继续执行后续的操作
			}
			else
			{
				AfxMessageBox(_T("服务器数据格式异常，请联系服售后服务人员"));
				break;
			}
			//**********************************************************用户名**********************************************************
			//_T(",\"name\":\"")  是根据json格式定义的特点
			int nIndex2 = ccc.Find(_T(",\"name\":\""));
			if (nIndex2 == -1)
			{
				return FALSE;
			}
			//索引前移9个字符就是，我们需要找的name
			nIndex2 = nIndex2 + 9;
			int nIndex22 = ccc.Find(_T("\""), nIndex2);
			if (nIndex22 == -1)
			{
				return FALSE;
			}
			index_n = nIndex22 - nIndex2;
			CString str2 = ccc.Mid(nIndex2, index_n);
			str2 = _T("用户名：") + str2;
			AfxMessageBox(str2);
			//**********************************************************用户代码**********************************************************
			//_T("\",\"order_id\":\"")  是根据json格式定义的特点
			int nIndex3 = ccc.Find(_T("\",\"order_id\":\""));
			if (nIndex3 == -1)
			{
				return FALSE;
			}
			//索引前移14个字符就是，我们需要找的order_id
			nIndex3 = nIndex3 + 14;
			int nIndex32 = ccc.Find(_T("\""), nIndex3);
			if (nIndex32 == -1)
			{
				return FALSE;
			}
			index_n = nIndex32 - nIndex3;
			CString str3 = ccc.Mid(nIndex3, index_n);
			str3 = _T("用户代码：") + str3;
			AfxMessageBox(str3);

			//**********************************************************货架号**********************************************************
			//_T(",\"exit_shelf_number\":\"")  是根据json格式定义的特点
			int nIndex4 = ccc.Find(_T(",\"exit_shelf_number\":\""));
			if (nIndex4 == -1)
			{
				return FALSE;
			}
			//索引前移14个字符就是，我们需要找的order_id
			nIndex4 = nIndex4 + 22;
			int nIndex42 = ccc.Find(_T("\""), nIndex4);
			if (nIndex42 == -1)
			{
				return FALSE;
			}
			index_n = nIndex42 - nIndex4;
			CString str4 = ccc.Mid(nIndex4, index_n);
			str4 = _T("货架号：") + str4;
			AfxMessageBox(str4);			
		}
















		pFile->Close();
		pHttpConnection->Close();
		//指针是需要用户手动释放资源的
		delete pFile;
		delete pHttpConnection;
		session.Close();

		return TRUE;
	}
}