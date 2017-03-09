
// ThicknessMeas_ProtoDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "ThicknessMeas_Proto.h"
#include "ThicknessMeas_ProtoDlg.h"
#include "afxdialogex.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//for TIMER readability
#define MESURE_POWER (100)
#define DUMMY_MEASURE (101)

static const int DataRateTimer = 1;
static const int ChartUpdateTimer = 2;
static const int DataInterval = 250;

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CThicknessMeas_ProtoDlg 대화 상자

CThicknessMeas_ProtoDlg::CThicknessMeas_ProtoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CThicknessMeas_ProtoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	//MJH
	m_InstrHdl_PM100D = VI_NULL; 
	m_InstrHdl_CLD1015 = VI_NULL;
	m_ftHandle = NULL;
	m_nTotalScan = 0;
	m_nNGcount = 0;
}

void CThicknessMeas_ProtoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LOG, m_ctlListLog);
	DDX_Control(pDX, IDC_STATIC_POWER, m_ctlSTpower);
	DDX_Control(pDX, IDC_CHART, m_ChartViewer);
}

BEGIN_MESSAGE_MAP(CThicknessMeas_ProtoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BT_INIT_PM100D, &CThicknessMeas_ProtoDlg::OnBnClickedBtInitPm100d)
	ON_BN_CLICKED(IDC_BT_CLOSE_PM100D, &CThicknessMeas_ProtoDlg::OnBnClickedBtClosePm100d)
	ON_BN_CLICKED(IDC_BT_INIT_CLD1015, &CThicknessMeas_ProtoDlg::OnBnClickedBtInitCld1015)
	ON_BN_CLICKED(IDC_BT_CLOSE_CLD1015, &CThicknessMeas_ProtoDlg::OnBnClickedBtCloseCld1015)
	ON_BN_CLICKED(IDC_BT_OPEN, &CThicknessMeas_ProtoDlg::OnBnClickedBtOpen)
	ON_BN_CLICKED(IDC_BT_CLOSE, &CThicknessMeas_ProtoDlg::OnBnClickedBtClose)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_MFCBT_TEC, &CThicknessMeas_ProtoDlg::OnBnClickedMfcbtTec)
	ON_BN_CLICKED(IDC_MFCBT_LASER, &CThicknessMeas_ProtoDlg::OnBnClickedMfcbtLaser)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BT_DUMMY_MEAS, &CThicknessMeas_ProtoDlg::OnBnClickedBtDummyMeas)
	ON_BN_CLICKED(IDC_BT_QUIT, &CThicknessMeas_ProtoDlg::OnBnClickedBtQuit)
	ON_CONTROL(CVN_ViewPortChanged, IDC_CHART, OnViewPortChanged)
	ON_BN_CLICKED(IDC_BT_WR_CMD, &CThicknessMeas_ProtoDlg::OnBnClickedBtWrCmd)
END_MESSAGE_MAP()


// CThicknessMeas_ProtoDlg 메시지 처리기

BOOL CThicknessMeas_ProtoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	//font 조정 
	 m_Font.CreateFont(26, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, DEFAULT_CHARSET,
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                    DEFAULT_PITCH | FF_SWISS, L"굴림체");

    // 버튼객체의 SetFont() 함수를 이용하여 생성한 폰트를 적용시킨다.
    m_ctlSTpower.SetFont(&m_Font, TRUE);

	//Chart
	for (int i = 0; i < sampleSize; ++i)
	{
        m_timeStamps[i] = m_dataSeriesA[i] = Chart::NoValue;
	}

	// Set m_nextDataTime to the current time. It is used by the real time random number 
    // generator so it knows what timestamp should be used for the next data point.
    SYSTEMTIME st;
    GetLocalTime(&st);
    m_nextDataTime = Chart::chartTime(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, 
        st.wSecond) + st.wMilliseconds / 1000.0;

	 // Initialize member variables
    m_extBgColor = getDefaultBgColor();     // Default background color

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CThicknessMeas_ProtoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CThicknessMeas_ProtoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CThicknessMeas_ProtoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CThicknessMeas_ProtoDlg::DisplayLog(CString str)
{
	m_ctlListLog.AddString(str);
	int nCnt = m_ctlListLog.GetCount();

	while(nCnt > 2000)
	{  
		m_ctlListLog.DeleteString(0);
		break;
	}	  

	m_ctlListLog.SetCurSel(nCnt - 1);
}

