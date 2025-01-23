/*--------------------------------------------------------------------------------*/
/* VBAN-TEXT Client minimal source code example                                   */
/*--------------------------------------------------------------------------------*/
/* WIN32 'C' Sample Code to send and recieve VBAN-TEXT request    V.Burel (c)2025 */
/*                                                                                */
/*  THIS PROGRAM HAS BEEN MADE TO CONNECT WITH MATRIX VERSION X.0.1.2 min         */
/*                                                                                */
/*  This program example shows                                                    */
/*  - How to use a single socket to send request / rcv response                   */
/*  - How to use Windows Edit control                                             */
/*  - How to use simple Windows Timer.                                            */
/*                                                                                */
/*--------------------------------------------------------------------------------*/
/*                                                                                */
/*  COMPILATION DIRECTIVES:                                                       */
/*                                                                                */
/*  To compile With Microsoft VC2005 or higher, you need to create an             */
/*  empty Win32 project with the following options:                               */
/*  - Configuration Properties / General : Char Set = NOT SET                     */
/*  - Configuration Properties / C/C++ / Preprocessor : _CRT_SECURE_NO_DEPRECATE  */
/*                                                                                */
/*  LINKER: Ws2_32.lib                                                            */
/*                                                                                */
/*  This source code can be compiled for 32bit or 64 bits targets as well         */
/*                                                                                */
/*--------------------------------------------------------------------------------*/
/*                                                                                */
/*  LICENSING: This source code can be used in any application or project         */
/*             connected to VB-Audio software applications                        */
/*                                                                                */
/*--------------------------------------------------------------------------------*/
#ifndef __cplusplus
	#ifndef STRICT
		#define STRICT
	#endif
#endif

#include <windows.h>
#include <stdio.h>
#include <winsock.h>

#include "vbantxt_client.h"

//
//define globals 
//

typedef struct tagAPP_CONTEXT
{
	HWND		hwnd_MainWindow;
	HINSTANCE	hinstance;

	HWND		hw_serveraddress;
	HWND		hw_port_out;
	HWND		hw_button_send;
	HWND		hw_CLI;
	HWND		hw_Log;

	HFONT		font1;
	UINT		winTimer;

	char *		pLogBuffer;
	long		pLogBuffer_nbByte;
	long		pLogBuffer_curs;
	BOOL		pLogBuffer_fDirty;
	BOOL		pLogBuffer_fFull;
} T_APP_CONTEXT, *PT_APP_CONTEXT, *LPT_APP_CONTEXT;

static T_APP_CONTEXT G_context={NULL, NULL};





/*******************************************************************************/
/**                                 LOG FUNCTIONS                             **/
/*******************************************************************************/

long LogBuffer_AddText(LPT_APP_CONTEXT lpapp, char * sss, long FrameNumber, BOOL fRCV)
{
	char  szeol[8]={13,10,0};
	char  ss1[64];
	int lll,ll2;
	char  *lpc;
	if (lpapp->pLogBuffer == NULL) return -1;

	lll=(long)strlen(sss);
	if (lll > (lpapp->pLogBuffer_nbByte - lpapp->pLogBuffer_curs)) 
	{
		if (lpapp->pLogBuffer_fFull == FALSE)
		{
			lpc=lpapp->pLogBuffer + lpapp->pLogBuffer_curs;
			sprintf(lpc, "****** Not Enough Memory in TXT Buffer (%i char / %i max)*******",(int)lpapp->pLogBuffer_curs, (int)lpapp->pLogBuffer_nbByte);
			lpapp->pLogBuffer_fFull=TRUE;
			lpapp->pLogBuffer_fDirty=TRUE;
		}
		return -1;
	}

	lpc=lpapp->pLogBuffer + lpapp->pLogBuffer_curs;
	ll2=0;
	if (fRCV == TRUE) sprintf(ss1, "RCV#%04i ",FrameNumber);
	else sprintf(ss1, "TRX#%04i ",FrameNumber);
	
	strcpy(lpc,ss1);
	ll2=(long)strlen(ss1);
	lpc=lpc+ll2;
	
	strcpy(lpc,sss);
	lpc=lpc+lll;
	lpapp->pLogBuffer_curs=lpapp->pLogBuffer_curs + ll2+lll;
	
	if ((lpc[-1] != 10) && (lpc[-2] != 13))
	{
		strcpy(lpc, szeol);
		lpc=lpc+2;
		lpapp->pLogBuffer_curs+=2;
	}
	*lpc=0;
	
	lpapp->pLogBuffer_fDirty=TRUE;
	return 0;
}



