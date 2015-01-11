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
//Filename       lbm.cpp
//System         new eng
//Author         Dave Whiteside
//Date           Thu 11 Jan 1996
//Description    lbm unpacking with transparancy stuff
//------------------------------------------------------------------------------
#include	"dosdefs.h"
#include	<string.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	"fonts.h"
#define		F_GRAFIX											//JIM 01Aug96
#include	"display.h"
//DeadCode JIM 02Jul96 #include	<assert.h>
#include	<fstream>
#include	"fileman.h"
#include	"lbm.h"
#include	"mouse.h"
//DeadCode JIM 08Apr97 #include	<stdio.h>

UWord	screenshotnum=0;
//------------------------------------------------------------------------------
//Procedure		ScreenShot
//Author		Dave Whiteside
//Date			Mon 19 Feb 1996
//
//Description	Take a ScreenShot and write the results to a file
//
//Inputs		WindowPtr
//
//Returns		
//
//------------------------------------------------------------------------------
void Graphic::ScreenShot()
{
	LBM		screenShot;
	ULong	packedlength;

	screenShot.MakeHead(PhysicalWidth,PhysicalHeight);
	screenShot.MakePall((char *)Palette);
	packedlength = screenShot.MakeBody(logicalscreenptr,BytesPerScanLine);
	screenShot.Save(screenshotnum++,packedlength);
	if (screenshotnum>99)
		screenshotnum=0;
}

LBM::LBM()
{
//	body		= new char[DEFAULT_LBM*DEFAULT_LBM];
	body		= new UByte[DEFAULT_LBM*DEFAULT_LBM];
	hdr			= new LBMHeader;
	pal			= new char[3*256];
	hdr->width	= DEFAULT_LBM;
	hdr->height	= DEFAULT_LBM;

}


LBM::~LBM()
{
	delete	hdr;
	delete	body;
	delete	pal;
}


void LBM::MakeHead(short w, short h)
{
	delete		hdr;

	hdr			= new LBMHeader;
	hdr->width	= w;
	hdr->height	= h;
	hdr->x		= 0;
	hdr->y		= 0;
	hdr->nPlanes= 8;
	hdr->masking= 0;
//Old_Code DAW 22Feb96 	hdr->compression=cmpNone;
	hdr->compression=cmpByteRun1;
	hdr->pad1	= 0;
	hdr->transparentColour= 0xff;
	hdr->xAspect= 1;
	hdr->yAspect= 1;
	hdr->pageWidth= w;
	hdr->pageHeight= h;

}

ULong LBM::MakeBody(LogicalPtr logicalscreenptr,int	BytesPerScanLine)
{
	delete		body;
	ULong		filelen,m,packedlength,x,k;
	UByte		c;
	//char*		p,q;

	body= new UByte[hdr->width*hdr->height*2];		//*2 cause it may be massive
//	body= new char[hdr->width*hdr->height*2];		//*2 cause it may be massive
//	q= 	*body;
	packedlength=0;
	m=0;
	for (int y=0;y<hdr->height;y++)
	{
		x=0;
		while (x<hdr->width)
		{
			if (x<hdr->width && (logicalscreenptr[x] == logicalscreenptr[x+1]))
			{											//packable
				k=0;
				c=logicalscreenptr[x+k];
				while ((logicalscreenptr[x+k]==c) && k<127 && ((x+k)<hdr->width))
					k++;
				body[packedlength++]=257-k;
				body[packedlength++]=c;
				x+=k;
			}
			else
			{											//not packable
				k=1;
				c=logicalscreenptr[x];
				while ((x+k < hdr->width-2 && (logicalscreenptr[x+k]!=c || logicalscreenptr[x+k+1]!=c)) && (k<127 && ((x+k)<hdr->width)))
					c=logicalscreenptr[x+k++];
				body[packedlength++]=k-1;
				for (int i=0;i<k;i++)
					body[packedlength++]=logicalscreenptr[(x++)];
				}
		}
		logicalscreenptr+=BytesPerScanLine;
	}
	return(packedlength);
}

void LBM::MakePall(char* newpal)
{
	delete	pal;
	pal = new char[3*256];
	for (int i=0;i<768;i++)
		pal[i]=newpal[i];										//JIM 16Jul96
}

void Put68KLong(FILE *fp,int size)
{
	fputc(size>>24,fp);
	fputc(size>>16,fp);
	fputc(size>>8,fp);
	fputc(size,fp);
}