void CThicknessMeas_ProtoDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	switch(nIDEvent)
	{
		case MESURE_POWER:
			MeasurePower();
			break;
		case DUMMY_MEASURE:
			DummyMeasure();
			break;
		default:
			KillTimer(nIDEvent);
			break;
		case DataRateTimer:
        // Is data acquisition timer - get a new data sample
			getData();
			break;
		case ChartUpdateTimer:
        // Is chart update timer - request chart update
			m_ChartViewer.updateViewPort(true, false);      
			break;
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CThicknessMeas_ProtoDlg::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	if(m_InstrHdl_PM100D != VI_NULL)
	{
		PM100D_close (m_InstrHdl_PM100D);
	}
	if(m_InstrHdl_CLD1015 != VI_NULL)
	{
		TL4000_close(m_InstrHdl_CLD1015);
	}
	if(m_ftHandle != NULL)
	{
		FT_Close(m_ftHandle);
	}

	CDialogEx::OnClose();
}

//////////////////////////////////////////////PM100D - Optical Power Meter///////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------
  Find Instruments
---------------------------------------------------------------------------*/
ViStatus CThicknessMeas_ProtoDlg::FindPM100D(ViString findPattern, ViChar **resource)
{
   ViStatus err;
   ViUInt32 deviceCount;
   static ViChar rsrcDescr[PM100D_BUFFER_SIZE];

   rsrcDescr[0] = '\0';
   *resource = rsrcDescr; //resourc -pointer to pointer

   err = PM100D_findRsrc (0, &deviceCount);
   switch(err)
   {
      case VI_SUCCESS:
         // At least one device was found. Nothing to do here. Continue with device selection menu.
         break;

      case VI_ERROR_RSRC_NFOUND:
		 DisplayLog(L"No matching instruments found");
         return (err);
                 
      default:
         return (err);
   }

   if(deviceCount <= 3) // deviceCount == 3 (?)
   {
		err = PM100D_getRsrcName(0, 0, rsrcDescr);
   }

   return (err);
}

/*===========================================================================
 GET ID
===========================================================================*/
ViStatus CThicknessMeas_ProtoDlg::Get_PM100D_Device_ID(ViSession handle)
{  
   ViStatus err;
   ViChar   nameBuf[PM100D_BUFFER_SIZE];
   ViChar   snBuf[PM100D_BUFFER_SIZE];
   ViChar   revBuf[PM100D_BUFFER_SIZE];
   ViChar   revBuf2[PM100D_BUFFER_SIZE];
   ViChar   revBuf3[PM100D_BUFFER_SIZE];
   CString  str;

   if((err = PM100D_identificationQuery (handle, VI_NULL, nameBuf, snBuf, revBuf))) return(err);
   str.Format(L"Instrument:    %S\n", nameBuf);
   DisplayLog(str);
   str.Format(L"Serial number: %S\n", snBuf);
   DisplayLog(str);
   str.Format(L"Firmware:      V%S\n", revBuf);
   DisplayLog(str);
   if((err = PM100D_revisionQuery (handle, revBuf2, VI_NULL))) return(err);
   str.Format(L"Driver:        V%S\n", revBuf2);
   DisplayLog(str);
   if((err = PM100D_getCalibrationMsg (handle, revBuf3))) return(err);
   str.Format(L"Cal message:   %S\n\n", revBuf3);
   DisplayLog(str);

   str.Format(L"%S SN:%S Firmware: V%S Driver: V%S Cal: %S",
	           nameBuf, snBuf, revBuf, revBuf2, revBuf3 );

   SetDlgItemText(IDC_ST_PM100D_INFO,str);

   return VI_SUCCESS;
}