/*******************************************************************************/
/**                                VBAN FUNCTIONS                             **/
/*******************************************************************************/

#pragma pack(1)

struct tagVBAN_HEADER 
{
	unsigned long vban;			// contains 'V' 'B', 'A', 'N'
	unsigned char format_SR;	// SR index (see SRList above) 
	unsigned char format_nbs;	// service function 
	unsigned char format_nbc;	// service id
	unsigned char format_bit;	// mask = 0x07 (see DATATYPE table below)
	char streamname[16];		// stream name
	unsigned long nuFrame;		// growing frame number
};

#pragma pack()

typedef struct tagVBAN_HEADER T_VBAN_HEADER;
typedef struct tagVBAN_HEADER *PT_VBAN_HEADER;
typedef struct tagVBAN_HEADER *LPT_VBAN_HEADER;

#define VBAN_PROTOCOL_MASK			0xE0
#define VBAN_PROTOCOL_SERVICE		0x60

#define VBAN_SERVICE_REQUESTREPLY	0x02
#define VBAN_SERVICE_FNCT_REPLY		0x80

typedef struct tagVBANTXT_CONTEXT
{
	int			finitlib;
	int			mode;
	long		WSAStartupReply;
	WSADATA		WSAData;
	SOCKET		vban_socket;
	unsigned long nuGrowingFrame;
} T_VBANTXT_CONTEXT, *PT_VBANTXT_CONTEXT, *LPT_VBANTXT_CONTEXT;


static T_VBANTXT_CONTEXT G_vban_ctx={0,0};

static long VBAN_SetSocketNonblocking(SOCKET socket)
{
    u_long flags;
	flags=1;
    return ioctlsocket(socket, FIONBIO, &flags);
}     

long VBANTEXT_InitSocket(SOCKET *psocket)
{
	long rep,nnn;
	long nsize, nbByte;

	*psocket  = socket(AF_INET, SOCK_DGRAM , IPPROTO_UDP);
	if (*psocket == INVALID_SOCKET) return -1;
	//set non blocking socket
	rep=VBAN_SetSocketNonblocking(*psocket);
	if (rep != 0)
	{
		closesocket(*psocket);
		*psocket=INVALID_SOCKET;
		return -2;
	}
	//disable debug mode
	nsize=sizeof(rep);
	nnn=getsockopt(*psocket , SOL_SOCKET,SO_DEBUG, (char*)&(rep),&nsize);
	if (nnn == 0)
	{
		if (rep != 0)
		{
			rep=0;
			nsize=sizeof(rep);
			setsockopt(*psocket , SOL_SOCKET,SO_DEBUG, (char*)&rep,nsize);
		}
	}
	//set Time Out = ZERO.
	rep=0;
	nsize=sizeof(rep);
	setsockopt(*psocket , SOL_SOCKET,SO_SNDTIMEO, (char*)&rep,nsize);
	setsockopt(*psocket , SOL_SOCKET,SO_RCVTIMEO, (char*)&rep,nsize);

	//set Socket buffer size
	nbByte = 1024*128;
	nsize=sizeof(nbByte);
	setsockopt(*psocket , SOL_SOCKET,SO_RCVBUF, (char*)&nbByte ,nsize);

	//128 kB buffer is senough to send VBAN-TEXT request
	nbByte = 1024*128;
	nsize=sizeof(nbByte);
	setsockopt(*psocket , SOL_SOCKET,SO_SNDBUF, (char*)&nbByte,nsize);

	return 0;
}