void Put68KWord(FILE *fp,UWord size)
{
	fputc(size>>8,fp);
	fputc(size,fp);
}

void	LBM::Save(UWord ScreenShot,ULong packedlength)
{
	FileNum	fil;
	FILE	*fp;
	div_t	val;
	char	*p;
	char	c;

	string nom=FILEMAN.namenumberedfile(FIL_STILLS_START);
	val = div(ScreenShot,10);
	p= 	strrchr(nom,'0');
	*p	=(val.rem)+'0';
	p--;
	*p	=(val.quot)+'0';
	p= 	strrchr(nom,'.');										//DAW 03Jun97
	p++;
	*p ='L';
	p++;
	*p='B';
	p++;
	*p='M';

	fp=fopen(nom,"wb");
	if (fp)
	{
		fprintf(fp,"FORM");
		Put68KLong(fp,48+3*256+packedlength);

		fprintf(fp,"PBM BMHD");
		Put68KLong(fp,20);
		Put68KWord(fp,hdr->width);
		Put68KWord(fp,hdr->height);
		Put68KWord(fp,hdr->x);
		Put68KWord(fp,hdr->y);
		fputc(hdr->nPlanes,fp);
		fputc(hdr->masking,fp);
		fputc(hdr->compression,fp);
		fputc(hdr->pad1,fp);
		Put68KWord(fp,hdr->transparentColour);
		putc(hdr->xAspect,fp);
		putc(hdr->yAspect,fp);
		Put68KWord(fp,hdr->pageWidth);
		Put68KWord(fp,hdr->pageHeight);

		fprintf(fp,"CMAP");
		Put68KLong(fp,3*256);
		fwrite(pal,3,256,fp);

		fprintf(fp,"BODY");
		Put68KLong(fp,packedlength);
		fwrite(body,1,packedlength,fp);
//Old_Code DAW 22Feb96 		Put68KLong(fp,hdr->width*hdr->height);
//Old_Code DAW 22Feb96 		fwrite(body,1,hdr->width*hdr->height,fp);

		fclose(fp);

	}
}


//------------------------------------------------------------------------------
//Procedure		FillExpandBuff
//Author		Jim Taylor
//Date			Tue 8 Apr 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
UByte	Graphic::ExpandBuff[256][8];
SLong	Graphic::fontcolour=-2;

void	Graphic::FillExpandBuff()
{
	for (int x=0;x<256;x++)
		for (int y=0;y<8;y++)
			ExpandBuff[x][y]=x;
}