void CThicknessMeas_ProtoDlg::OnBnClickedBtInitPm100d()
{
   ViStatus err;
   ViChar   *rscPtr;
   CString	strLog;

   err = FindPM100D(PM100D_FIND_PATTERN, &rscPtr);

   if(err != VI_SUCCESS)
   {
	   DisplayLog(L"something went wrong - find_instruments(PM100D_FIND_PATTERN, ...)");
	   return;
   }
   else
   {
		DisplayLog(L"find_instruments(PM100D_FIND_PATTERN, ...)");
   }

   err = PM100D_init(rscPtr, VI_OFF, VI_ON, &m_InstrHdl_PM100D);

   if(err != VI_SUCCESS)
   {
	   DisplayLog(L"[ERROR] Open session to PM100x instrument ...)");
	   return;
   }
   else
   {
	   DisplayLog(L"---------------PM100D init-------------------------");
	   Get_PM100D_Device_ID(m_InstrHdl_PM100D);
   }

   SetTimer(MESURE_POWER,1000,NULL);
}

void CThicknessMeas_ProtoDlg::MeasurePower()
{
	//Measure Power
   ViStatus	  err;
   ViReal64       power = 0.0;
   ViInt16        power_unit;
   char           *unit;
   CString		  strLog;

   err = PM100D_getPowerUnit(m_InstrHdl_PM100D, &power_unit);
//#define PM100D_POWER_UNIT_WATT   (0)
//#define PM100D_POWER_UNIT_DBM    (1) 

   switch(power_unit)
   {
      case PM100D_POWER_UNIT_DBM:   unit = "dBm";  break;
      default:                      unit = "W";    break;
   }

   if(err == VI_SUCCESS)
   {
	   err = PM100D_measPower(m_InstrHdl_PM100D, &power);
	   if(err == VI_SUCCESS)
	   {
		   
			strLog.Format(L"%15.9f %s", power*(10^9), unit);
			//DisplayLog(strLog);
			m_ctlSTpower.SetWindowTextW(strLog);
	   }
   }
}

void CThicknessMeas_ProtoDlg::OnBnClickedBtClosePm100d()
{
	ViStatus err = PM100D_close (m_InstrHdl_PM100D);
	 //if(!err)
	if(err == VI_SUCCESS)
	{
		KillTimer(MESURE_POWER);
		m_InstrHdl_PM100D = VI_NULL;
		DisplayLog(L"-------------------------PM100D ViSession Close-------------------------");
	}
}


//////////////////////////////////////////////CLD1015 - Laser Diode Controller////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------
  Find Instruments