long VBANTEXT_SendRequest(char * szIpaddress, unsigned short nuPort, char * szStreamName, char * szUTF8Text)
{
	SOCKADDR_IN dest;
	LPT_VBAN_HEADER lpHeader;
	char Buffer[2048];

	long rep,nbc, nbByte;
	LPT_VBANTXT_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	if (lpctx->finitlib == 0) return -1;

	// Init local buffer
	memset(Buffer, 0, 2048);
	lpctx->nuGrowingFrame++;

	// build VBAN Header
	lpHeader = (LPT_VBAN_HEADER)Buffer;
	lpHeader->vban			='NABV';	// VBAN
	lpHeader->format_SR		=0x52;		// 0x40 (TXT sub protocol) + 0x12 (256000 bps) 
	lpHeader->format_nbs	=0;			// sub channel = 0 
	lpHeader->format_nbc	=0;			// unused 
	lpHeader->format_bit	=0x10;		// UTF-8 format 
	strncpy(lpHeader->streamname, szStreamName, 16);
	lpHeader->nuFrame		=lpctx->nuGrowingFrame; 

	// copy text after header (1436 byte max);
	nbc = (long)strlen(szUTF8Text);
	if (nbc > 1436) nbc=1436;
	strncpy((char*)(lpHeader+1), szUTF8Text, nbc);
	
	//send request
	memset(&dest,0, sizeof(SOCKADDR_IN));
	dest.sin_family = AF_INET;
	dest.sin_port = htons (nuPort);		
	dest.sin_addr.s_addr = inet_addr(szIpaddress);

	nbByte = sizeof(T_VBAN_HEADER) + nbc;
	rep = sendto(lpctx->vban_socket,Buffer,nbByte,0,(struct sockaddr*)&dest, sizeof(dest));
	if (rep == SOCKET_ERROR) return -2;

	return 0;
}



long VBANTEXT_GetRequest(unsigned long *nuFrame, char * szStreamName, char * szUTF8Text)
{
	LPT_VBAN_HEADER lpHeader;
	char  Buffer[2048];
	SOCKADDR_IN from;
	long rep,datasize, fromlen, nbByte;
	unsigned char protocol,function,id;

	LPT_VBANTXT_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	if (lpctx->finitlib == 0) return -1;
	
	lpHeader = (LPT_VBAN_HEADER)Buffer;
	
	fromlen=sizeof(SOCKADDR_IN);
	rep= recvfrom(lpctx->vban_socket,Buffer,2048,0,(struct sockaddr*)&from,&fromlen);
	while (rep != SOCKET_ERROR)
	{
		// if we recieve something
		datasize = rep;
		if (datasize > sizeof(T_VBAN_HEADER))
		{
			// if it's a VBAN packet
			if (lpHeader->vban == 'NABV')
			{
				protocol =lpHeader->format_SR & VBAN_PROTOCOL_MASK;
				// if it's a VBAN-SERVICE packet
				if (protocol == VBAN_PROTOCOL_SERVICE)
				{
					function = lpHeader->format_nbs;
					id = lpHeader->format_nbc;
					if ((function == VBAN_SERVICE_FNCT_REPLY) && (id == VBAN_SERVICE_REQUESTREPLY))
					{
						// if it's SERVICE REPLY packet
						nbByte = datasize-sizeof(T_VBAN_HEADER);
						strncpy(szStreamName, lpHeader->streamname,16);
						szStreamName[16]=0;
						*nuFrame = lpHeader->nuFrame;
						if (nbByte > 0) memcpy(szUTF8Text, lpHeader+1, nbByte);
						szUTF8Text[nbByte]=0;
						return 1; //return 1 only if we get a service request response
					}
				}
			}
		}
		rep= recvfrom(lpctx->vban_socket,Buffer,2048,0,(struct sockaddr*)&from,&fromlen);
	}
	return 0;
}

long VBANTEXT_End(void)
{
	long rep;
	LPT_VBANTXT_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	if (lpctx->finitlib == 0) return 0;

	if (lpctx->vban_socket != INVALID_SOCKET)
	{
		shutdown(lpctx->vban_socket, 2);
		rep=closesocket(lpctx->vban_socket);
		lpctx->vban_socket =INVALID_SOCKET;
	}
	
	memset(lpctx,0, sizeof(T_VBANTXT_CONTEXT));
	return 0;
}

long VBANTEXT_Init(long mode)
{
	LPT_VBANTXT_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	memset(lpctx,0,sizeof(T_VBANTXT_CONTEXT));
	lpctx->mode=mode;
	lpctx->finitlib=1;
	lpctx->WSAStartupReply=WSAStartup(MAKEWORD(1,1), &(lpctx->WSAData));
	lpctx->vban_socket = INVALID_SOCKET;
	return VBANTEXT_InitSocket(&(lpctx->vban_socket));	
}


















