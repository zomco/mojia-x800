#include "stdafx.h"
#include "MCoordinate.h"


MCoordinate::MCoordinate()
{
}


MCoordinate::~MCoordinate()
{
}




//框架坐标点
void MCoordinate::frame_layer_ai()
{
	//1、第一条水平线条
	lev_f = { 5, 49, 1920 - 5, 49 };

	//2、水平第二条线条  显示屏的尺寸：1920 1200
	//1200-49-2-3-3-3 = 1140 = 屏幕宽度、上面已经占据了的像素、下面已经占据了的像素、长宽上下各预留3个像素、高上面预留一个像素
	//长宽分配的像素1140*(840/(840+800)) = 584
	//高分配的像素1140*(800/(840+800)) = 556
	//(5 、 639 = 49+3+584+3)  840=光幕测量的距离
	//(f1,639)   f1 = f * (23.0 / 32.0);	
	int x2 = (int)(1920.0*(23.0 / 32.0));
	lev_s = { 5, 639, x2, 639};

	//3、水平第三条线条
	//(5, 1200-2)
	//(1920-5,1200-2)
	lev_t = { 5, 1198, 1915, 1198 };

	//4、竖直第一条线条
	//(x4,49)   x4=x2
	//(x4,1198)
	ver_f = { x2, 49, x2, 1198 };
}

//框架坐标点 ---- 自适应
void MCoordinate::frame_layer()
{
	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);

	//1、第一条水平线条
	lev_f = {transform_x(5), transform_x(49), transform_x(1920 - 5), transform_x(49)};
	
	//2、水平第二条线条  显示屏的尺寸：1920 1200
	//用于显示长宽，和用于显示高的像素，各站50%
	int y2 = (int)((height - transform_x(49))/2.0) + transform_x(49);
	//后面这个点x的坐标
	int x2 = (int)(width*(23.0 / 32.0));
	//水平第二条线条
	lev_s = {transform_x(5), y2, x2, y2 };

	//3、水平第三条线条
	//(5, 1200-2)
	//(1920-5,1200-2)
	lev_t = { 5, 1198, 1915, 1198 };
	lev_t = { transform_x(5), height -2 , transform_x(1920 - 5), height - 2 };

	//4、竖直第一条线条
	//(x2,49)  
	//(x2,1198)
	ver_f = { x2, transform_x(49), x2, height - 2 };
}

//控件坐标初始化函数
void  MCoordinate::con_layer_ai()
{
	CRect rect;
	//-------------------- button ---------------------
	//关闭窗口
	b_close = { 1869, 0, 1920, 39 };
	//最小化窗口			
	//(1920-51-51,0);(1920-51,39)
	b_small = { 1818, 0, 1869, 39 };

	b_file = { 430, 0, 503, 48 };
	b_date = { 513, 0, 587, 48 };
	b_set  = { 597, 0, 670, 48 };			
	b_help = { 680, 0, 756, 48 };
	b_split = { 763, 0, 839, 0 };
	//-------------------- picture ---------------------
	//logo
	p_logo = { 0, 0, 303,48 };
	//右上方的功能提示图片坐标赋值
	p_length  = { 1399, 150, 1560, 200 };
	p_wide    = { 1399, 210, 1560, 260 };
	p_height  = { 1399, 270, 1560, 320 };
	p_volume  = { 1399, 330, 1560, 380 };
	p_weight  = { 1399, 390, 1560, 440 };
	p_barcode = { 1399, 450, 1560, 500 };
	p_money   = { 1399, 510, 1560, 560 };
	//照片 ---- "包裹取证" 
	p_evidence = { 1399, 720, 1900, 1180 };


	// -------------------- tab --------------------
	t_length = { 1570, 150, 1900, 200 };
	t_wide   = { 1570, 210, 1900, 260 }; 
	t_height = { 1570, 270, 1900, 320 }; 
	t_volume = { 1570, 330, 1900, 380 }; 
	t_weight = { 1570, 390, 1900, 440 }; 
	t_barcode= { 1570, 450, 1900, 500 }; 
	t_money  = { 1570, 510, 1900, 560 }; 
	//标题tab ---- “包裹数据”，“拍照取证”
	t_box_date     = { 1399, 75,  1900, 135 }; 
	t_pic_evidence = { 1399, 645, 1900, 705 }; 
}

