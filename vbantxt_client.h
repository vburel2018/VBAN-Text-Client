/*--------------------------------------------------------------------------------*/
/* VBAN-TEXT Client minimal source code example                                   */
/*--------------------------------------------------------------------------------*/
/* WIN32 'C' Sample Code to send and recieve VBAN-TEXT request    V.Burel (c)2025 */
/*                                                                                */
/*  THIS PROGRAM HAS BEEN MADE TO CONNECT WITH MATRIX VERSION X.0.1.2 min         */
/*                                                                                */
/*  This program example shows                                                    */
/*  - How to use socket                                                           */
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

#ifndef __VBANTXTCLIENT_H__
#define __VBANTXTCLIENT_H__


//version information (for program)
#define SZPUBLICVERSION		"1.0.0.0"			//displayed version in about box
#define SZPUBLICNAME		"VBAN-TEXT Client"	//displayed title in main window 

//Information for Main window 
#define UI_WIN_DX	800
#define UI_WIN_DY	600

//version information (used in resource file)
#define __FILEVERSION__			1,0,0,0
#define __PRODUCTVERSION__		1,0,0,0
#define __SZFILEVERSION__		"1, 0, 0, 0\0"
#define __SZPRODUCTVERSION__	"1, 0, 0, 0\0"
 
	#define __COMMENTS__			"Example of source code"
	#define __COMPANYNAME__			"Audio Mechanic & Sound Breeder\0"
	#define __FILEDESCRIPTION__		"VBAN-TEXT Client Application\0"
	#define __INTERNALNAME__		"VBANTXTClient"
	#define __LEGALCOPYRIGHT__		"Copyright V.Burel©2025\0"
	#define __ORIGINALFILENAME__	"VBANTXTClient.EXE\0"
	#define __PRODUCTNAME__			"VBANTXTClient\0"


//definitions for MENU

#define IDT_DOSOMETHING		50
#define IDM_QUIT			100
#define IDM_ABOUT			101

//definitions for STRING-TABLE

#define IDS_CONFIRMCLOSE	100

#define IDC_SERVERADDRESS	200
#define IDC_PORT_OUT		201
#define IDC_SEND			202
#define IDC_CLI				300
#define IDC_LOG				400

#endif /*__MINPRG_H__*/


