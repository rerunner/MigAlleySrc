/*
	 MiG Alley
	 Copyright (C) 1998, 1999, 2000, 2001 Empire Interactive (Europe) Ltd,
	 677 High Road, North Finchley, London N12 0DA

	 Please see the document licence.doc for the full licence agreement

2. LICENCE
 2.1 	
 	Subject to the provisions of this Agreement we now grant to you the 
 	following rights in respect of the Source Code:
  2.1.1 
  	the non-exclusive right to Exploit  the Source Code and Executable 
  	Code on any medium; and 
  2.1.2 
  	the non-exclusive right to create and distribute Derivative Works.
 2.2 	
 	Subject to the provisions of this Agreement we now grant you the
	following rights in respect of the Object Code:
  2.2.1 
	the non-exclusive right to Exploit the Object Code on the same
	terms and conditions set out in clause 3, provided that any
	distribution is done so on the terms of this Agreement and is
	accompanied by the Source Code and Executable Code (as
	applicable).

3. GENERAL OBLIGATIONS
 3.1 
 	In consideration of the licence granted in clause 2.1 you now agree:
  3.1.1 
	that when you distribute the Source Code or Executable Code or
	any Derivative Works to Recipients you will also include the
	terms of this Agreement;
  3.1.2 
	that when you make the Source Code, Executable Code or any
	Derivative Works ("Materials") available to download, you will
	ensure that Recipients must accept the terms of this Agreement
	before being allowed to download such Materials;
  3.1.3 
	that by Exploiting the Source Code or Executable Code you may
	not impose any further restrictions on a Recipient's subsequent
	Exploitation of the Source Code or Executable Code other than
	those contained in the terms and conditions of this Agreement;
  3.1.4 
	not (and not to allow any third party) to profit or make any
	charge for the Source Code, or Executable Code, any
	Exploitation of the Source Code or Executable Code, or for any
	Derivative Works;
  3.1.5 
	not to place any restrictions on the operability of the Source 
	Code;
  3.1.6 
	to attach prominent notices to any Derivative Works stating
	that you have changed the Source Code or Executable Code and to
	include the details anddate of such change; and
  3.1.7 
  	not to Exploit the Source Code or Executable Code otherwise than
	as expressly permitted by  this Agreement.

questions about this file may be asked at http://www.simhq.com/
*/

//------------------------------------------------------------------------------
//Filename       ddrwinit.cpp
//System         
//Author         Andrew McRae
//Date           Mon 25 Nov 1996
//Description    Windows version of hardinit.cpp
//------------------------------------------------------------------------------

#define INITGUID
#define WIN32_LEAN_AND_MEAN
#undef LPDDCAPS
//#include <windows.h> // NHV
#include <dos.h>

#define	F_GRAFIX
#include "DosDefs.H"
#include <ddraw.h>
#include "Display.H"
//NHV #undef LPDDCAPS
//NHV #include <windows.h>

#include "Palette.H"
#define	F_BATTLE
#define	F_GRAFIX
#include "Display.H"
#include "Win3d.h"												//PD 31Jan97
#include "myError.h"
#include "hardpasm.h"
#include "polygon.h"
#include "savegame.h"

//#include "winmove.h"
#include "gameset.h"

// Global DirectDraw structures
// cannot include windows.h in display.h

DDSURFACEDESC   ddsd;
DDCAPS			DDDriverCaps;
DDCAPS			DDHelCaps;

//------------------------------------------------------------------------------
//Procedure		GetDOSRAM
//Author		Andrew McRae
//Date			Mon 25 Nov 1996
//
//Description	Reserve some memory
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
short Display::GetDOSRAM(long& buffer, short size)
{
	buffer = (int)new char [size];

	return ((buffer!=NULL)?TRUE:FALSE);
}

//------------------------------------------------------------------------------
//Procedure		GetDOSRAM
//Author		Paul.   
//Date			Wed 26 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool Display::GetDOSRAM(DPMIMem&	dpmimem,			//Definition for mem region
						ULong		size,		//0		//Size overiding dpmimem.size
						UWord		off			//-1	//Offset overriding dpmimem.off
						)
{
	dpmimem.flat = new char [size];
	dpmimem.desc = TRUE;
	dpmimem.seg = TRUE;

	return(TRUE);
}