---------------------------------------------------------------------------*/
ViStatus CThicknessMeas_ProtoDlg::FindCLD1015(ViString findPattern, ViChar **resource)
{
   ViStatus       err;
   ViSession      resMgr;
   ViFindList     findList;
   ViUInt32       findCnt;
   static ViChar  rscStr[VI_FIND_BUFLEN];

   //CString strLog = L"Scanning for instruments ...";

//   cout << "Scanning for instruments ...\n";

   if((err = viOpenDefaultRM(&resMgr))) return(err);
   switch((err = viFindRsrc(resMgr, findPattern, &findList, &findCnt, rscStr)))
   {
      case VI_SUCCESS:
         break;

      case VI_ERROR_RSRC_NFOUND:
         viClose(resMgr);
		 DisplayLog(L"No matching instruments found - FindCLD1015(...)");
         return (err);

      default:
         viClose(resMgr);
         return (err);
   }

   if(findCnt <= 3) // findCnt == 1 (?)
   {
      // Found only one matching instrument - return this
      *resource = rscStr;
   }
    viClose(findList);
    viClose(resMgr);
    return (VI_SUCCESS); 
//---------------------------------------------------------------------------------------------------------------------------------
}
/*---------------------------------------------------------------------------
 Read out device ID and print it to screen
---------------------------------------------------------------------------*/
ViStatus CThicknessMeas_ProtoDlg::Get_CLD1015_Device_ID(ViSession instrHdl)
{
   ViStatus err;
   ViChar   nameBuf[TL4000_BUFFER_SIZE];
   ViChar   snBuf[TL4000_BUFFER_SIZE];
   ViChar   fwRevBuf[TL4000_BUFFER_SIZE];
   ViChar   drvRevBuf[TL4000_BUFFER_SIZE];
   ViChar   calMsgBuf[TL4000_BUFFER_SIZE];

   CString str;

   err = TL4000_identificationQuery (instrHdl, VI_NULL, nameBuf, snBuf, fwRevBuf);
   if(err) return(err);
   

	str.Format(L"instrument: %S", nameBuf);
	DisplayLog(str);
	str.Format(L"serial no: %S", snBuf);
	DisplayLog(str);
	str.Format(L"Firmware: V%S", fwRevBuf);
	DisplayLog(str);

    err = TL4000_calibrationMessage (instrHdl, calMsgBuf);
   if(err) return(err);
	str.Format(L"Calibration: %S",calMsgBuf);
	DisplayLog(str);

   err = TL4000_revisionQuery (instrHdl, drvRevBuf, VI_NULL);
   if(err) return(err);
	str.Format(L"Driver: V%S", drvRevBuf);
	DisplayLog(str);

	str.Format(L"instrument: %S	\n serial no: %S \n Firmware: V%S \n Calibration: %S \n Driver: V%S",
				nameBuf, snBuf, fwRevBuf, calMsgBuf, drvRevBuf);
	SetDlgItemText(IDC_ST_CLD1015_INFO,str);

   return(VI_SUCCESS);
}
void CThicknessMeas_ProtoDlg::OnBnClickedBtInitCld1015()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
   ViStatus    err;
   ViChar      *rscPtr;

   err = FindCLD1015(TL4000_FIND_PATTERN_CLD, &rscPtr);
   //if(err) 
   if(err != VI_SUCCESS)
   {
	   DisplayLog(L"something went wrong - find_instruments(TL4000_FIND_PATTERN_CLD, ...)");
	   return;
   }
   else
   {
		DisplayLog(L"find_instruments(TL4000_FIND_PATTERN_CLD, ...)");
   }
   
   err = TL4000_init(rscPtr, VI_ON, VI_ON, &m_InstrHdl_CLD1015); // (Instrument Handle)
    
   if(err != VI_SUCCESS)
   {
	   DisplayLog(L"can not open session to instrument (CLD1015)");
	   return;
   }
   else if(err == VI_SUCCESS)
   {
	   DisplayLog(L"----------------CLD1015 Init---------------------");
   }


   // Get instrument info
   err = Get_CLD1015_Device_ID(m_InstrHdl_CLD1015);
   
   //if(err)
   if(err != VI_SUCCESS)
   {
	   DisplayLog(L"get_device_id err");
	   return;
   }

   //TEC,LASER state check
    ViBoolean  bON = false;
	err = TL4000_getTecOutputState(m_InstrHdl_CLD1015,&bON);

	//if(!err)
	if(err == VI_SUCCESS) //VI_SUCCESS 0L
	{
		m_bTEC = bON;
		if(m_bTEC == TRUE)
		{
			((CMFCButton*)GetDlgItem(IDC_MFCBT_TEC))->SetTextColor(RGB(255,0,0));
			GetDlgItem(IDC_MFCBT_TEC)->SetWindowTextW(L"TEC is ON");
		}
	}
	else 
	{
		return;
	}

	bON = false;
	err = TL4000_getLdOutputState(m_InstrHdl_CLD1015,&bON);

	//if(!err)
	if(err == VI_SUCCESS)
	{
		m_bLaser = bON;
		if(m_bLaser == TRUE)
		{
			((CMFCButton*)GetDlgItem(IDC_MFCBT_LASER))->SetTextColor(RGB(255,0,0));
			GetDlgItem(IDC_MFCBT_TEC)->SetWindowTextW(L"LASER is ON");
		}
	}
	else
	{
		return;
	}
}

