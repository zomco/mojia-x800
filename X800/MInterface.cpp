#include "stdafx.h"
#include "MInterface.h"
#include "MCoordinate.h"


MInterface::MInterface()
{
}


MInterface::~MInterface()
{
}




//画一条直线
void MInterface::paint(CPaintDC &dc, CRect r)
{
	dc.MoveTo(r.left, r.top);
	dc.LineTo(r.right, r.bottom);
}


//画界面框架
void MInterface::paint_frame(CPaintDC &dc)
{
	//定义一个画笔
	CPen pn(PS_SOLID, 2, RGB(255, 106, 106));
	CPen *pOldPen = dc.SelectObject(&pn);
	//给坐标赋值
	MCoordinate layer;
	layer.frame_layer();
	//水平第一条线
	paint(dc, layer.lev_f);
	//水平第二条线
	paint(dc, layer.lev_s);
	//水平第三条线
	paint(dc, layer.lev_t);
	//竖直第四条线
	paint(dc, layer.ver_f);
}