//------------------------------------------------------------------------------
//Procedure		FreeDOSRAM
//Author		Andrew McRae
//Date			Mon 25 Nov 1996
//
//Description	Free memory.
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Display::FreeDOSRAM(short handle)
{
	DEBUGTEXT("Display::FreeDOSRAM does nowt\n");
}

//------------------------------------------------------------------------------
//Procedure		FreeDOSRAM
//Author		Paul.   
//Date			Wed 26 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Display::FreeDOSRAM(DPMIMem& handle)
{
	delete [] handle.flat;
}
	

//------------------------------------------------------------------------------
//Procedure		MemScreen
//Author		Paul.   
//Date			Wed 26 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Display::MemScreen(ScreenPages dpg)
{
	DirectDraw_MemScreen(dpg);										//ARM 23Jul96
}

//------------------------------------------------------------------------------
//Procedure		FindModeWin
//Author		Jim Taylor
//Date			Tue 12 Dec 1995
//
//Description	Tests if we can set this mode up under windows
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Display::FindModeWin(InitDisplayDef& mode)
{
	ScreenNumOnLogical=0;
	prevmodeAX=0xffff;
	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		SelectLibWin
//Author		Jim Taylor
//Date			Tue 12 Dec 1995
//
//Description	Selects correct driver file given general windows support
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
FileNum	Display::SelectLibWin()
{
	return (INVALIDFILENUM);
}

enum	{	VESA_1=31,
			VESA_support=1,VESA_info=2,VESA_bios=4,VESA_colour=8,VESA_graphics=16,
		 	VESA_2=255,
			VESA_nonvga=32,VESA_NoBank=64,VESA_Linear=128};
enum	{VWIN_avail=1,VWIN_read=2,VWIN_write=4};

struct	VesaModeInfo
{
 	UWord	flags;
	UByte	WinAFlags;
	UByte	WinBFlags;
	UWord	Granularity;
	UWord	WinSize;
	UWord	WinASeg;
	UWord	WinBSeg;
	ULong	DirectCall;
	UWord	BytesPerScanLine;
//extended info
	UWord	HoriRes;
	UWord	VertRes;
	UByte	XXCharWid;
	UByte	XXCharHeight;
	UByte	XXBitPlanes;
	UByte	BitPerPixel;
	UByte	XXMemBanks;
	UByte	MemModel;
	UByte	XXMemBanksize;
	UByte	DisplayPages;
	UByte	ResByte1;
//vesa 1.2 info
	UByte	rgbmsp[4][2];
	enum	{V12FLAG_pal68=1,V12FLAG_pl4avail};
	UByte	v12flag;
//vesa 2.0 info
	ULong	linscr1;
	ULong	linscr2;
	ULong	linmemk;
};

//------------------------------------------------------------------------------
//Procedure		ListVesaModes
//Author		Jim Taylor
//Date			Wed 23 Oct 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Window::ListVesaModes(ScreenRez* modelist)
{
	Master()->ListVesaModes(modelist);
}

//------------------------------------------------------------------------------
//Procedure		ListVesaModes
//Author		Paul.   
//Date			Fri 28 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Display::ListVesaModes(ScreenRez* modelist)
{
	ULong n;
	int	i;

	for (i=0;i<20;i++)
	{
		modelist[i].width=0;		// clear list
		modelist[i].colourdepth=8;	// clear list
		modelist[i].driver=-1;		// clear list
	}

	for (n=0; n<DD.cModes; n++)
	{
		// emit valid mode into list

		modelist[n].width = -DD.DDModes[n].width;
		modelist[n].colourdepth = DD.DDModes[n].colourdepth;
		modelist[n].driver = DD.DDModes[n].driver;

		if (n == 20)
			break;
	}

	return;

}


//------------------------------------------------------------------------------
//Procedure		FindModeVESA
//Author		Jim Taylor
//Date			Tue 12 Dec 1995
//
//Description	Tests to see if given resolution supported by VESA drivers
//
//Inputs		mode defines X,Y,colourdepth
//
//Returns		If TRUE, then sets up most fields.
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//Procedure		FindModeVESA
//Author		Andrew McRae
//Date			Mon 25 Nov 1996
//
//Description	Test to see if given mode is supported by direct draw.
//				Modes must have been enumerated first.
//
//Inputs		Mode
//
//Returns		True/False
//
//------------------------------------------------------------------------------
Bool	Display::FindModeVESA(InitDisplayDef& mode)
{
	return FALSE;
}

//------------------------------------------------------------------------------
//Procedure		SelectLibVESA
//Author		Jim Taylor
//Date			Tue 12 Dec 1995
//
//Description	Supposed to select the correct vesa driver library based on 
//				info filled in above.
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
FileNum	Display::SelectLibVESA()
{
	if (svgalibmodefix==SVGA_VESAc)
		return (LIB_VBE_COPY);
	elser
	if (svgalibmodefix==SVGA_VESAf)
		return (LIB_VBE_SWAP2);
	elser
		return (LIB_VBE_DEBUG);
}



//------------------------------------------------------------------------------
//Procedure		FindModeDirty
//Author		Jim Taylor
//Date			Tue 12 Dec 1995
//
//Description	Supposed to test for custom driver (eg 320x240)
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Display::FindModeDirty(InitDisplayDef& mode)
{
	return FALSE;
}

//------------------------------------------------------------------------------
//Procedure		SelectLibDirty
//Author		Jim Taylor
//Date			Tue 12 Dec 1995
//
//Description	Supposed to load selected custom driver
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
FileNum	Display::SelectLibDirty()
{
	if(svgalibmodefix==SVGA_CUSTOM_1d)
		return (LIB_320_DEBUG);
	else
		return (LIB_320_COPY);
}

//------------------------------------------------------------------------------
//Procedure		LoadGraphicsLib
//Author		Jim Taylor
//Date			Wed 13 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Display::LoadGraphicsLib(FileNum f)
{
	if (f == INVALIDFILENUM) return;							//ARM 07Jun96

//	STUBLOAD(Display::,f);
}


//------------------------------------------------------------------------------
//Procedure		ChangeRes
//Author		Andrew McRae
//Date			Tue 26 Nov 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Display::ChangeRes(ScreenRez& srez)
{
}

//------------------------------------------------------------------------------
//Procedure		Exit
//Author		R. Hyde 
//Date			Wed 30 Oct 1996
//
//Description	release logical screen memory on exit / mode change
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Display::Exit()
{
	ResetGraphicsMode();

	KillSurfaces ();

	LogicalPtr lp=logicalscreenptr;
	logicalscreenptr=(LogicalPtr)-1;							//PD 07Jul97
}

//------------------------------------------------------------------------------
//Procedure		SetTextMode
//Author		Jim Taylor
//Date			Tue 12 Dec 1995
//
//Description	Set back to remembered text mode
//				Needs to cope with windows!
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Display::SetTextMode()
{
	return;
}

SWord	winmode_w,winmode_h,winmode_bpp;

//------------------------------------------------------------------------------
//Procedure		InitDirectDraw
//Author		Andrew McRae
//Date			Mon 25 Nov 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool Display::InitDirectDraw ()
{
    HRESULT         ddrval;
    DDSCAPS         ddscaps;

	// Init Structure Pointers
	DD.lpddsd 			= &ddsd;
	DD.lpDDDriverCaps 	= &DDDriverCaps;
	DD.lpDDHelCaps 		= &DDHelCaps;

	DD.lpDDSPrimary = NULL;
	DD.lpDDSBack 	= NULL;
	DD.lpDDSBack2 	= NULL;

	DD.lpDDPal 		= NULL;									//PD 28Feb97
	DD.PaletteBuffer= NULL;									//PD 28Feb97

	DD.cModes 	= 0;
	DD.ModeX	= FALSE;

	DD.lpDirect3D = NULL;									//PD 10Mar97
	DD.lpImageMapTable2 = NULL;								//PD 10Mar97

    // Create DirectDraw Object
	// Constructor builds a driver and supported mode
	// list internally - scans for hardware support
	// Also selects a driver capeable of 640x480x8 as the
	// current driver
	if (!DD.lpDirectDD)
		DD.lpDirectDD 	= new DirectDD;
	DD.lpDD2 		= DD.lpDirectDD->lpDD2;

	if (DD.lpDD2==NULL)											//PD 05Feb97
	{
		ExitDirectDraw ();
		_Error.ReallyEmitSysErr("Failed to find Direct Draw modes!");
        return FALSE;
	}

	// Get Capabilities of hardware

	DD.lpDDDriverCaps->dwSize	=
	DD.lpDDHelCaps->dwSize 		= sizeof (DDCAPS);

	ddrval = DD.lpDD2->GetCaps(	DD.lpDDDriverCaps,
									DD.lpDDHelCaps);
	if (ddrval != DD_OK)
	{
		ExitDirectDraw ();
		return FALSE;
	}

	if (!EnumModes())
	{
		ExitDirectDraw ();
		return FALSE;
	}

	// Set default mode 640x480x8

	//Won't get here unless the driver can
	//support 640x480x8 so no error checking
	//is required
	DD.DDCurrMode=0xffff;

	if (DD.lpDirectDD->isFullScreen())
		for (int x=0;x<DD.cModes;x++)
		{
			if (DD.DDModes[x].width==gameSettings.m_dwWidth &&
				DD.DDModes[x].height==gameSettings.m_dwHeight &&
				DD.DDModes[x].colourdepth==16)
			{ //NHV the real screen width / height is set here for the 3d world
				DD.DDCurrMode=x;
		Mono_Text.Print(gameSettings.m_dwHeight,UByteP("gameSettingsHeight"));
		Mono_Text.Print(gameSettings.m_dwWidth,UByteP("gameSettingsWidth"));
		Mono_Text.Print(DD.DDCurrMode,UByteP("Mode num"));


				break;
			}
		}

	if (DD.DDCurrMode==0xFFFF)
		for (int counter=0;counter<DD.cModes;counter++)
			if (DD.DDModes[counter].width==winmode_w &&
		
				DD.DDModes[counter].height==winmode_h &&
				DD.DDModes[counter].colourdepth==winmode_bpp)
			{
				DD.DDCurrMode=counter;
		Mono_Text.Print(winmode_w,UByteP("winmode_w"));
		Mono_Text.Print(winmode_h,UByteP("winmode_h"));
		Mono_Text.Print(DD.DDCurrMode,UByteP("Mode num"));
			}

	if (DD.DDCurrMode!=0xffff)
	if (!Save_Data.fSoftware &&
		DD.lpDirectDD->DriverDoesHardware3D() &&				//PD 19Nov97
		DD.DDModes[DD.DDCurrMode].colourdepth!=8)				//PD 19Nov97
		DD.lpDirect3D=new Direct3D(&DD);						//PD 19Nov97
	else														//PD 19Nov97
		DD.lpDirect3D=NULL;										//PD 19Nov97

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		ExitDirectDraw
//Author		Andrew McRae
//Date			Mon 25 Nov 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool Display::ExitDirectDraw ()
{
	HRESULT	dderrval;

	DD.ModeX = FALSE;

	// Exit Direct Draw

    if (DD.lpDD2 != NULL)   KillSurfaces ();

	// Free Palette Table

	if (DD.PaletteBuffer!=NULL)
	{
		delete (DD.PaletteBuffer);
		DD.PaletteBuffer=NULL;
	}

	if( DD.lpDirectDD->isFullScreen()){
		dderrval=DD.lpDD2->RestoreDisplayMode();
		dderrval=DD.lpDD2->SetCooperativeLevel(DD.hWndApp,DDSCL_NORMAL);
		dderrval=DD.lpDD2->SetCooperativeLevel(DD.hWndApp,DDSCL_NORMAL);
	}

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		EnumModes
//Author		Andrew McRae
//Date			Mon 25 Nov 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Display::EnumModes ()
{
	DD.lpDirectDD->GetModes(DD);

	return TRUE;
}