void CThicknessMeas_ProtoDlg::OnBnClickedBtCloseCld1015()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if(m_bLaser == TRUE)
	 {
		OnBnClickedMfcbtLaser();
	 }
	 if(m_bTEC == TRUE)
	 {
		OnBnClickedMfcbtTec();
	 }

	 ViStatus err =TL4000_close (m_InstrHdl_CLD1015);
	 //if(!err)
	 if(err == VI_SUCCESS)
	 {
		m_InstrHdl_CLD1015 = VI_NULL;
		DisplayLog(L"-------------------------CLD1015 ViSession Close-------------------------");
	 }
}

void CThicknessMeas_ProtoDlg::OnBnClickedMfcbtTec()
{
	if(m_InstrHdl_CLD1015 == VI_NULL)
	{
		MessageBox(L"Remote Mode off. Initialize First",L"Notice",MB_OK|MB_ICONWARNING);
		return;
	}
	ViStatus    err;
	
	if(m_bTEC == TRUE)
	{
		if(m_bLaser == TRUE)
		{
			MessageBox(L"Turn-off Laser First",L"Notice",MB_OK|MB_ICONWARNING);
			return;
		}
		err = TL4000_switchTecOutput(m_InstrHdl_CLD1015,false);
		//if(!err)
		if(err == VI_SUCCESS)
		{
			((CMFCButton*)GetDlgItem(IDC_MFCBT_TEC))->SetTextColor(RGB(0,0,0));
			GetDlgItem(IDC_MFCBT_TEC)->SetWindowTextW(L"TEC is OFF");
			m_bTEC = FALSE;
		}
	}
	else
	{
		err = TL4000_switchTecOutput(m_InstrHdl_CLD1015,true);
		//if(!err)
		if(err == VI_SUCCESS)
		{
			((CMFCButton*)GetDlgItem(IDC_MFCBT_TEC))->SetTextColor(RGB(255,0,0));
			GetDlgItem(IDC_MFCBT_TEC)->SetWindowTextW(L"TEC is ON");
			m_bTEC = TRUE;
		}
	}
}

void CThicknessMeas_ProtoDlg::OnBnClickedMfcbtLaser()
{
	if(m_InstrHdl_CLD1015 == VI_NULL)
	{
		MessageBox(L"Remote Mode off. Initialize First",L"Notice",MB_OK|MB_ICONWARNING);
		return;
	}
	ViStatus    err;

	if(m_bLaser == TRUE)
	{
		err = TL4000_switchLdOutput(m_InstrHdl_CLD1015, false);
		
		if(err == VI_SUCCESS)
		{
			((CMFCButton*)GetDlgItem(IDC_MFCBT_LASER))->SetTextColor(RGB(0,0,0));
			GetDlgItem(IDC_MFCBT_LASER)->SetWindowTextW(L"LASER is OFF");
			m_bLaser = FALSE;
		}
	}
	else
	{
		if(m_bTEC == FALSE)
		{
			MessageBox(L"Turn-on TEC First",L"Notice",MB_OK|MB_ICONWARNING);
			return;
		}

		err = TL4000_switchLdOutput(m_InstrHdl_CLD1015, true);
		
		if(err == VI_SUCCESS)
		{
			((CMFCButton*)GetDlgItem(IDC_MFCBT_LASER))->SetTextColor(RGB(255,0,0));
			GetDlgItem(IDC_MFCBT_LASER)->SetWindowTextW(L"LASER is ON");
			m_bLaser = TRUE;
		}
	}
}

//////////////////////////////////////////////I-MON USB - Spectrometer////////////////////////////////////////////////////////
void CThicknessMeas_ProtoDlg::MBCS2Unicode(LPCSTR lpData,LPWSTR ReturnData)
{
	memset(ReturnData,0,(strlen(lpData)/2)+5);
	MultiByteToWideChar(949,0,lpData,-1,ReturnData,(int)strlen(lpData));
	return;
}

