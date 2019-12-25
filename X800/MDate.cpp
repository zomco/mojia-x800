#include "stdafx.h"
#include "MDate.h"




//--------------------标记参数	------------------
//静态数据成员可以初始化，但只能在类体外面进行初始化
int MDate::paint_flag = 0;
int MDate::comm_count = 3;

int MDate::max_height = 800;
int MDate::max_length = 840;


//最小外接矩形是否是切角后的，1代表是，0代表不是
int MDate::cut_outline = 1;
int MDate::cut_corner = 0;

//机器工作的标记参数，1代表工作中（就是光幕随时准备好了工作），0代表没有工作中
int MDate::machine_start = 0;

//数据推送处理模式参数，1代表本地(入库)，0代表服务器（出库）
int MDate::date_model = 1;






//5、*********************************************客户修改数据*********************************************
//正数就是加尺寸，负数就是减尺寸，单位是毫米（mm）
int MDate::length_change	= 0;
int MDate::width_change		= 0;
int MDate::height_change	= 0;
//6、*********************************************保存数据格式*********************************************
//0,为原始数据，1为进位后的数据
int MDate::save_date_orginal = 0;



int MDate::yi_jida_flag = 0;//0代表将要扫国家条码信息，1代表将要扫包裹条码信息
int MDate::scan_door = 1;//等于1代表开发扫码功能，等于0代表关闭扫码功能

















MDate::MDate()
{
}


MDate::~MDate()
{
}


//输出数字方便 ---- 同时在程序部署的时候方便隐藏提示信息
void MDate::message(int in, CString str)
{
	/*
	CString ss;
	ss.Format(_T(" %d -- "), in);
	ss = ss + str;
	AfxMessageBox(ss);
	*/
}

void MDate::message(CString str)
{
	//AfxMessageBox(str);
}

void MDate::message(int in)
{
	CString ss;
	ss.Format(_T("- %d -"), in);
	AfxMessageBox(ss);
}

int MDate::message_ok(CString str)
{
	int n = AfxMessageBox(str, MB_OKCANCEL | MB_ICONQUESTION | MB_SYSTEMMODAL);
	return n;
}

void MDate::message(double dou)
{
	CString ss;
	ss.Format(_T(" %f "), dou);
	AfxMessageBox(ss);
}
