#pragma once
class MCoordinate
{
public:
	MCoordinate();
	~MCoordinate();




	//********************************************interface坐标数据********************************************
	//水平第一条线
	CRect lev_f;		//level
	//水平第二条线
	CRect lev_s;
	//水平第三条线
	CRect lev_t;
	//竖直第一条线
	CRect ver_f;		//vertical

	//********************************************控件 ---- 坐标数据********************************************
	// ---------button--------
	CRect b_close;			//关闭窗口
	CRect b_small;			//最小化窗口

	CRect b_file;
	CRect b_date;
	CRect b_set;
	CRect b_help;
	CRect b_split;

	// ---------picture--------
	CRect p_logo;
	//
	CRect p_length;
	CRect p_wide;
	CRect p_height;
	CRect p_volume;
	CRect p_weight;
	CRect p_barcode;
	CRect p_money;
	//照片 ---- "包裹取证" 
	CRect p_evidence;

	// -----------tab----------
	CRect t_length;
	CRect t_wide;
	CRect t_height;
	CRect t_volume;
	CRect t_weight;
	CRect t_barcode;
	CRect t_money;
	//标题tab ---- “包裹数据”，“拍照取证”
	CRect t_box_date;
	CRect t_pic_evidence;




	//框架坐标初始化函数
	void frame_layer();
	//框架坐标初始化函数,适用于所有显示屏
	void frame_layer_ai();
	//控件坐标初始化函数
	void con_layer();
	void con_layer_ai();
	//对画包裹长宽高进行参考点设置
	void set_reference(int ref[4]);
	void set_reference_ai(int ref[4]);





	//对x坐标进行变换
	int transform_x(int x);
	//对y坐标进行变换
	int transform_y(int y);
};

