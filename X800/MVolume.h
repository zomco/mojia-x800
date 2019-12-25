#pragma once
/*
1、解析数据，必须是一个二位矩阵，得到高的图像数据和 长宽图像数据（通过数组的形式出现）
2、第一次过滤数据（过滤长宽的异常点）
3、计算长宽的最小外接矩形
4、切角
5、通过切角数据得到最小外接矩形
6、最小外接矩形
7、对切角数据做一次逼近算法最终确定长宽的值
*/


//这个语句一定要在
#include <opencv2/opencv.hpp>

//注意开发环境要设置成：debug / x64  环境。
using namespace std;
using namespace cv;



class MVolume
{
public:
	MVolume();
	~MVolume();


	//***********************************************串口*************************************************
	//串口接收数据的等待时间(前面3个数据的等待时间)，读取完重量数据后，
	int wait_time = 20000;
	//串口号
	int port_number = 0;
	//配置参数----param[0]----输入缓冲区；param[1]----输出缓冲区；param[2]----波特率
	//parameter[3] ---- 串口读数据的超时;parameter[4] ---- 串口写数据的超时;
	//这里设置的超时，会对wait_time变量的取值范围有影响
	int param[5] = { 16384, 1024, 115200, 20000, 20000 };//16384----24576
	//握手指令
	BYTE shake_date[6] = { 0xdd, 0x01, 0x01, 0x00, 0x00, 0xee };
	BYTE shake_return[6] = { 0xdd, 0x01, 0x01, 0x00, 0x00, 0xee };
	//***********************************************光幕*************************************************
	//1、初始化指令
	BYTE volume_init_send[6]	= { 0xdd, 0x01, 0x02, 0x00, 0x00, 0xee };
	BYTE volume_init_right[6]	= { 0xdd, 0x01, 0x02, 0x00, 0x00, 0xee };//
	BYTE volume_init_error[6]	= { 0xdd, 0x01, 0x02, 0xff, 0xff, 0xee };//这个指令暂时不用
	//2、开启光幕电源指令
	BYTE volume_start_send[6]	= { 0xdd, 0x01, 0x05, 0x00, 0x00, 0xee };
	BYTE volume_start_right[6]	= { 0xdd, 0x01, 0x05, 0x00, 0x00, 0xee };
	//3、关闭光幕电源指令
	BYTE volume_close_send[6]	= { 0xdd, 0x01, 0x06, 0x00, 0x00, 0xee };
	BYTE volume_close_right[6]	= { 0xdd, 0x01, 0x06, 0x00, 0x00, 0xee };
	//4、光幕开始工作指令
	BYTE volume_work_send[6]	= { 0xdd, 0x04, 0x01, 0x00, 0x00, 0xee };
	BYTE volume_work_right[6]	= { 0xdd, 0x04, 0x01, 0x00, 0x00, 0xee };
	//5、光幕停止工作指令
	BYTE volume_suspend_send[6]		= { 0xdd, 0x04, 0x02, 0x00, 0x00, 0xee };
	BYTE volume_suspend_right[6]	= { 0xdd, 0x04, 0x02, 0x00, 0x00, 0xee };
	//6、光幕检测
	BYTE volume_check_send[6]	= { 0xdd, 0x01, 0x03, 0x00, 0x00, 0xee };
	BYTE volume_check_right[6]	= { 0xdd, 0x01, 0x03, 0x00, 0x00, 0xee };
	BYTE volume_check_error[6]	= { 0xdd, 0x01, 0x03, 0xff, 0xff, 0xee };
	//7、光幕清洁模块工作指令
	//电机启动指令 ---- 电机由光幕串口控制
	BYTE clean_work[6]			= { 0xdd, 0x03, 0x01, 0x00, 0x00, 0xee };//命令输入后，清洁模块会来回清扫一次，清扫结束后才会发送返回数据。
	BYTE clean_return[6]		= { 0xdd, 0x03, 0x01, 0x00, 0x00, 0xee };
	//8、光幕重新发送包裹数据给上位机
	BYTE send_date_second[6]	= { 0xdd, 0x04, 0x03, 0x00, 0x00, 0xee };//dd 04 03 00 00 ee
	//***********************************************灯泡*************************************************
	//1、灯泡绿色，随时准备工作
	BYTE bulb_green_send[6]		= { 0xdd, 0x01, 0x07, 0x02, 0x00, 0xee };
	BYTE bulb_green_receive[6]	= { 0xdd, 0x01, 0x07, 0x02, 0x00, 0xee };
	//2、灯泡黄色，工作正在进行中
	BYTE bulb_yellow_send[6]	= { 0xdd, 0x01, 0x07, 0x03, 0x00, 0xee };
	BYTE bulb_yellow_receive[6] = { 0xdd, 0x01, 0x07, 0x03, 0x00, 0xee };
	//3、灯泡红色，工作异常
	BYTE bulb_red_send[6]		= { 0xdd, 0x01, 0x07, 0x01, 0x00, 0xee };
	BYTE bulb_red_receive[6]	= { 0xdd, 0x01, 0x07, 0x01, 0x00, 0xee };
	//4、关闭灯泡
	BYTE bulb_close_send[6]		= { 0xdd, 0x01, 0x07, 0x00, 0x00, 0xee };
	BYTE bulb_close_receive[6]	= { 0xdd, 0x01, 0x07, 0x00, 0x00, 0xee };
	//***********************************************电机*************************************************
	//1、控制电机速度指令
	//BYTE machine_spend_send第四个，和第五个数据代表变频器的频率0x20 == 频率是32（16进制）， 第4位代表整数位，第五位代表小数位
	BYTE machine_spend_send[6]	= { 0xdd, 0x02, 0x01, 0x19, 0x00, 0xee };//19 -- 32
	BYTE machine_spend_right[6] = { 0xdd, 0x02, 0x01, 0x00, 0x00, 0xee };
	BYTE machine_spend_error[6] = { 0xdd, 0x02, 0x01, 0xff, 0xff, 0xee };//这个指令暂时不用
	//2、电机启动指令 ---- 电机由光幕串口控制
	BYTE machine_work_send[6]	= { 0xdd, 0x02, 0x02, 0x00, 0x00, 0xee };
	BYTE machine_work_right[6]	= { 0xdd, 0x02, 0x02, 0x00, 0x00, 0xee };
	BYTE machine_work_error[6]	= { 0xdd, 0x02, 0x02, 0xff, 0xff, 0xee };//这个指令暂时不用
	//3、电机暂停指令 ---- 电机由光幕串口控制
	BYTE machine_suspend_send[6]	= { 0xdd, 0x02, 0x03, 0x00, 0x00, 0xee };
	BYTE machine_suspend_right[6]	= { 0xdd, 0x02, 0x03, 0x00, 0x00, 0xee };
	BYTE machine_suspend_error[6]	= { 0xdd, 0x02, 0x03, 0x00, 0x00, 0xee };//这个指令暂时不用
	//4、0代表电机暂停工作状态，1代表电机工作状态
	int machine_state = 0;
	//**********************************************编码器************************************************
	//3、编码器开机检查
	BYTE encoder_send[6]			= { 0xdd, 0x01, 0x04, 0x00, 0x00, 0xee };
	BYTE encoder_receive[6]			= { 0xdd, 0x01, 0x04, 0x00, 0x00, 0xee };
	BYTE encoder_receive_error[6]	= { 0xdd, 0x01, 0x04, 0xff, 0xff, 0xee };