//控件坐标初始化函数 ---- 自适应
void MCoordinate::con_layer()
{
	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);

	//-------------------- button ---------------------
	//关闭窗口
	b_close = { transform_x(1869), 0, transform_x(1920), transform_x(39) };
	//最小化窗口			
	//(1920-51-51,0);(1920-51,39)
	b_small = { transform_x(1818), 0, transform_x(1869), transform_x(39) };

	//屏幕顶上的按钮
	b_file = { transform_x(430), 0, transform_x(503), transform_x(48) };
	b_date = { transform_x(513), 0, transform_x(587), transform_x(48) };
	b_set =  { transform_x(597), 0, transform_x(670), transform_x(48) };
	b_help = { transform_x(680), 0, transform_x(756), transform_x(48) };
	b_split = { transform_x(763), 0, transform_x(839), transform_x(48) };

	//logo
	p_logo = { 0, 0, transform_x(303), transform_x(48) };
	
	//-------------------- picture ---------------------
	//测量信息显示标签
	p_length	= { transform_x(1399), transform_x(150), transform_x(1560), transform_x(200) };
	p_wide		= { transform_x(1399), transform_x(210), transform_x(1560), transform_x(260) };
	p_height	= { transform_x(1399), transform_x(270), transform_x(1560), transform_x(320) };
	p_volume	= { transform_x(1399), transform_x(330), transform_x(1560), transform_x(380) };
	p_weight	= { transform_x(1399), transform_x(390), transform_x(1560), transform_x(440) };
	p_barcode	= { transform_x(1399), transform_x(450), transform_x(1560), transform_x(500) };
	p_money		= { transform_x(1399), transform_x(510), transform_x(1560), transform_x(560) };

	//照片 ---- "包裹取证" 
	p_evidence	= { transform_x(1399), transform_x(720), transform_x(1900), height-2 };
	
	// -------------------- tab --------------------
	t_length	= { transform_x(1570), transform_x(150), transform_x(1900), transform_x(200) };
	t_wide		= { transform_x(1570), transform_x(210), transform_x(1900), transform_x(260) };
	t_height	= { transform_x(1570), transform_x(270), transform_x(1900), transform_x(320) };
	t_volume	= { transform_x(1570), transform_x(330), transform_x(1900), transform_x(380) };
	t_weight	= { transform_x(1570), transform_x(390), transform_x(1900), transform_x(440) };
	t_barcode	= { transform_x(1570), transform_x(450), transform_x(1900), transform_x(500) };
	t_money		= { transform_x(1570), transform_x(510), transform_x(1900), transform_x(560) };
	//标题tab ---- “包裹数据”，“拍照取证”
	t_box_date		= { transform_x(1399), transform_x(75), transform_x(1900), transform_x(135) };
	t_pic_evidence	= { transform_x(1399), transform_x(645), transform_x(1900), transform_x(705) };
}

//定位点
void MCoordinate::set_reference_ai(int ref[4])
{
	//屏幕像素的数量
	int x = GetSystemMetrics(SM_CXSCREEN);
	int y = GetSystemMetrics(SM_CYSCREEN);

	//右上角的定位点----下面要做除法，所以要把int换算成float。如果用int做除法，结果只保留int为的数据
	float fx = (float)x;
	//float fy = (float)y;
	//这个坐标点代表x轴上点的位置，是电脑屏幕上的比例----方便在所有屏幕上适配（是一个比例，不是一个确定的点） 
	double bx = (fx * 23.0) / 32.0;

	ref[0] = (int)bx - 80;
	ref[1] = 49;
	ref[2] = ref[0];
	ref[3] = y - 2;
}

//定位点 ---- 自适应
void MCoordinate::set_reference(int ref[4])
{
	//屏幕像素的数量
	int x = GetSystemMetrics(SM_CXSCREEN);
	int y = GetSystemMetrics(SM_CYSCREEN);

	//右上角的定位点----下面要做除法，所以要把int换算成float。如果用int做除法，结果只保留int为的数据
	float fx = (float)x;
	//float fy = (float)y;
	//这个坐标点代表x轴上点的位置，是电脑屏幕上的比例----方便在所有屏幕上适配（是一个比例，不是一个确定的点） 
	double bx = (fx * 23.0) / 32.0;

	ref[0] = (int)bx - transform_x(80);
	ref[1] = transform_x(49);
	ref[2] = ref[0];
	ref[3] = y - 2;
}


//对x坐标进行变换
int MCoordinate::transform_x(int x)
{
	int width = GetSystemMetrics(SM_CXSCREEN);
	
	return (int)((x / 1920.0) * width);
}


//对y坐标进行变换
int MCoordinate::transform_y(int y)
{
	int height = GetSystemMetrics(SM_CYSCREEN);

	return (int)((y / 1200.0) * height);
}