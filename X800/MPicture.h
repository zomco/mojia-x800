#pragma once


// MPicture
class MPicture : public CWnd
{
	DECLARE_DYNAMIC(MPicture)

public:
	MPicture();
	virtual ~MPicture();

protected:
	DECLARE_MESSAGE_MAP()


public:
	//注册窗口类型
	void Register();
	//Tab初始化函数
	BOOL Create(LPCTSTR szCapt, DWORD dwStyle, const RECT& rect, CWnd* pParent, UINT nID);
	//加载的图片resource
	UINT normal;
	int style_flag;
	void LoadBitMaps(UINT bitmap)
	{
		normal = bitmap;
	}
	//
	void LoadBitMaps(UINT bitmap, int style)
	{
		normal = bitmap;
		style_flag = style;
	}

	//消息映射函数
	afx_msg void OnPaint();
};