void CThicknessMeas_ProtoDlg::Unicode2MBCS(LPWSTR lpData,LPSTR lpRtd)
{
	memset(lpRtd,0x00,wcslen(lpData)*2+1);
	WideCharToMultiByte(949,0,lpData,-1,lpRtd,(int)wcslen(lpData)*2,NULL,NULL);
	return;
}

void CThicknessMeas_ProtoDlg::OnBnClickedBtOpen()
{
	FT_STATUS ftStatus;
	ftStatus = FT_Open(0,&m_ftHandle); 

	if (ftStatus == FT_OK) 
	{
		// FT_Open OK, use ftHandle to access device 
		DisplayLog(L"-------------------------I-MON open-------------------------");
		GetDlgItem(IDC_BT_OPEN)->EnableWindow(FALSE);
		CString strDeviceID = GetCommand(L"*IDN?");
		CString strFWver = GetCommand(L"*VERS?");
		CString str;
		str.Format(L"%s / %s", strDeviceID, strFWver);
		SetDlgItemText(IDC_ST_IMON_INFO,str);

	}
	else 
    { 
		// FT_Open failed
		DisplayLog(L" FT_Open failed");
	}
}

void CThicknessMeas_ProtoDlg::OnBnClickedBtClose()
{
	FT_STATUS ftStatus = FT_Close (m_ftHandle);
	if(ftStatus == FT_OK)
	{
		KillTimer(DUMMY_MEASURE);
		DisplayLog(L"-------------------------I-MON Close-------------------------");
		GetDlgItem(IDC_BT_OPEN)->EnableWindow(TRUE);
		m_ftHandle = NULL;
	}
	else
	{
		DisplayLog(L"FT_CLOSE() failed");
	}
}

void CThicknessMeas_ProtoDlg::OnBnClickedBtDummyMeas()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if(m_ftHandle == NULL)
	{
		MessageBox(L"Device not opened.", L"Warning", MB_OK | MB_ICONWARNING);
		return;
	}
	
	SetTimer(DUMMY_MEASURE,500,NULL);
	//DummyMeasure();
}

CString CThicknessMeas_ProtoDlg::DummyMeasure()
{
	CString strReturn = L"";
	CString strLog = L""; 
	CString strCmd = 
		L"*MEASure:DARKspectra 100 1 4"; //Run dark measurement, tint ≠ 0 output\r\noutput of data according to defined format
	
    m_strData = GetCommand(strCmd);

	drawChart(&m_ChartViewer);
	
	return strReturn;
}

void CThicknessMeas_ProtoDlg::OnBnClickedBtQuit()
{
	KillTimer(DUMMY_MEASURE);
	/*KillTimer(DataRateTimer);
	KillTimer(ChartUpdateTimer);*/
}

void CThicknessMeas_ProtoDlg::OnBnClickedBtWrCmd()
{
	if(m_ftHandle == NULL)
	{
		MessageBox(L"Device not opened.", L"Warning", MB_OK | MB_ICONWARNING);
		return;
	}

	CString strCmd = L"";
	
	GetDlgItemText(IDC_EDIT_GET_CMD,strCmd);
	if(strCmd.GetLength() > 0)
	{	
		GetCommand(strCmd);		
	}
	else
	{
		MessageBox(L"Input Command First.", L"Warning", MB_OK | MB_ICONWARNING);
		return;
	}
}

