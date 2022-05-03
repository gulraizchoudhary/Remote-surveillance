/**
    Camera.cpp
    Purpose: Login to camera and get stream data from HIKVision Camera

    @author Gulraiz Iqbal
    @version 1.0 22/10/2015
*/


#include "Camera.h"



uchar* pixels =NULL;
LONG h,w;
Mat result(1080, 1920, CV_8UC3);
LONG lPort;
HWND hWnd=GetConsoleWindow();

//Set Camera Login details
void Camera::setLoginDetails(char * ip, WORD port, char * username, char * password){
	this->ip= ip;
	this->port=port;
	this->username=username;
	this->password=password;
}

//Login into camera
void Camera::login(){

	//Initialize SDK
	NET_DVR_Init();

	//Set connection time and reconnection time
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);

	//Login on given location
	lUserID = NET_DVR_Login_V30(this->ip, this->port, this->username, this->password, &struDeviceInfo);
	if (lUserID < 0)
	{
		printf("Login error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Cleanup();
	}

	//---------------------------------------
	//Set exception callback function
	NET_DVR_SetExceptionCallBack_V30(0, NULL,&g_ExceptionCallBack, NULL);

	//---------------------------------------
	//Start preview and set to callback stream data

	NET_DVR_PREVIEWINFO struPlayInfo = {0};
	struPlayInfo .hPlayWnd =   NULL;      //If need to decode, please set it valid. If want to get stream data only, it can be set to NULL
	struPlayInfo .lChannel     = 1;       //Preview channel NO.
	struPlayInfo .dwStreamType = 0;       //0-main stream, 1-sub stream, 2-stream3, 3-stream4.
	struPlayInfo.dwLinkMode = 4;         //0-TCP mode, 1-UDP mode, 2-Multi-play mode, 3-RTP mode, 4-RTP/RTSP, 5-RTSP/HTTP

	// PlayM4_SetDisplayBuf(lPort,10);
	this->lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, &g_RealDataCallBack_V30, 0);

}

//GetRealPlayHandle value  return by NET_DVR_RealPlay_V40
LONG Camera::getlRealPlayHandle(){
	return lRealPlayHandle;
}



//prepare and start preview of stream data
void Camera::GetPreview(){

	while(waitKey(1) != 27){
		Convertyv12toBGR();
		imshow("video capture",result);
	}
}


int Camera::WriteAVIvideo(){
	
	if(!result.empty()){
		VideoWriter oVideoWriter ("MyVideo.avi",CV_FOURCC('D','I','V','X'),10,Size(1920,1080),true);

   if ( !oVideoWriter.isOpened() ) //if not initialize the VideoWriter successfully, exit the program
   {
        cout << "ERROR: Failed to write the video" << endl;
		return -1;
   }
   int count=0;
   while(waitKey(1) != 27){
	   
	   if(Convertyv12toBGR()){
			imshow("video capture",result);
			
			if(count>14)
				oVideoWriter.write(result); //writer the frame into the file
	   }
	   count++;
   }
   
   oVideoWriter.release();
}
	return -1;
}

//Get Mat object 
int Camera::Convertyv12toBGR(){
	if(sizeof(pixels)==4){
		yv12ToRgb(pixels,w,h);
		return 1;
	}
	return 0;
}


//Logout from camera
void Camera::logout(){
	if (lRealPlayHandle < 0)
	{
		printf("NET_DVR_RealPlay_V30 error\n");
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
	}

	//---------------------------------------
	//Close preview
	NET_DVR_StopRealPlay(lRealPlayHandle);
	//Logout
	NET_DVR_Logout_V30(lUserID);
	NET_DVR_Cleanup();
}


//Convert yv12 colorspace to BGR colorspace 
void Camera::yv12ToRgb( uchar *pBuffer, const int w, const int h )
{

#define clip_8_bit(val)       \
	{                       \
	if( val < 0 )        \
	val = 0;          \
				else if( val > 255 ) \
				val = 255;        \
}


	long ySize=w*h;
	long uSize;
	uSize=ySize>>2;

	uchar *output=result.data;
	uchar *pY=pBuffer;
	uchar *pU=pY+ySize;
	uchar *pV=pU+uSize;

	int y, u, v;
	int r, g, b;    

	int sub_i_uv;
	int sub_j_uv;

	const int uv_width  = w / 2;
	const int uv_height = h / 2;

	for( int i = 0; i < h; ++i ) {
		// calculate u & v rows
		sub_i_uv = ((i * uv_height) / h);

		for( int j = 0; j < w; ++j ) {
			// calculate u & v columns
			sub_j_uv = (j * uv_width) / w;

			y = pY[(i * w) + j] - 16;
			u = pU[(sub_i_uv * uv_width) + sub_j_uv] - 128;
			v = pV[(sub_i_uv * uv_width) + sub_j_uv] - 128;


			r = (int)((1.1644 * y) + (1.5960 * v));
			g = (int)((1.1644 * y) - (0.3918 * u) - (0.8130 * v));
			b = (int)((1.1644 * y) + (2.0172 * u));


			clip_8_bit( b );
			clip_8_bit( g );
			clip_8_bit( r );

			*output++=r;
			*output++=g;
			*output++=b;
		}
	}

}