/*******************************************************************************/
/**                                QUIT & ABOUT                               **/
/*******************************************************************************/

void ManageCloseMessage(HWND hw)
{
	PostMessage(hw,WM_DESTROY,0,0L); 
}

void ManageAboutBox(HWND hw)
{
	char titre[]="About...";
	char message[512];
	strcpy(message,SZPUBLICNAME);
	strcat(message,"\nVersion : ");
	strcat(message,SZPUBLICVERSION);
	strcat(message,"\nStandalone Application\n");
	strcat(message,"\nExample of 'C' Source code\n");
	
	MessageBox(hw,message,titre,MB_APPLMODAL | MB_OK | MB_ICONINFORMATION);
}


/*******************************************************************************/
/**                                INIT / END                                 **/
/*******************************************************************************/

BOOL InitSoftware(LPT_APP_CONTEXT lpapp, HWND hw)
{
	LOGFONT fc;
	int y0;
	lpapp->hwnd_MainWindow = hw;

	// create font
	memset(&fc,0, sizeof(LOGFONT));
	fc.lfHeight	= 14;
	fc.lfWeight	= 400;
	strcpy(fc.lfFaceName,"Courier New");
	lpapp->font1=CreateFontIndirect(&fc);

	// create windows controls
	y0 = 30;
	lpapp->hw_serveraddress=CreateWindowEx(WS_EX_CLIENTEDGE,"edit","127.0.0.1",
					 WS_CHILD | WS_VISIBLE | ES_LEFT | WS_TABSTOP,
					 10,y0,200,25,
					 hw,(HMENU)IDC_SERVERADDRESS,lpapp->hinstance,NULL);

	lpapp->hw_port_out=CreateWindowEx(WS_EX_CLIENTEDGE,"edit","6980",
					 WS_CHILD | WS_VISIBLE | ES_LEFT | WS_TABSTOP,
					 250,y0,100,25,
					 hw,(HMENU)IDC_PORT_OUT,lpapp->hinstance,NULL);

	lpapp->hw_button_send =CreateWindow("button","Send Request",WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					 400,y0-20,150,45,
					 hw,(HMENU)IDC_SEND,lpapp->hinstance,NULL);

	//CLI control (where to enter TEXT request)
	lpapp->hw_CLI=CreateWindowExW(WS_EX_CLIENTEDGE,L"edit",NULL,
				 WS_CHILD | WS_VISIBLE | ES_LEFT | WS_TABSTOP | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN,
				 0,90,400,100,
				 hw,(HMENU)IDC_CLI,lpapp->hinstance,NULL);
	SendMessage(lpapp->hw_CLI,WM_SETFONT,(WPARAM)lpapp->font1,MAKELPARAM(1,0));
	SendMessage(lpapp->hw_CLI,EM_SETMARGINS,EC_LEFTMARGIN | EC_RIGHTMARGIN,(LPARAM) MAKELONG(2,2));

	//LOG control (to display possible reply)
	lpapp->hw_Log=CreateWindowEx(WS_EX_CLIENTEDGE,"edit",NULL,
				 WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | ES_READONLY | ES_WANTRETURN,
				 0,190,400,400,
				hw,(HMENU)IDC_LOG,lpapp->hinstance,NULL);
	SendMessage(lpapp->hw_Log,WM_SETFONT,(WPARAM)lpapp->font1,MAKELPARAM(1,0));
	SendMessage(lpapp->hw_Log,EM_SETMARGINS,EC_LEFTMARGIN | EC_RIGHTMARGIN,(LPARAM) MAKELONG(2,2));


	//init log buffer	
	lpapp->pLogBuffer_nbByte = 1024 * 1024;
	lpapp->pLogBuffer_curs=0;
	lpapp->pLogBuffer = (char*)malloc(lpapp->pLogBuffer_nbByte + 1024);
	lpapp->pLogBuffer_fDirty=FALSE;

	// init VBAN stuff
	VBANTEXT_Init(0);
	return TRUE;
}

