
// ThicknessMeas_ProtoDlg.h : 헤더 파일
//

#pragma once

#include "visa.h"
#include "PM100D.h" //Optical Power Meter - THORLABS PM100D
#include "TL4000.h" //Laser Diode Controller - THORLABS CLD1015
#include "ftd2xx.h" //Spectrometer (- USB driver) - Ibsen I-MON USB
#include "afxwin.h"
#include "ChartViewer.h"
#include <afxmt.h> //동기화

const int sampleSize = 512;

// CThicknessMeas_ProtoDlg 대화 상자
class CThicknessMeas_ProtoDlg : public CDialogEx
{
// 생성입니다.
public:
	CThicknessMeas_ProtoDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_THICKNESSMEAS_PROTO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnViewPortChanged();
	DECLARE_MESSAGE_MAP()

	//PM100D - Optical Power Meter
	ViSession   m_InstrHdl_PM100D;
	ViStatus	FindPM100D(ViString findPattern, ViChar **resource);
	ViStatus	Get_PM100D_Device_ID(ViSession handle);
	void		MeasurePower();

	//CLD1015 - Laser Diode Controller
	ViSession   m_InstrHdl_CLD1015;
	BOOL		m_bTEC, m_bLaser;
	ViStatus	FindCLD1015(ViString findPattern, ViChar **resource);
	ViStatus	Get_CLD1015_Device_ID(ViSession handle);
	
	//I-MON USB - Spectrometer
	void MBCS2Unicode(LPCSTR lpData,LPWSTR ReturnData);
	void Unicode2MBCS(LPWSTR lpData,LPSTR lpRtd);

	FT_HANDLE m_ftHandle;
	int		  m_nTotalScan,m_nIMON_USB_Recon_Cnt,m_nFFT_DoubleFault;
	CString	  DummyMeasure();
	CString   WriteFwCommand(CString strCmd);
	CString   WriteFwCommand2(CString strCmd);
	void	  IMON_Reconnect();
	
	void      GetMeasConfig();
	void	  GetWavRange();
	void	  SetWavRange(int wbeg, int wend, int wstep);
	void	  FFTtest();
	double    WavLenCalib(int pix); // Wavelength Calibration - 파장교정
	//double    WavLenFit(double pix); //wavelength fit (?)
	double	  FixTemperDrift(double dLamda, double dTemperature); // Compensation for Temperature Drift - 온도편차보정
	double	  MeasureTemperature();

	//chart
	CChartViewer m_ChartViewer;
	CString		 m_strData,m_strCmd,m_strChartTitle;
	double		 m_nDataArray[512];
	double		 m_nDataArrTemp[512];
	double		 m_nXDataArray[512], m_nYDataArray[512];
	char		 m_ChTitle;

	void getData();
	void DrawChartFormat4(CChartViewer *viewer); //format4 (ASCII output, separated by <CR>)
	void DrawChartFormat7(CChartViewer *viewer); //format7 (ASCII output with wavelength, separated by <CR>)
	
private:
    CFont m_Font;
	double m_timeStamps[sampleSize];	// The timestamps for the data series
	double m_dataSeriesA[sampleSize];	// The values for the data series A
	int m_extBgColor;		// The default background color.
	double m_nextDataTime;  // Used by the random number generator to generate real time data.
	// utility to get default background color
	int getDefaultBgColor();
	void DrawFFTChart(CChartViewer *viewer);

public:
	afx_msg void OnBnClickedBtInitPm100d();
	afx_msg void OnBnClickedBtClosePm100d();
	afx_msg void OnBnClickedBtInitCld1015();
	afx_msg void OnBnClickedBtCloseCld1015();
	afx_msg void OnBnClickedBtOpen();
	afx_msg void OnBnClickedBtClose();
	CListBox m_ctlListLog;
	void DisplayLog(CString str);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CStatic m_ctlSTpower;
	afx_msg void OnBnClickedMfcbtTec();
	afx_msg void OnBnClickedMfcbtLaser();
	afx_msg void OnClose();
	afx_msg void OnBnClickedBtDummyMeas();
	afx_msg void OnBnClickedBtQuit();
	afx_msg void OnBnClickedBtWrCmd();
	afx_msg void OnBnClickedBtSetWavRange();
	CString m_strMeasure;
	afx_msg void OnBnClickedBtDrawChart();
	afx_msg void OnBnClickedBtFft();
	afx_msg void OnBnClickedBtFftStop();
};