//Decode compressed data to YV12 format
void CALLBACK Camera::g_DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)  
{  

	if ( pFrameInfo->nType == T_YV12 ) 
	{
		pixels = (uchar*)pBuf;
		h=pFrameInfo->nHeight;
		w= pFrameInfo->nWidth;
		/*
		printf("nPort=%d,nSize=%d,pFrameInfo.nWidth=%ld,pFrameInfo.nHeight=%ld,\n pFrameInfo.nStamp=%ld,pFrameInfo.nType=%ld,pFrameInfo.nFrameRate=%ld.\n",   
		nPort,nSize,pFrameInfo->nWidth,pFrameInfo->nHeight, pFrameInfo->nStamp,pFrameInfo->nType,pFrameInfo->nFrameRate); 
		*/

	}
} 


//Get real stream data
void CALLBACK Camera::g_RealDataCallBack_V30(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer,DWORD dwBufSize,void* dwUser)
{

	switch (dwDataType)
	{
	case NET_DVR_SYSHEAD: //System head

		if (!PlayM4_GetPort(&lPort))  //Get unused port
		{
			break;
		}
		//m_iPort = lPort; //The data called back at the first time is system header. Please assign this port to global port, and it will be used to play in next callback
		if (dwBufSize > 0)
		{
			if (!PlayM4_SetStreamOpenMode(lPort, STREAME_REALTIME))  //Set real-time stream playing mode
			{
				break;
			}

			if (!PlayM4_OpenStream(lPort, pBuffer, dwBufSize, 1024*1024)) //Open stream
			{
				break;
			}

			if(!PlayM4_SetDecCallBack(lPort,g_DecCBFun))  
			{  
				break;  
			}

			if (!PlayM4_Play(lPort, hWnd)) //Start play
			{
				break;
			}
		}

	case NET_DVR_STREAMDATA:   //Stream data
		
		if (dwBufSize > 0 && lPort != -1)
		{
			if (!PlayM4_InputData(lPort, pBuffer, dwBufSize))
			{
				break;
			}
		}
		

	}
	
}


//Callback function to handl exception
void CALLBACK Camera::g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
	char tempbuf[256] = {0};
	switch(dwType)
	{
	case EXCEPTION_RECONNECT:    //reconnect when preview
		printf("----------reconnect--------%d\n", time(NULL));
		break;
	default:
		break;
	}
}
bool Camera::Ptz_ZoomIn(int time, bool isFast){
	if(isFast && NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_ZoomIn,0,7)){
		Sleep(time);
		return NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_ZoomIn,1,7);
	}
	else if(!isFast && NET_DVR_PTZControl(this->lRealPlayHandle,this->v_ZoomIn,0))
	{
			Sleep(time);
		return NET_DVR_PTZControl(this->lRealPlayHandle,this->v_ZoomIn,1);
	}
	return false;
}


bool Camera::Ptz_ZoomOut(int time, bool isFast){
	if(isFast && NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_ZoomOut,0,7)){
		Sleep(time);
		return NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_ZoomOut,1,7);
	}
	else if(!isFast && NET_DVR_PTZControl(this->lRealPlayHandle,this->v_ZoomOut,0)){

	Sleep(time);
	return NET_DVR_PTZControl(this->lRealPlayHandle,this->v_ZoomOut,1);
	}

	return false;
}

bool Camera::Ptz_FOCUS_NEAR(int time, bool isFast){
	if(isFast && NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_FOCUS_NEAR,0,7)){
	
		Sleep(time);
		return NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_FOCUS_NEAR,1,7);
	}
	else if(!isFast && NET_DVR_PTZControl(this->lRealPlayHandle,this->v_FOCUS_NEAR,0)){

	Sleep(time);
	return NET_DVR_PTZControl(this->lRealPlayHandle,this->v_FOCUS_NEAR,1);
	}
	return false;
}


bool Camera::Ptz_FOCUS_FAR(int time, bool isFast){

	if(isFast && NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_FOCUS_FAR,0,7)){
	
		Sleep(time);
		return NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_FOCUS_FAR,1,7);
	}

	else if(!isFast && 	NET_DVR_PTZControl(this->lRealPlayHandle,this->v_FOCUS_FAR,0)){

	Sleep(time);
	return NET_DVR_PTZControl(this->lRealPlayHandle,this->v_FOCUS_FAR,1);
	}
	return false;
}

bool Camera::Ptz_TILT_UP(int time, bool isFast){

	if(isFast && NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_TILT_UP,0,7)){
	
		Sleep(time);
		return NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_TILT_UP,1,7);
	}
	else if(!isFast && NET_DVR_PTZControl(this->lRealPlayHandle,this->v_TILT_UP,0)){
	Sleep(time);
	return NET_DVR_PTZControl(this->lRealPlayHandle,this->v_TILT_UP,1);
	}
	return false;
}


