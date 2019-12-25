#pragma once
class MInterface
{
public:
	MInterface();
	~MInterface();


	//画一条直线
	void paint(CPaintDC &dc, CRect r);
	//画界面框架
	void paint_frame(CPaintDC &dc);
};