CString CThicknessMeas_ProtoDlg::GetCommand(CString strCmd)
{
	strCmd += L"\r";

	CString strLog = L"";
	CString strRet = L"";
	FT_STATUS ftStatus;
	DWORD BytesWritten; 
	char TxBuffer[256];// Contains data to write to device 

	memset(TxBuffer,0x00,sizeof(TxBuffer));

	Unicode2MBCS(strCmd.GetBuffer(0), TxBuffer);
	
	ftStatus = FT_Write(m_ftHandle, TxBuffer, sizeof(TxBuffer), &BytesWritten); 
	if (ftStatus == FT_OK) 
	{ 
		// FT_Write OK 
		strLog.Format(L"FT_Write FT_OK - %s",strCmd);
		DisplayLog(strLog);
	} 
	else 
	{ 
		strLog.Format(L"FT_Write Failed - %s",strCmd);
		DisplayLog(strLog);
	}

	DWORD RxBytes = 1024*10;
	DWORD BytesReceived; 

	char RxBuffer[1024*10];
	::ZeroMemory(RxBuffer, sizeof(RxBuffer));

	//TCHAR TRxBuffer[256*2];
	TCHAR TRxBuffer[1024*20];
	::ZeroMemory(TRxBuffer, sizeof(TRxBuffer));
	
	//FT_GetStatus(m_ftHandle,&RxBytes,&TxBytes,&EventDWord);
	double nElapse = 0.2*1000; //read with a timeout of 0.2 seconds
	FT_SetTimeouts(m_ftHandle,(ULONG)nElapse,0); 
	ftStatus = FT_Read(m_ftHandle,RxBuffer,RxBytes,&BytesReceived); 
	if (ftStatus == FT_OK) 
	{ 
		//if (BytesReceived == RxBytes) 
		if (BytesReceived > 0) 
		{ 
			// FT_Read OK 
			DisplayLog(L"FT_Read OK ");

			MBCS2Unicode(RxBuffer,TRxBuffer);
			strLog.Format(L"%s",TRxBuffer);
			strRet.Format(L"%s",TRxBuffer);
			//DisplayLog(strLog);
			SetDlgItemText(IDC_EDIT_MEAS,strRet);
		} 
		else 
		{ 
			// FT_Read Failed 
			DisplayLog(L"FT_Read Failed ");
		} 
	} 
	else
	{
		//AfxMessageBox(L"FT_Read Timeout");
		DisplayLog(L"FT_Read Timeout");
	}
	return strRet;
}
//////////////////////////Chart
// View port changed event
void CThicknessMeas_ProtoDlg::OnViewPortChanged()
{
    drawChart(&m_ChartViewer);
}

// A utility to shift a new data value into a data array
//
static void shiftData(double *data, int len, double newValue)
{
    memmove(data, data + 1, sizeof(*data) * (len - 1));
    data[len - 1] = newValue;
}

//
// The data acquisition routine. In this demo, this is invoked every 250ms.
//
void CThicknessMeas_ProtoDlg::getData()
{
    // The current time in millisecond resolution
    SYSTEMTIME st;
    GetLocalTime(&st);
    double now = Chart::chartTime(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, 
        st.wSecond) + st.wMilliseconds / 1000.0;
    
    // This is our formula for the random number generator
    do
    {
        // Get a data sample
        double p = m_nextDataTime * 4;
        double dataA = 20 + cos(p * 129241) * 10 + 1 / (cos(p) * cos(p) + 0.01);
        /*double dataB = 150 + 100 * sin(p / 27.7) * sin(p / 10.1);
        double dataC = 150 + 100 * cos(p / 6.7) * cos(p / 11.9);*/

        // Shift the values into the arrays
        shiftData(m_dataSeriesA, sampleSize, dataA);
       /* shiftData(m_dataSeriesB, sampleSize, dataB);
        shiftData(m_dataSeriesC, sampleSize, dataC);*/
        shiftData(m_timeStamps, sampleSize, m_nextDataTime);

        m_nextDataTime += DataInterval / 1000.0;
    }
    while (m_nextDataTime < now);

    //
    // We provide some visual feedback to the latest numbers generated, so you can see the
    // data being generated.
    //
//    char buffer[1024];
    
    //sprintf(buffer, "%.2f", m_dataSeriesA[sampleSize - 1]);
	//sprintf_s(buffer, "%.2f", m_dataSeriesA[sampleSize - 1]);


    //m_ValueA.SetWindowText(CString(buffer));

   /* sprintf(buffer, "%.2f", m_dataSeriesB[sampleSize - 1]);
    m_ValueB.SetWindowText(CString(buffer));

    sprintf(buffer, "%.2f", m_dataSeriesC[sampleSize - 1]);
    m_ValueC.SetWindowText(CString(buffer));*/
}

