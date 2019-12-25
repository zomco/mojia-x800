// MPicture.cpp: 实现文件

#include "stdafx.h"
#include "MPicture.h"

// MPicture
IMPLEMENT_DYNAMIC(MPicture, CWnd)

MPicture::MPicture()
{
	//照片是否添加roundrect边框效果
	style_flag = 0;
}

MPicture::~MPicture()
{
}

//注册函数
void MPicture::Register()
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
	windowclass.lpszClassName = _T("M_Picture");
	AfxRegisterClass(&windowclass);
}


//创建MButton函数
BOOL  MPicture::Create(LPCTSTR szCapt, DWORD dwStyle, const RECT& rect, CWnd* pParent, UINT nID)
{
	static BOOL b = TRUE;
	if (b)
	{//只执行一次

		b = FALSE;
		Register();
	}
	return CWnd::Create(_T("M_Picture"), szCapt, dwStyle, rect, pParent, nID);
}


//消息映射
BEGIN_MESSAGE_MAP(MPicture, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// MPicture 消息处理程序
void MPicture::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CWnd::OnPaint()
	CBitmap bmp;
	bmp.LoadBitmap(normal);
	//获取bitmap的属性参数
	BITMAP bm;
	bmp.GetBitmap(&bm);
	//创建兼容内存DC并选择背景图
	CDC cdc;
	cdc.CreateCompatibleDC(&dc);
	cdc.SelectObject(&bmp);
	//开始加载图片------------------------------------------------------------------------------------------------
	CRect rect;
	GetClientRect(rect);
	dc.SetStretchBltMode(COLORONCOLOR);
	dc.StretchBlt(0, 0, rect.right, rect.bottom, &cdc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
	//是否要添加roundrect效果
	if (style_flag == 1)
	{
		//画一个圆角矩形框
		CPen pen(PS_SOLID, 1, RGB(255, 255, 255));
		//NULL_BRUSH 就是不填充的意思
		dc.SelectStockObject(NULL_BRUSH);
		dc.SelectObject(&pen);
		dc.RoundRect(rect, { 10,10 });
	}
}