	//****************************************************************************************************


	

	//*******************************************包裹数据*************************************************
	//存储包谷数据的变量 ---- 2048 * 4 * 2 = 16384（一个数据是2个字节）
	BYTE box_date[30000] = { 0 };
	//测量次数（扫码周期的数据 ---- 是周期的数量）
	int cycle_counts = 0;

	//长、宽的数据
	int len_width_begin[2048] = { 0 };
	int len_width_end[2048] = { 0 };
	//高的数据
	int height_up_begin[2048] = { 0 };
	int height_down_end[2048] = { 0 };

	//一个扫描周期对应的脉冲积分数
	int pulse_front[2048] = { 0 };
	int pulse_behind[2048] = { 0 };
	//编码器二合一的数据
	int pulse[2048] = { 0 };

	//包裹的长、宽、高信息
	//包裹的长
	float length_f = 0.0;
	//包裹的宽
	float width_f =  0.0;
	//包裹的高
	float height_f = 0.0;
	//包裹进位后的数据（小数点进位）
	int length = 0;
	int width =  0;
	int height = 0;
	//****************************************************************************************************





	//-------------------------编码器、滚轮相关信息-------------------------
	//编码器一个脉冲对应的距离，以mm为单位
	//double pulse_length = 0.102400;//第一次的数据
	//double pulse_length = 0.102837;//对第一次修正后的数据
	//double pulse_length = 0.102750;//对第二次修正后的数据
	double pulse_length = 0.051268;//对第二次修正后的数据0.100217 0.100751 0.101502



