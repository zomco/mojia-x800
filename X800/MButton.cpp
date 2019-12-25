// MButton.cpp: 实现文件
//

#include "stdafx.h"
#include "MButton.h"


//而诸多老师为了方便辨认，以及宏名常用大写表示，所以常将“标识符A”写成“_头文件名大写_H”。
//实际上这里的“标识符A”的名称与头文件名称没有什么必然联系。
#define M_BTN "__M_BTN__"
//目的：避免重复被包含
#ifndef IDC_HAND
#define IDC_HAND				MAKEINTRESOURCE(32649) 
#endif // !IDC_HAND
//这个不需要奇怪，这个数字只是系统预定义的图标资源的ID号。

//MFC系统的代码 ---- 暂时不用管它
// MButton
IMPLEMENT_DYNAMIC(MButton, CWnd)

MButton::MButton()
{
	//初始化按钮三态相关变量的值
	btn_down = FALSE;
	btn_bTrack = FALSE;
}

MButton::~MButton()
{
}

//注册函数
void MButton::Register()
{
	//是将第一个成员初始化为0
	//WNDCLASS是一个结构体
	WNDCLASS windowclass = { 0 };
	//获取当前.exe的句柄
	HINSTANCE hInst = AfxGetInstanceHandle();
	//CS_HREDRAW当窗口水平方向的宽度变化时重绘整个窗口.CS_VREDRAW 当窗口垂直方向的宽度变化时重绘整个窗口.
	windowclass.style = CS_HREDRAW | CS_VREDRAW;
	//WindowProc是你给自己的窗口定义的窗口处理函数
	//DefWindowProc是windows平台提供的默认窗口处理函数
	windowclass.lpfnWndProc = ::DefWindowProc;
	//实例句柄
	windowclass.hInstance = hInst;
	//窗口的最小化图标
	windowclass.hIcon = NULL;
	windowclass.hCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);//IDC_ARROW   IDC_HAND
	windowclass.hbrBackground = ::GetSysColorBrush(COLOR_BTNFACE);
	//HBRUSH hbrBackground;----窗口背景色
	//LPCTSTR lpszMenuName;----窗口菜单
	//注册窗口类型名
	windowclass.lpszClassName = _T(M_BTN);
	AfxRegisterClass(&windowclass);
}

//创建MButton函数
BOOL  MButton::Create(LPCTSTR szCapt, DWORD dwStyle, const RECT& rect, CWnd* pParent, UINT nID)
{
	static BOOL b = TRUE;
	if (b)
	{
		//只执行一次
		b = FALSE;
		Register();
	}
	return CWnd::Create(_T(M_BTN), szCapt, dwStyle, rect, pParent, nID);
}


//消息映射map
BEGIN_MESSAGE_MAP(MButton, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


//MButton 消息处理程序
BOOL MButton::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//去除背景绘图，可以减少闪烁
	//return CWnd::OnEraseBkgnd(pDC);
	return TRUE;
}

//----------------------------------------------------------------------------------------------------------------------
void MButton::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus(pNewWnd);
}


//这个函数存在的意义就是：设置btn_down变量的值
void MButton::OnLButtonDown(UINT nFlags, CPoint point)
{
	//这个有疑问-----可能不要----等下试一试
	SetFocus();
	//当按下后，就标记btn_down = TRUE; 为onpaint准备好判断变量
	btn_down = TRUE;
	//触发OnPaint（）函数，重新绘制MButton的状态
	Invalidate(FALSE);
	//GetParent()->SendMessage 
	//以后可能需要
	CWnd::OnLButtonDown(nFlags, point);
}


//这个函数存在的意义就是：设置btn_down变量的值
void MButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	btn_down = FALSE;
	//注意鼠标弹起的收不一定在按钮Track范围里面，因为按下后，可以移动鼠标在其他地方up,所以要CRect代码
	CRect rect;
	GetClientRect(rect);
	//按下并弹起向父窗口反射命令消息
	//当鼠标在track范围之外up的时候，不会触发按钮消息。
	if (rect.PtInRect(point))
	{
		GetParent()->SendMessage(WM_COMMAND, GetDlgCtrlID(), 0);
	}
	//触发OnPaint（）函数，重新绘制MButton的状态
	Invalidate(FALSE);
	CWnd::OnLButtonUp(nFlags, point);
}


void MButton::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CWnd::OnPaint()
	CBitmap bmp;
	//bmp.LoadBitmap(IDB_BITMAP1);
	//当鼠标在track外面且鼠标已经up了----正常颜色
	if (btn_bTrack == FALSE && btn_down == FALSE)
	{
		bmp.LoadBitmap(normal);
	}
	//当鼠标在track里面,且鼠标是up状态 ----灰色 
	if (btn_bTrack == TRUE && btn_down == FALSE)
	{
		bmp.LoadBitmap(gray);
	}
	//当鼠标在track外面且鼠标是down状态----灰色
	if (btn_bTrack == FALSE && btn_down == TRUE)
	{
		bmp.LoadBitmap(gray);
	}
	//当鼠标在track外面且鼠标是down状态----加量
	if (btn_bTrack == TRUE && btn_down == TRUE)
	{
		bmp.LoadBitmap(light);
	}
	//申明一个bitmap结构体，为了修改bitmap属性比较方便
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
}


//这个函数存在的意义就是：设置btn_bTrack变量的值
void MButton::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect rect;
	GetClientRect(rect);

	if (rect.PtInRect(point))
	{
		//为控件捕获鼠标消息---这样鼠标的操作消息，这个控件才可以接收到
		if (GetCapture() != this)
		{
			SetCapture();
		}
		//为OnPaint提供三态按钮的核心参数
		btn_bTrack = TRUE;
	}
	else
	{
		//应为鼠标按下去的时候。还是可以移动鼠标的，这个时候有可能鼠标不在rect范围里面
		if (!btn_down)
		{
			ReleaseCapture();
		}
		btn_bTrack = FALSE;
	}
	//触发OnPaint（）函数，重新绘制MButton的状态
	Invalidate(FALSE);
	CWnd::OnMouseMove(nFlags, point);
}