bool Camera::Ptz_TILT_DOWN(int time, bool isFast){
	if(isFast && NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_TILT_DOWN,0,7)){
	
		Sleep(time);
		return NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_TILT_DOWN,1,7);
	}

	else if(!isFast && 	NET_DVR_PTZControl(this->lRealPlayHandle,this->v_TILT_DOWN,0)){

	Sleep(time);
	return NET_DVR_PTZControl(this->lRealPlayHandle,this->v_TILT_DOWN,1);
	}
	return false;
}

bool Camera::Ptz_PAN_LEFT(int time, bool isFast){
	if(isFast && NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_PAN_LEFT,0,7)){
	
		Sleep(time);
		return NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_PAN_LEFT,1,7);
	}
	else if(!isFast && 	NET_DVR_PTZControl(this->lRealPlayHandle,this->v_PAN_LEFT,0)){

	Sleep(time);
	return NET_DVR_PTZControl(this->lRealPlayHandle,this->v_PAN_LEFT,1);
	}
	return false;
}
bool Camera::Ptz_PAN_RIGHT(int time, bool isFast){

	if(isFast && NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_PAN_RIGHT,0,7)){
	
		Sleep(time);
		return NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_PAN_RIGHT,1,7);
	}

	else if(!isFast && NET_DVR_PTZControl(this->lRealPlayHandle,this->v_PAN_RIGHT,0)){

	Sleep(time);
	return NET_DVR_PTZControl(this->lRealPlayHandle,this->v_PAN_RIGHT,1);
	}
	return false;
}


bool Camera::Ptz_UP_LEFT(int time, bool isFast){
	if(isFast && NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_UP_LEFT,0,7)){
	
		Sleep(time);
		return NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_UP_LEFT,1,7);
	}
	
	else if(!isFast && 	NET_DVR_PTZControl(this->lRealPlayHandle,this->v_UP_LEFT,0)){

	Sleep(time);
	return NET_DVR_PTZControl(this->lRealPlayHandle,this->v_UP_LEFT,1);
	}
	return false;
}
bool Camera::Ptz_UP_RIGHT(int time, bool isFast){
	if(isFast && NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_UP_RIGHT,0,7)){
	
		Sleep(time);
		return NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_UP_RIGHT,1,7);
	}

	else if(!isFast && 	NET_DVR_PTZControl(this->lRealPlayHandle,this->v_UP_RIGHT,0)){

	Sleep(time);
	return NET_DVR_PTZControl(this->lRealPlayHandle,this->v_UP_RIGHT,1);
	}
	return false;
}

bool Camera::Ptz_DOWN_LEFT(int time, bool isFast){
	if(isFast && NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_DOWN_LEFT,0,7)){
	
		Sleep(time);
		return NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_DOWN_LEFT,1,7);
	}
	else if(!isFast && 	NET_DVR_PTZControl(this->lRealPlayHandle,this->v_DOWN_LEFT,0)){

	Sleep(time);
	NET_DVR_PTZControl(this->lRealPlayHandle,this->v_DOWN_LEFT,1);
	}

	return false;
}


bool Camera::Ptz_DOWN_RIGHT(int time, bool isFast){
	
	if(isFast && NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_DOWN_RIGHT,0,7)){
	
		Sleep(time);
		return NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_DOWN_RIGHT,1,7);
	}
	
	else if (!isFast && NET_DVR_PTZControl(this->lRealPlayHandle,this->v_DOWN_RIGHT,0)){

	Sleep(time);
	return NET_DVR_PTZControl(this->lRealPlayHandle,this->v_DOWN_RIGHT,1);
	}
	return false;
}

bool Camera::Ptz_PAN_AUTO(int time, bool isFast){
	if(isFast && NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_PAN_AUTO,0,7)){
	
		Sleep(time);
		return NET_DVR_PTZControlWithSpeed(this->lRealPlayHandle,this->v_PAN_AUTO,1,7);
	}
	else if(!isFast && 	NET_DVR_PTZControl(this->lRealPlayHandle,this->v_PAN_AUTO,0)){

	Sleep(time);
	return NET_DVR_PTZControl(this->lRealPlayHandle,this->v_PAN_AUTO,1);
	}

	return false;
}


Camera::Camera(void){
	//Set PTz commands
	this->v_ZoomIn=11;
	this->v_ZoomOut=12;
	this->v_FOCUS_NEAR=13;
	this->v_FOCUS_FAR=14;
	this->v_TILT_UP=21;
	this->v_TILT_DOWN=22;
	this->v_PAN_LEFT=23;
	this->v_PAN_RIGHT=24;
	this->v_UP_LEFT= 25;
	this->v_UP_RIGHT= 26;
	this->v_DOWN_LEFT=27;
	this->v_DOWN_RIGHT=28;
	this->v_PAN_AUTO=29;
}


Camera::~Camera(void)
{
}