	//***********************************************************切角数据*******************************************************
	//切角的边长
	double cut_date = 0;
	//切角后的长、宽的数据
	int cut_len_width_begin[2048] = { 0 };
	int cut_len_width_end[2048] = { 0 };
	//切角的8个顶点数据
	int cut_point[4][2][2] = { 0 };
	//切角后的最小外接矩形
	RotatedRect cut_rectPoint;
	//**************************************************************************************************************************












	//***********************************************************串口*******************************************************
	//1、寻址串口号码函数
	BOOL find_port(int ports[]);
	//2、创建串口
	BOOL create_port(HANDLE& hCom);
	//3、读取数据信息 
	int read_date(HANDLE hCom);
	//***********************************************************光幕*******************************************************
	//1、光幕初始化
	BOOL init(HANDLE& hCom);
	//2、开启光幕电源
	BOOL volume_start(HANDLE hCom);
	//3、关掉光幕电源
	BOOL volume_close(HANDLE hCom);
	//4、光幕开始工作
	BOOL volume_work(HANDLE hCom);
	//5、光幕暂停工作
	BOOL volume_suspend(HANDLE hCom);
	//6、光幕检查
	BOOL volume_check(HANDLE hCom);
	//7、清洁光幕
	BOOL clean_screen(HANDLE hCom);
	//8、重新要求光幕发送包裹数据给上位机
	BOOL send_date_again(HANDLE hCom);
	//***********************************************************灯泡*******************************************************
	//1、灯泡绿色，随时准备工作
	BOOL bulb_green(HANDLE hCom);
	//2、灯泡黄色，工作正在进行中
	BOOL bulb_yellow(HANDLE hCom);
	//3、灯泡红色，工作异常
	BOOL bulb_red(HANDLE hCom);
	//4、关闭灯泡
	BOOL bulb_close(HANDLE hCom);
	//***********************************************************电机*******************************************************
	//1、设置变频器
	BOOL make_hz(HANDLE hCom);
	//2、电机启动
	BOOL machine_work(HANDLE hCom);
	//3、电机关闭
	BOOL machine_close(HANDLE hCom);
	//**********************************************************编码器******************************************************
	//1、光幕检查
	BOOL encoder_check(HANDLE hCom);

















	//***********************************************************体积*******************************************************
	//1、对串口发过来的数据进行解析
	void parse_date();
	//2、编码器数据
	void get_encoder_date();
	//3、编码器数据分析
	BOOL filter_encoder();
	//4、计算长宽的最小外接矩形
	RotatedRect get_min_rect();
	//5、对图像的4个角的数据进行切除
	void cut_corner();
	//6、切角后，原始数据不变，得到切角后的数据
	void cut_corner_date();
	//7、获取高度信息
	void get_height();
	//8、获取长宽的信息/切角后的
	void get_length_width_cut();
	//9、画图像/用切角后的数据
	void paint_image(CPaintDC& dc);
	//10、画包裹长、宽、高的外框
	void paint_outline(CPaintDC& dc);












	//5、画出包裹长、宽、高的图像,并且返回高的最大值
	void show_image(CPaintDC& dc);
	//6、计算出包裹的长宽高、并画出最小外接矩形
	void shou_outline(CPaintDC& dc);
	//7/计算出包裹切角后的长宽高、并画出最小外接矩形,和最高值的线条
	void cut_shou_outline(CPaintDC& dc);
	//8、计算体积，画出图像
	void show_volume(CPaintDC& dc, CString str_path);



	

	//**********************************************************************************************************************


	//***********************************************************算法*******************************************************
	//求一个脉冲对应输送带运动的距离
	//原理：通过计算没有被最小外界矩形的面积来判断 ---- 面积最小的情况下，就是最佳值
	void pulse_lenth_area();

	//求一个脉冲对应输送带运动的距离
	//原理：通过侧出来的长宽，和实际物体的长宽比较，差值最小的就是我们要的情况
	//int p_lenth,int p_width  标定物体的长和宽
	void pulse_lenth_distance(int p_lenth, int p_width);

	



	//**********************************************************************************************************************

	//*******************************************************图像处理*******************************************************
	

	//2、图像保持到指定文件夹里面
	BOOL save_picture(CString str_path);
	//**********************************************************************************************************************



	String toString(CString cs);




};

