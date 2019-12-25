#include "stdafx.h"
#include "MAlgorithm.h"
#include"math.h"



MAlgorithm::MAlgorithm()
{
}


MAlgorithm::~MAlgorithm()
{
}




//***********************************************************切角算法*******************************************************
//1、在一条直线上取点
/*
CPoint point_1		开始的点
CPoint point_2		终点
double length		离开始点的距离
CPoint point_out	在point_1，point_2，这2个点的直线上，距点point_1距离length长度，在点point_2方向上的点。
*/
CPoint MAlgorithm::get_point(CPoint point_1,CPoint point_2, double length)
{
	//1、计算POINT point_1, POINT point_2的长度
	double length_line = sqrt((point_1.x - point_2.x)*(point_1.x - point_2.x) + (point_1.y - point_2.y)*(point_1.y - point_2.y));
	//2、计算出，需要得到的长度，和2点之间直线长度的比例
	double percent = length / length_line;
	//3、计算出point_out的值来（坐标比例关系）
	CPoint point_out;
	point_out.x = (int)(percent*(point_2.x - point_1.x) + point_1.x);
	point_out.y = (int)(percent*(point_2.y - point_1.y) + point_1.y);

	return point_out;
}

//2、通过2个点得到一条直线
/*
CPoint point_1,CPoint point_2,	2个已知的点
MPoint point_out				通过2个已知的点得到的一条直线的2个参数k、b（y = k*x + b）
*/
MPoint MAlgorithm::get_line(CPoint point_1, CPoint point_2)
{
	MPoint point_out;

	if (point_1.x - point_2.x == 0)
	{
		//当垂直X轴的时候，斜率不存在。
		point_out.slope = FALSE;
		point_out.slope_x = point_1.x;
	}
	else
	{	//斜率存在的条件下
		point_out.k = ((double)(point_1.y - point_2.y)) / (point_1.x - point_2.x);
		point_out.b = (int)(point_1.y - point_1.x*point_out.k);
	}

	return point_out;
}

//3、判断点是否是要处理的点
/*
CPoint point_1			判断这个点是否是需要处理的点
CPoint point_2			参考点，要用这个点来判断point_1 是否是需要处理的点
MPoint point_out		参考直线。
*/
BOOL MAlgorithm::judge_boundary(CPoint point_1,CPoint point_2, MPoint point_out)
{
	
	BOOL bl_1;
	BOOL bl_2;

	if (point_out.slope == FALSE)
	{	//1、当直线没有斜率的时候
		//参考点和直线的比较
		if (point_2.x > point_out.slope_x)
		{
			bl_1 = TRUE;
		}
		else
		{
			bl_1 = FALSE;
		}
		//2、比较点和直线的关系
		if (point_1.x > point_out.slope_x)
		{
			bl_2 = TRUE;
		}
		else if(point_1.x < point_out.slope_x)
		{
			bl_2 = FALSE;
		}
		else
		{
			return FALSE;//比较点在直线上就不做处理
		}
		//3、确定当前比较点是否和参考点是一个方向
		if (bl_1 == bl_2)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{	//2、当直线有斜率的时候
		//1、参考点和直线的关系（在直线的上面，还是直线的下面） 
		double y_1 = (point_out.k*point_2.x) + point_out.b;
		if ((y_1 - (double)point_2.y) > 0)//x一样，直线上的点，和参考点比较
		{
			bl_1 = TRUE;
		}
		else
		{
			bl_1 = FALSE;
		}

		//2、比较点和直线的关系
		double y_2 = (point_out.k*point_1.x) + point_out.b;
		if ((y_2 - (double)point_1.y) > 0)//x一样，直线上的点，和参考点比较
		{
			bl_2 = TRUE;
		}
		else if ((y_2 - (double)point_1.y) < 0)
		{
			bl_2 = FALSE;
		}
		else
		{
			return FALSE;//比较点在直线上就不做处理
		}

		//3、确定当前比较点是否和参考点是一个方向
		if (bl_1 == bl_2)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}
//**************************************************************************************************************************