//
// Draw the chart and display it in the given viewer
//
void CThicknessMeas_ProtoDlg::drawChart(CChartViewer *viewer)
{
	m_nTotalScan++;

	int	nCnt = 0;
	int nIdx1 = 0;
	int nIdx2 = 0;
	do
	{
	   nIdx2 = m_strData.Find(L"\r",nIdx1);
	   if(nIdx2>0)
	   {
		   m_Data[nCnt] = _wtof(m_strData.Mid(nIdx1,(nIdx2-nIdx1)));
		   nIdx1 = nIdx2+1;
		   nCnt++;
	   }
	}
	while(nIdx2 != -1);

	if(nCnt != 513)
	{
		m_nNGcount++;
	}
	
    XYChart *c = new XYChart(600, 400, 0xf4f4f4, 0x000000, 1);
    c->setRoundedFrame(m_extBgColor);
    
    // Set the plotarea at (55, 62) and of size 320 x 175 pixels. Use white (ffffff) 
    // background. Enable both horizontal and vertical grids by setting their colors to 
    // grey (cccccc). Set clipping mode to clip the data lines to the plot area.
    c->setPlotArea(55, 0, 500, 350, 0xffffff, -1, -1, 0xcccccc, 0xcccccc);
    c->setClipping();

	// Add a title to the chart using 15 pts Times New Roman Bold Italic font, with a light
    // grey (dddddd) background, black (000000) border, and a glass like raised effect.
    c->addTitle("Dummy Test (*MEASure:DARKspectra 100 1 4)", "timesbi.ttf", 15
        )->setBackground(0xdddddd, 0x000000, Chart::glassEffect());
   
    LegendBox *b = c->addLegend2(55, 33, 3, "arialbd.ttf", 9);
    b->setBackground(Chart::Transparent, Chart::Transparent);
    b->setWidth(520);

	// Add a title to the x axis
    c->xAxis()->setTitle("?");
    // Configure the y-axis with a 10pts Arial Bold axis title
    c->yAxis()->setTitle("?", "arialbd.ttf", 10);

	// Configure the x-axis to auto-scale with at least 75 pixels between major tick and 
    // 15  pixels between minor ticks. This shows more minor grid lines on the chart.
    c->xAxis()->setTickDensity(75, 15);

    // Set the axes width to 2 pixels
    c->xAxis()->setWidth(2);
    c->yAxis()->setWidth(2);

	c->addAreaLayer(DoubleArray(m_Data, (int)(sizeof(m_Data) / sizeof(m_Data[0]))),
        0x80ff0000, "", 3);

	m_ChartViewer.setChart(c);  //m_chartView에 Chart를 보여주기 위한 코드

	ZeroMemory(m_Data,sizeof(m_Data));
	delete c;
	c = NULL;

	SetDlgItemInt(IDC_EDIT_COUNT,m_nTotalScan);
	SetDlgItemInt(IDC_EDIT_NG,m_nNGcount);
	
}

/////////////////////////////////////////////////////////////////////////////
// General utilities

//
// Get the default background color
//
int CThicknessMeas_ProtoDlg::getDefaultBgColor()
{
    LOGBRUSH LogBrush; 
    HBRUSH hBrush = (HBRUSH)SendMessage(WM_CTLCOLORDLG, (WPARAM)CClientDC(this).m_hDC, 
        (LPARAM)m_hWnd); 
    ::GetObject(hBrush, sizeof(LOGBRUSH), &LogBrush); 
    int ret = LogBrush.lbColor;
    return ((ret & 0xff) << 16) | (ret & 0xff00) | ((ret & 0xff0000) >> 16);
}

