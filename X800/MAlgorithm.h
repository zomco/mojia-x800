#pragma once
#include "MPoint.h"


class MAlgorithm
{
public:
	MAlgorithm();
	~MAlgorithm();


	
	//1、二维坐标系中的2个点的直线，确定直线上一个点（这个点到起点的距离是length）
	//2、POINT point_out 是要得到那个点
	//3、POINT point_1是起点；  POINT point_2是终点
	CPoint get_point(CPoint point_1, CPoint point_2, double length);

	//1、通过2个点确定一条直线
	//2、POINT point_1, POINT point_2直线上的2个点
	//3、y = k*x + b  MPoint point_out中的2个点是（k，b）
	MPoint get_line(CPoint point_1, CPoint point_2);

	//1、判断某个点和参考点是否在直线的同一边，同一边返回TURE，否则返回FALSE
	//2、point_1 某个点（需要判断的点），point_2参考点
	//3、point_out直线
	BOOL judge_boundary(CPoint point_1, CPoint point_2, MPoint point_out);
};

