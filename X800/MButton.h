#pragma once


// MButton
class MButton : public CWnd
{
	DECLARE_DYNAMIC(MButton)
public:
	MButton();
	virtual ~MButton();

protected:
	DECLARE_MESSAGE_MAP()

public:
	//鼠标光标相对于按钮的3种状态，我们设置3个bool变量来标记。在onpaint（）函数里面作为判断用。
	//按下时的状态
	BOOL btn_down;
	//鼠标跟踪时的状态--光标在按钮范围内
	BOOL btn_bTrack;

	//加载的3个图片resource
	UINT normal;
	UINT gray;
	UINT light;

	//注册自定义按钮的类型
	void Register();
	//按钮创建函数--格式与CButton::Create相同
	BOOL Create(LPCTSTR szCapt, DWORD dwStyle, const RECT& rect, CWnd* pParent, UINT nID);
	//这种简单的函数，可以在.h文件里面出现，也可以在.cpp文件里面出现
	void LoadBitMaps(UINT bitmap1, UINT bitmap2, UINT bitmap3)
	{
		normal = bitmap1;
		gray = bitmap2;
		light = bitmap3;
	}
	//message function
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};