//------------------------------------------------------------------------------
//Procedure	Graphic::UnpackRow
//------------------------------------------------------------------------------
//Author		Paul.   
//Date			Fri 7 Jul 1995
//Modified		DAW 11Jan96
//
//Description	Unpack a row of artwork
//
//Inputs		Source data
//				Length of line in data
//				start	x
//				start	y
//				physical min x
//				physical max x
//
//Returns		Data ptr moved
//
//Externals
//------------------------------------------------------------------------------
UByte *Graphic::UnpackRow(UByte *c, SLong width, SLong x, SLong y, SLong minx, SLong maxx)
//------------------------------------------------------------------------------
{
//DEADCODE JIM 29/03/99 	if (BytesPerPixel>=2)
//DEADCODE JIM 29/03/99 		if (BytesPerPixel<3)
//DEADCODE JIM 29/03/99 			return (Unpack2Row(c,width,x<<1,y,minx<<1,maxx<<1));
//DEADCODE JIM 29/03/99 		elseif (BytesPerPixel>3)
//DEADCODE JIM 29/03/99 			return (Unpack4Row(c,width,x<<2,y,minx<<2,maxx<<2));
//DEADCODE JIM 29/03/99 		else
//DEADCODE JIM 29/03/99 			return (Unpack3Row(c,width,x<<2,y,minx<<2,maxx<<2));

#define	TRANSFERVALUESIZE	UByte
#define	LOADVALUE(x)	TRANSFERVALUE=x
#define	COPYVALUE(t,s)	*t++=s

#include	"lbmcpp.h"

//------------------------------------------------------------------------------
//Procedure	Graphic::SkipRow
//------------------------------------------------------------------------------
//Author		Paul.   
//Date			Fri 7 Jul 1995
//Modified		DAW 19Jan96
//
//Description	cleaned and made slightly more cunning
//
//Inputs		data area
//				length of line
//
//Returns		new position in the data area
//
//Externals		Nufin
//------------------------------------------------------------------------------
UByte *Graphic::SkipRow(UByte *c, SWord length) 
{
	while(length>0)
	{
	SLong	Count;
	char	b=*c;

		if(b<0x080)						//Copy
		{
			Count = 1+(SByte)b;
			c+=Count+1;
		}
		else
		{
			if(b>0x080)					//Run
			{
				Count = 1-(SByte)b;
				c+=2;
			}
			else							//transparent
			{
				c++;
				Count = 1 + (UByte)*c++;
			}
		}
		length -= Count;
	}
	return(c);
}

//------------------------------------------------------------------------------
//Procedure	Graphic::SearchIFFHunk
//------------------------------------------------------------------------------
//Author		Paul.   
//Date			Fri 7 Jul 1995
//Modified		DAW 22Jan96 - made it more safe and it now looks at the header
//				for each chunk for a faster safer search
//
//Description	Search a buffer for a 4 character string
//				LBM's always have a header that looks like:-
//				FORMxxxx
//				PBM 
//				BMHDxxxx
//				[CMAPxxxx|BODYxxxx]
//				where xxxx is a 68k style long word denoting length of this block
//
//Inputs		ptr to search string
//				ptr to buffer - LBM style in mem [ has blocks
//									FORM, BMHD, CMAP, BODY]
//
//Returns		position of correct data or 0 if failed
//
//Externals
//------------------------------------------------------------------------------
UByte *Graphic::SearchIFFHunk(UByte *search,UByte *ptr)			//MGA 21Feb96
{
	ULong	maxlen;
	UByte*	tmp=0;

	if (((ULong*)ptr)[0]==((ULong*)"FORM")[0])					//MGA 21Feb96
	{																//Good
		maxlen=(ptr[4]<<24)+(ptr[5]<<16)+(ptr[6]<<8)+ptr[7];
		ptr+=12;
		tmp=ptr+maxlen;

		while (		(((ULong*)ptr)[0]!=((ULong*)"BODY")[0])		//MGA 21Feb96
				&&	(((ULong*)ptr)[0]!=((ULong*)search)[0])		//MGA 21Feb96
			)
			ptr+=(ptr[4]<<24)+(ptr[5]<<16)+(ptr[6]<<8)+ptr[7]+8;
	}
	if (((ULong*)ptr)[0]==((ULong*)search)[0])					//MGA 21Feb96
		return(ptr+8);
	else
		return(0);												//MGA 21Feb96
}																//jim actually.

//------------------------------------------------------------------------------
//Procedure		SearchIFFHunks
//Author		Paul.   
//Date			Tue 27 Oct 1998
//------------------------------------------------------------------------------
SLong Graphic::SearchIFFHunks(SLong count,IFFHunkSearch* pSearch,UByte* ptr)
{

	SLong found=0;

	if (((ULong*)ptr)[0]==((ULong*)"FORM")[0])
	{
		ptr+=12;

		while (found!=count)
		{
			for (SLong i=0;i<count;i++)
				if (pSearch[i].hunkPtr==NULL &&
					pSearch[i].searchVal==*(ULong*)ptr)
				{
					pSearch[i].hunkPtr=ptr+8;
					found++;
					break;
				}

			if (*(ULong*)ptr==*(ULong*)"BODY") break;

			ptr+=(ptr[4]<<24)+(ptr[5]<<16)+(ptr[6]<<8)+ptr[7]+8;
		}
	}

	return found;
}

//------------------------------------------------------------------------------
//Procedure		Graphic::Readword
//------------------------------------------------------------------------------
//Author		Paul.   
//Date			Fri 7 Jul 1995
//Modified		DAW 22Jan96
//
//Description	Read a word in 68k format, from the buffer and store it in the
//				variable and return the new position of the buffer
//
//Inputs		buffer and variable
//
//Returns		buffer and variable
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
UByte *Graphic::ReadWord(UByte *c, SWord &number)
{
	number=(c[0]<<8)+c[1];
	c+=2;
	return(c);
}

//------------------------------------------------------------------------------
//Procedure		Graphic::ReadLong
//------------------------------------------------------------------------------
//Author		Paul.   
//Date			Fri 7 Jul 1995
//Modified		DAW 22Jan96
//
//Description	Read a Long in 68k format
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
UByte *Graphic::ReadLong(UByte *c, SLong &number)
{
	number=(c[0]<<24)+(c[1]<<16)+(c[2]<<8)+c[3];
	c+=4;
	return(c);
}