BOOL EndSoftware(LPT_APP_CONTEXT lpapp, HWND hw)
{
	VBANTEXT_End();
	if (lpapp->font1 != NULL) DeleteObject(lpapp->font1);
	lpapp->font1=NULL;
	if (lpapp->pLogBuffer != NULL) free(lpapp->pLogBuffer);
	lpapp->pLogBuffer=NULL;

	return TRUE;
}



/*******************************************************************************/
/*                               WINDOWS CALLBACK                              */
/*******************************************************************************/

void ManageMenu(LPT_APP_CONTEXT lpapp, HWND hw, WPARAM p,LPARAM w)
{
	char szIpaddress[64];
	char szPort[64];
	char szUTF8Text[2048];
	switch(LOWORD(p))
	{
		case IDM_QUIT:
			ManageCloseMessage(hw);
			break;
		case IDM_ABOUT:
			ManageAboutBox(hw);
			break;
		// Send VBAN-TEXT request:
		case IDC_SEND:
			GetWindowText(lpapp->hw_serveraddress,szIpaddress,64);
			GetWindowText(lpapp->hw_port_out,szPort,64);
			GetWindowText(lpapp->hw_CLI,szUTF8Text,2048);
			VBANTEXT_SendRequest(szIpaddress, (unsigned short)atoi(szPort), "Command1", szUTF8Text);
			//add to log
			LogBuffer_AddText(lpapp, szUTF8Text, G_vban_ctx.nuGrowingFrame, FALSE);	

			break;
	}
}

void ProcessTimer(LPT_APP_CONTEXT lpapp, HWND hw)
{
	char szStreamName[64];
	char szUTF8Text[2048];
	long rep;
	unsigned long nuFrame;

	// we check if we recieved VBAN request
	rep= 1;
	while (rep == 1)
	{
		rep= VBANTEXT_GetRequest(&nuFrame, szStreamName, szUTF8Text);
		if (rep == 1)
		{
			//add to log
			LogBuffer_AddText(lpapp, szUTF8Text, nuFrame, TRUE);	
		}
	}

	// we check if we have to update the Log window
	if (lpapp->pLogBuffer_fDirty == TRUE)
	{
		if ((lpapp->hw_Log != NULL) && (lpapp->pLogBuffer != NULL))
		{
			SetWindowText(lpapp->hw_Log, lpapp->pLogBuffer);
			PostMessage(lpapp->hw_Log, WM_VSCROLL, SB_BOTTOM, 0);
		}
		lpapp->pLogBuffer_fDirty=FALSE;
	}
}

LRESULT CALLBACK MainWindowManageEvent(HWND hw,			//handle of the window.
											UINT msg,   //Message Ident.
											WPARAM p1,	//parameter 1.
											LPARAM p2)	//parameter 2
{
	int dx,dy;
	LPT_APP_CONTEXT lpapp; 	
	char sss[256];
	HDC dc;
	PAINTSTRUCT ps;
	lpapp = &G_context;
	switch (msg)
	{


		case WM_CREATE:
			if (InitSoftware(lpapp,hw) == FALSE) return -1;//return -1 here cancel the window creation

			SetFocus(lpapp->hw_CLI);
			lpapp->winTimer=(UINT)SetTimer(hw,1200,20,NULL);
			break;
		case WM_SIZE:
			dx=(long)LOWORD(p2);
			dy=(long)HIWORD(p2);
			if (dy < 500) dy=500;
			SetWindowPos(lpapp->hw_CLI,HWND_TOP,0,90,dx,100,SWP_SHOWWINDOW | SWP_NOMOVE);
			SetWindowPos(lpapp->hw_Log,HWND_TOP,0,190,dx,dy-200,SWP_SHOWWINDOW | SWP_NOMOVE);
			break;
		case WM_TIMER:
			if (p1 == 1200) ProcessTimer(lpapp, hw);
			return 0;
		case WM_COMMAND:
			ManageMenu(lpapp, hw,p1,p2);
			break;

		case WM_PAINT:
			dc=BeginPaint(hw,&ps);
			SetTextColor(dc,RGB(60,60,60));
			SetBkMode(dc,TRANSPARENT);
			strcpy(sss,"IP-Address To");
			TextOut(dc,10,10,sss,(int)strlen(sss));

			strcpy(sss,"UDP Port:");
			TextOut(dc,250,10,sss,(int)strlen(sss));

			strcpy(sss,"VBAN-TEXT request to send:");
			TextOut(dc,10,70,sss,(int)strlen(sss));
			EndPaint(hw,&ps);
	        break;
		case WM_CLOSE:
			ManageCloseMessage(hw);
			break;
		case WM_DESTROY:
			if (lpapp->winTimer != 0) KillTimer(hw,lpapp->winTimer);
			lpapp->winTimer=0;
			EndSoftware(lpapp, hw);
			PostQuitMessage(0);
			break;
		default:
			return (DefWindowProc(hw,msg,p1,p2));

	}
	return (0L);
}


