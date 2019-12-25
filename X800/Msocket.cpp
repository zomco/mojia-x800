#include "stdafx.h"
#include "Msocket.h"
#include "resource.h"
#include "MDate.h"
#include <string>
#include "md5c.h"


Msocket::Msocket()
{
}


Msocket::~Msocket()
{
}


void Msocket::Mconnect()
{
	Create();
	if (!Connect(_T("119.23.144.133"), 80))
	{
		Close();
		AfxMessageBox(_T("链接服务器失败"));
	}
	else
	{
		AfxMessageBox(_T("链接服务器成功"));
	}
	
	//int cmd = INF_ADD;
	//Send(&cmd, sizeof(cmd));


	

	CString str_to_md5;
	//1、将“高”添加到字符串里面去----“measuring”是登录的账号
	str_to_md5 = L"appid=measuring&height=";
	str_to_md5 += L"76.0";
	//2、将“长”添加到字符串里面去
	str_to_md5 += L"&length=";
	str_to_md5 += L"76.0";
	//3、将“条码”添加到字符串里面去
	str_to_md5 += L"&orderNo=";
	str_to_md5 += L"566000546152";
	//4、将“重量”添加到字符串里面去
	str_to_md5 += L"&weight=";
	str_to_md5 += L"760.0";
	//5、将“宽”添加到字符串里面去
	str_to_md5 += L"&width=";
	str_to_md5 += L"76.0";
	//6、将“key”添加到字符串里面去----pass_word是登录的密码，这个密码是原始密码经过MD5加密后得到的数据
	/*
	比如：key = 994be6 这个是登录密码（这个是原始密码），这个登录密码经过MD5加密后得到
	pass_word = 9a68e8bcb7f17e106b535fd71a88baf3（就是下面语句用到的）
	*/
	str_to_md5 += L"4afda99e4e510c5f758e8ee4a07140bc";//pass_word;//这个是：key加密后的数据


	/*
	第二步：将str_to_md5字符串加密生产一个md5格式的秘钥保存在cstr_md5字符串变量中

	备注：
	宽字节转多字节，转换成UTF-8格式（这个一般是网路传输的时候用的格式）。
	因为都是英文字母，所以转换后都是单字节格式，所以可以用char定义变量。
	本地系统默认Unicode编码，而网络传输一般用UTF-8编码，如果不懂百度学习一下这2种编码的区别。
	*/
	//1、我猜CString 是用Unicode编码，一个字符占用2个字节，要将他们都转换成单字节格式
	USES_CONVERSION;
	std::string str = W2A(str_to_md5);
	//下面用到的加密函数不支持string的变量，只支持char*型的变量，所有要将std::string变量转换成char*型变量 
	char *md5data = (char*)str.c_str();
	//2、生成秘钥
	//这个变量用来保存----sign签名秘钥
	unsigned char md[16];
	//制作秘钥的对象
	md5c mm;
	//void MDString(char *string, unsigned char digest[16]);----原始函数的格式
	//生成秘钥的函数
	mm.MDString(md5data, md);
	//3、得到了string格式的----签名秘钥
	std::string strMd5 = "";
	char tmp[3] = { 0 };
	for (int i = 0; i < 16; i++)
	{
		//X 表示以十六进制形式输出
		//02 表示不足两位，前面补0输出；出过两位，不影响
		sprintf_s(tmp, "%02X", md[i]);
		strMd5.append(tmp);
	}
	//4、得到了CString格式的----签名秘钥
	CString cstrMD5 = L"";
	cstrMD5 += strMd5.c_str();
	//5、最终得到----签名秘钥cstr_md5（小写）
	//测试cstr_md5显示的是大写的字母，必须做MakeLower（）处理
	CString cstr_md5 = cstrMD5.MakeLower();//使所有大写字母变为小写


	


	
	

	




	CString appId = L"measuring";
	CString body = L"height=76.0&length=76.0&orderNo=566000546152&weight=760.0&width=76.0";
	CString sign = cstr_md5;//这个就是MD5加密后的sign ---- 我已经验证没有任何问题了












	//std::string sss = W2A(m_strRequest);


	








	//Send();







	



}

void Msocket::OnReceive(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类
	char s[2048];
	int nlen = Receive(s, sizeof(s) - 1);

	//如果发过来的是一个空数据
	if (nlen <= 0)
	{
		AfxMessageBox(_T("接收到服务器的一个空数据"));
		return;
	}



	
	//输出数据的长度。
	MDate::message(nlen);

	//输出数据。
	CString str(s);
	AfxMessageBox(str);

	CSocket::OnReceive(nErrorCode);
}
