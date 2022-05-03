#pragma once

/**
    Camera.h
    Purpose: Camera class header file. Handles login / logout from camera and get stream data from HIKVision Camera

    @author Gulraiz Iqbal
    @version 1.0 22/10/2015
*/
#include <stdio.h>
#include <iostream>


#if (_WIN32_WINNT < 0x0500) // This switch is needed to make the program compile
#undef _WIN32_WINNT	    // because GetConsoleWindow needs it. See Documentation
#define _WIN32_WINNT 0x0500 // for GetConsoleWindow in MSDN.
#endif

#include <Windows.h>
#include "HCNetSDK.h"
#include <time.h>
#include "plaympeg4.h"
#include <opencv2/opencv.hpp>


using namespace std;
using namespace cv;

class Camera
{
public:
	char* ip; WORD port; char* username; char* password;

	//Camera Constructor
	Camera(void);
	
	//Set login details and set in global variables
	void setLoginDetails(char * ip, WORD port, char * username, char * password);
	
	//Connect with camera using login details
	void login();
	
	
	//Get stream after decode
	void GetPreview();

	LONG getlRealPlayHandle();

	//logout from camera
	void logout();
	
	//Get Mat frame from camera
	int  Convertyv12toBGR();

	//set custom path to store video
	void setVideoPath(char path);

	//set custom path to store images
	void setImagePath(char path);

	//Write video 
	int WriteAVIvideo();

	//PTz Controls functions
	bool Ptz_ZoomIn(int time, bool isFast);
	bool Ptz_ZoomOut(int time, bool isFast);

	bool Ptz_FOCUS_NEAR(int time, bool isFast);
	bool Ptz_FOCUS_FAR(int time, bool isFast);

	bool Ptz_TILT_UP(int time, bool isFast);
	bool Ptz_TILT_DOWN(int time, bool isFast);

	bool Ptz_PAN_LEFT(int time, bool isFast);
	bool Ptz_PAN_RIGHT(int time, bool isFast);

	bool Ptz_UP_LEFT(int time, bool isFast);
	bool Ptz_UP_RIGHT(int time, bool isFast);

	bool Ptz_DOWN_LEFT(int time, bool isFast);
	bool Ptz_DOWN_RIGHT(int time, bool isFast);

	bool Ptz_PAN_AUTO(int time, bool isFast);

	//Camera destructor
	~Camera(void);

private:

	//Login user and device details
	LONG lUserID;
	NET_DVR_DEVICEINFO_V30 struDeviceInfo;
	
	//variables to handle PTz commands
	int v_ZoomIn;
	int v_ZoomOut;
	int v_FOCUS_NEAR;
	int v_FOCUS_FAR;
	int v_TILT_UP;
	int v_TILT_DOWN;
	int v_PAN_LEFT;
	int v_PAN_RIGHT;
	int v_UP_LEFT;
	int v_UP_RIGHT;
	int v_DOWN_LEFT;
	int v_DOWN_RIGHT;
	int v_PAN_AUTO;
	long lRealPlayHandle;

	
	//Callback function to get data and decode callback function
	static void CALLBACK g_DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2);
	static void CALLBACK g_RealDataCallBack_V30(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer,DWORD dwBufSize,void* dwUser);
	static void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser);

	static void CALLBACK DisplayCBFun(long nPort, char *pBuf, long nSize, long nWidth, long nHeight, long nStamp, long nType, long nReceved);
	//convert image from PY12 to RGB
	static void yv12ToRgb( uchar *pBuffer, const int w, const int h );

};

