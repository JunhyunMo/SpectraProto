
// ThicknessMeas_Proto.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CThicknessMeas_ProtoApp:
// �� Ŭ������ ������ ���ؼ��� ThicknessMeas_Proto.cpp�� �����Ͻʽÿ�.
//

class CThicknessMeas_ProtoApp : public CWinApp
{
public:
	CThicknessMeas_ProtoApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CThicknessMeas_ProtoApp theApp;