/*******************************************************************************/
/**                              MAIN PROCDURE                                **/
/*******************************************************************************/

int APIENTRY WinMain(HINSTANCE handle_app,			//Application hinstance.
							HINSTANCE handle_prev,  //NULL.
							LPTSTR param,           //Command Line Parameter.
							int com_show)           //How to display window (optionnal).
{
	LPT_APP_CONTEXT lpapp;
	long	wstyle;
	MSG		msg;    
	char	szWindowClassName[]="MainWindowClass";
	char *	title="Sorry";

	WNDCLASS	wc;
	HWND hw;
	//we first init our App Context Data structure
	lpapp = &G_context;
	memset(lpapp, 0, sizeof(T_APP_CONTEXT));
	lpapp->hinstance=handle_app;

	//here you can make some early initialization and analyze command line if any.

	//we define a window class to create a window from this class 
	wc.style		=CS_HREDRAW | CS_VREDRAW;  	  		//property.
	wc.lpfnWndProc=(WNDPROC)MainWindowManageEvent;		//Adresse of our Callback.
	wc.cbClsExtra =0;					  				//Possibility to store some byte inside a class object.
	wc.cbWndExtra =0;                          			//Possibility to store some byte inside a window object.
	wc.hInstance  =handle_app; 	                		//handle of the application hinstance.
	wc.hIcon      =LoadIcon(NULL, IDI_APPLICATION);    	//handle of icon displayed in the caption.
	wc.hCursor    =LoadCursor(NULL,IDC_ARROW);			//handle of cursor mouse .
	wc.hbrBackground=(HBRUSH)(COLOR_MENU+1);			//Background color.
	wc.lpszMenuName="MyMainAppMenu";    				//pointer on menu defined in resource.
	wc.lpszClassName=szWindowClassName;       			//pointer on class name.

	//register class.
	if (RegisterClass(&wc)==0)
	{
		MessageBox(NULL,"Failed to register main class...",title,MB_APPLMODAL | MB_OK | MB_ICONERROR);
		return 0;
	}
	//then we can create a windows from this class.
	
	//Classical Window without Sizing border.
	//wstyle=WS_DLGFRAME | WS_OVERLAPPED | WS_VISIBLE | WS_MINIMIZEBOX | WS_SYSMENU;
	
	//classical Main Window
	wstyle=WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	hw=CreateWindow(szWindowClassName,	// address of registered class name.
						 SZPUBLICNAME,					// address of window name string
						 wstyle,						// window style
						 CW_USEDEFAULT,					// horizontal position of window
						 CW_USEDEFAULT,					// vertical position of window
						 UI_WIN_DX,						// window width
						 UI_WIN_DY,						// window height
						 NULL,							// parent handle is NULL since it's a main window.
						 NULL,							// menu name defined in resource (NULL if no menu or already defined in the Class).
						 handle_app,					// handle of application instance
						 NULL); 						// address of window-creation data

	if (hw==NULL)
	{
		MessageBox(NULL,"Failed to create window...",title,MB_APPLMODAL | MB_OK | MB_ICONERROR);
		return 0;
	}
	ShowWindow(hw,SW_SHOW);				//Display the window.
	UpdateWindow(hw);					//Send WM_PAINT.
	/*---------------------------------------------------------------------------*/
	/*                             Messages Loop.                                */
	/*---------------------------------------------------------------------------*/
	while (GetMessage(&msg,NULL,0,0))	//Get Message if any.
	{
		TranslateMessage(&msg);			//Translate the virtuel keys event.
		DispatchMessage(&msg);			//DispatchMessage to the related window.
	}

	//here you can make last uninitialization and release
	return (int)(msg.wParam);
}


