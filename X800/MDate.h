//经常需要改动的数据----放在一个全局类中
#pragma once
class MDate
{
public:
	MDate();
	~MDate();



	//经常需要修改的数据就放在这个类里面来
public:
	//--------------------标记参数	--------------------
	//开机后，扫码前这个参数赋值0，扫码之后这个参数赋值1，这个参数是给onpaint函数用的，如果这个值是1，就会执行画图函数
	static int paint_flag;			//画包谷图像是否执行的标记号
	static int comm_count;			//串口的总数量
	static int max_height;			//机器测量包裹高的最大值
	static int max_length;			//机器的宽度


	//注意图像一直都是原始数据的图像，这里讨论的是最小外接矩形。
	static int cut_outline;			//切角数据后 ---- 包裹最小外接矩形图像，1代表切角数据图像，0代表原始数据图像
	static int cut_corner;			//是否需要画切角效果 ---- 1代表有切角效果，0代表没有切角效果
	

									
	//设备工作的标记参数
	static int machine_start;


	//数据推送处理模式参数
	static int date_model;



	//5、*********************************************客户修改数据*********************************************
	static int length_change;
	static int width_change;
	static int height_change;
	//6、*********************************************保存数据格式*********************************************
	static int save_date_orginal;//0,为原始数据，1为进位后的数据


	//当前条码是国家还是包裹信息
	//扫码标志数据，如果是0说明是第一次扫码，需要扫国际，如果是1说明是第二次扫码，需要扫包裹条码
	static int yi_jida_flag;
	static int scan_door;



	






	//输出数字方便 ---- 同时在程序部署的时候方便隐藏提示信息
	static void message(int in, CString str);
	static void message(CString str);
	static void message(int in);
	static int message_ok(CString str);
	static void message(double dou);
};