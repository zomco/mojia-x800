#pragma once
class MPoint
{
public:
	MPoint();
	~MPoint();

	//y = k*x + b; 一次函数的2个参数
	double k;
	int b;
	BOOL slope = TRUE;//有斜率
	int  slope_x;
};

