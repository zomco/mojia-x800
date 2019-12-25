// MTab.cpp: 实现文件

#include "stdafx.h"
//#include "Volume_Measure.h"
#include "MTab.h"




#define M_TAB "__M_TAB__"

//SystemColors.GradientInactiveCaption 属性,获取一个 Color 结构，它是非活动窗口标题栏的颜色渐变中的最亮色。
#ifndef COLOR_GRADIENTINACTIVECAPTION
#define CLLOR_GRADIENTINACTIVECAPTION	28
#endif

// MTab
IMPLEMENT_DYNAMIC(MTab, CWnd)

//构造函数
MTab::MTab()
{
	//标签是否是主标题
	style_flag = 0;
}
//类的对象的销毁函数
MTab::~MTab()
{
}

//注册函数
void MTab::Register()
{
	WNDCLASS windowclass = { 0 };
	HINSTANCE hInst = AfxGetInstanceHandle();
	windowclass.style = CS_HREDRAW | CS_VREDRAW;
	/*
	WNDCLASSEX 中最重要的成员莫过于lpfnWndProc了。前缀 lpfn （long point function）表示该成员是一个指向函数的长指针。
	在 Win32中由于内存模式是 FLAT 型，所以没有 near 或 far 的区别。每一个窗口类必须有一个窗口过程，当 Windows 把属于
	特定窗口的消息发送给该窗口时，该窗口的窗口类负责处理所有的消息，如键盘消息或鼠标消息。由于窗口过程差不多智能地处
	理了所有的窗口消息循环，所以您只要在其中加入消息处理过程即可。
	*/
	windowclass.lpfnWndProc = ::DefWindowProc;
	windowclass.hInstance = hInst;
	windowclass.hIcon = NULL;
	windowclass.hCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);//标准的箭头
	windowclass.hbrBackground = ::GetSysColorBrush(COLOR_BTNSHADOW);
	//NULL 表示和父窗口保持一致的RGB
	//windowclass.hbrBackground = NULL;
	windowclass.lpszClassName = _T(M_TAB);
	AfxRegisterClass(&windowclass);
}

//创建tab对象的函数
BOOL MTab::Create(DWORD dwStyle, const RECT& rect, CWnd* pParent, UINT nID)
{
	static BOOL b = TRUE;
	if (b)
	{	//只执行一次	
		b = FALSE;
		Register();
	}
	return CWnd::Create(_T(M_TAB), NULL, dwStyle, rect, pParent, nID);
}


BEGIN_MESSAGE_MAP(MTab, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// MTab 消息处理程序
void MTab::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CWnd::OnPaint()
					   //set font for dc
					   //给dc字体----因为我们要用drawtext函数

	if (style_flag == 1)
	{
		CFont font;
		font.CreatePointFont(145, _T("宋体"));
		dc.SelectObject(font);
		//这就需要使用SetBkMode函数来设置DrawText函数的输出方式，显示设备共有两种输出方式：OPAQUE和TRANSPARENT。
		//OPAQUE的方式是用当前背景的画刷的颜色输出显示文字的背景，而TRANSPARENT是使用透明的输出，也就是文字的背景是不改变的。
		dc.SetBkMode(TRANSPARENT);

		CRect rect;
		GetClientRect(rect);

		//draw 文字之前要把之前的文字给抹除掉
		//VC MFC中获得当前窗体（客户区）的颜色
		//使用FillSolidRect 就不要在定义CBrush br1(GetSysColor(COLOR_3DFACE))了。
		dc.FillSolidRect(rect, RGB(45, 45, 48));//

												//填充roundrect阴影
		CBrush br(RGB(126, 126, 128));
		dc.SelectObject(&br);
		dc.RoundRect(rect, { 10,10 });

		//设置字体颜色
		dc.SetTextColor(RGB(255, 255, 255));
		//文字水平方向靠左，竖直方向居中，单行
		dc.DrawText(cs_text, cs_text.GetLength(), rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	}
	else
	{
		CFont font;
		font.CreatePointFont(145, _T("黑体"));//145
		dc.SelectObject(font);
		//这就需要使用SetBkMode函数来设置DrawText函数的输出方式，显示设备共有两种输出方式：OPAQUE和TRANSPARENT。
		//OPAQUE的方式是用当前背景的画刷的颜色输出显示文字的背景，而TRANSPARENT是使用透明的输出，也就是文字的背景是不改变的。
		dc.SetBkMode(TRANSPARENT);

		CRect rect;
		GetClientRect(rect);

		//draw 文字之前要把之前的文字给抹除掉
		//VC MFC中获得当前窗体（客户区）的颜色
		//使用FillSolidRectan 就不要在定义CBrush br1(GetSysColor(COLOR_3DFACE))了。
		dc.FillSolidRect(rect, RGB(45, 45, 48));//

												//画一个圆角矩形框
		CPen pen(PS_SOLID, 1, RGB(255, 255, 255));
		//NULL_BRUSH 就是不填充的意思
		dc.SelectStockObject(NULL_BRUSH);
		dc.SelectObject(&pen);
		dc.RoundRect(rect, { 10,10 });

		//设置字体颜色
		dc.SetTextColor(RGB(255, 255, 255));
		//文字水平方向靠左，竖直方向居中，单行
		CRect rect_text = { rect.left + 5, rect.top, rect.right, rect.bottom };
		dc.DrawText(cs_text, cs_text.GetLength(), rect_text, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
	}
}



