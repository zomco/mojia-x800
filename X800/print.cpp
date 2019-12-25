#include "print.h"
#include "stdafx.h"
#include "X800.h"
#include "X800Dlg.h"
#include "stdafx.h"
using namespace std;
using namespace BarTender;
using namespace System;

void PrintLable(char *ExpNum, char *CliNum, char *ShlNum, char *StaffName, char *PagSize, char *PagWeight)
{
	BarTender::Application ^btApp = gcnew BarTender::Application;
	BarTender::Format ^btFormat = gcnew BarTender::Format;
	//引用标签模板
	System::String^ str = gcnew System::String("C:\\paint\\file1.btw");//C:\Users\hua_zhi\Desktop\新建文件夹 (2)\print
	btFormat = btApp->Formats->Open(str, false, "");
	//打印份数
	btFormat->PrintSetup->IdenticalCopiesOfLabel = 1;
	//标签数
	btFormat->PrintSetup->NumberSerializedLabels = 1;


	//ExpressNumber单号
	System::String^ ExpressNumber = gcnew System::String(ExpNum);
	btFormat->SetNamedSubStringValue("ExpressNumber", ExpressNumber);//打印信息推送到模板相应位置
	//ClientName客户名
	System::String^ ClientName = gcnew System::String(CliNum);
	btFormat->SetNamedSubStringValue("ClientName", ClientName);
	//ShelfNumber货架号
	System::String^ ShelfNumber = gcnew System::String(ShlNum);
	btFormat->SetNamedSubStringValue("ShelfNumber", ShelfNumber);
	//PackingStaff打包人员
	System::String^ PackingStaff = gcnew System::String(StaffName);
	btFormat->SetNamedSubStringValue("PackingStaff", PackingStaff);
	//Size尺寸
	System::String^ Size = gcnew System::String(PagSize);
	btFormat->SetNamedSubStringValue("Size", Size);
	//Weight尺寸
	System::String^ Weight = gcnew System::String(PagWeight);
	btFormat->SetNamedSubStringValue("Weight", Weight);

	btFormat->PrintOut(false, false);
	//退出时不保存模板
	btFormat->Close(BarTender::BtSaveOptions::btDoNotSaveChanges);
	//bartender进行同步界面退出
	btApp->Quit(BarTender::BtSaveOptions::btDoNotSaveChanges);
}