#pragma once


// MTab
class MTab : public CWnd
{
	DECLARE_DYNAMIC(MTab)

public:
	MTab();
	virtual ~MTab();

protected:
	DECLARE_MESSAGE_MAP()


	//-------------------- my code --------------------
public:
	CString cs_text;			//显示的文字

								//注册窗口类型
	void Register();
	//Tab初始化函数
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParent, UINT nID);
	//设置tab类型
	int style_flag;
	//消息映射函数
	afx_msg void OnPaint();
};


