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
//Filename       sky.cpp
//System         
//Author         Andrew McRae
//Date           Mon 10 Mar 1997
//Description    Atmosphere Class
//------------------------------------------------------------------------------
//	Jobs
//	
//	Use savegame variables instead of WIND and GUSTS
//	SetContrails sets the centre of the contrail band. This needs is a variable that we need to vary
//	Use SetConditions, Wind and Gust to set things up for each day
//
//	Notes
//	SetConditions allows us to set up temp and press using fraction of allowable range as input
//	SetWind allows us to set the low and high wind speeds
//	SetGusts allows us to set up wind gusts
//
#include <stdio.h>
#include <string.h>

#include "dosdefs.h"
#include "worldinc.h"
#include "modvec.h"
#include "model.h"
#include "monotxt.h"
#include "mytime.h"


//------------------------------------------------------------------------------
//Procedure		Atmosphere
//Author		Andrew McRae
//Date			Mon 10 Mar 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
//Atmosphere::Atmosphere ()
//{
//	WIND = FALSE;
//	GUSTS = FALSE;

//	pMinTemp = _CurveRsc.FindCurve ("ATMOS", "MinTemp");
//	pMaxTemp = _CurveRsc.FindCurve ("ATMOS", "MaxTemp");
  
//	pMinPressure = _CurveRsc.FindCurve ("ATMOS", "MinPres");
//	pMaxPressure = _CurveRsc.FindCurve ("ATMOS", "MaxPres");

	// Atmosphere (temp location)
//TempCode ARM 10Oct97 	Sky.SetConditions (288, 217, 101300);

//	Sky.SetConditions (.5,.5);
//standard atmosphere
//	Sky.SetConditions (.6,.5);

	
//	Sky.SetWind (15, 0, 0);

//	Sky.SetContrails(FEET2CM(36000));
//	FCRD wind_at_zero, wind_at_30k;
//
//	wind_at_zero.x = 15;
//	wind_at_zero.y = 0;
//	wind_at_zero.z = 0;
//
//	wind_at_30k.x = 50;
//	wind_at_30k.y = 0;
//	wind_at_30k.z = 20;
//
//	Sky.SetWind (wind_at_zero, wind_at_30k, FEET2CM(30000));

//	Sky.SetGusts (25, 125, 3, 7, 56000);

//}
//void Atmosphere::SetContrails (FP contraillevel)
//{
//	Sky.ContrailBandCentre = contraillevel;
//}
void Atmosphere::SetMissionWind (SWord month)
{

	FP pie;
	pie = 3.14159265359;
	FP delta;
	delta = month - 6;
	if (delta < 0)
		delta = - delta;
	if (delta < 3)
	{//Summer
		wind0 = Math_Lib.rnd (6);
		dir0 = 135 + Math_Lib.rnd (45);
		windalt = 80 + Math_Lib.rnd (45);
		diralt = 45 + Math_Lib.rnd (90);

	}else
	{
		wind0 = Math_Lib.rnd (10);
		dir0 = 225 + Math_Lib.rnd (90);
		windalt = 100 + Math_Lib.rnd (75);
		diralt = 270 + Math_Lib.rnd (45);
	}

//calc wind as vector
//wind0 and windalt are in knots, the x,yz vectors are in m/s
	FP dir = (180 + dir0) * pie * 2 / 360;
	FP cos = FCos(dir);
	FP sin = FSin(dir);
	Wind0.x = wind0 * sin * 0.514;									  //RDH 02/05/99
	Wind0.y = 0;
	Wind0.z = wind0 * cos * 0.514;									  //RDH 02/05/99

	dir = (180 + diralt) * pie * 2 / 360;
	cos = FCos(dir);
	sin = FSin(dir);
	WindAlt.x = windalt * sin * 0.514;								  //RDH 02/05/99
	WindAlt.y = 0;
	WindAlt.z = windalt * cos * 0.514;								  //RDH 02/05/99

	WindAltHeight = WIND_DIRECTION_CHANGE;		//always as 35,000ft for MA

//##calc as f(date)
	delta = TempVar;
	if (delta < 0)
		delta = -delta;
//DeadCode CSB 23/12/98		Frequency = 6553 * delta;		// 0 = always, 65535 = very infrequent
//DeadCode CSB 23/12/98	
//DeadCode CSB 23/12/98		delta = delta /4;
//DeadCode CSB 23/12/98		MinMagnitude = 0;				// 1 = 1 m/s
//DeadCode CSB 23/12/98		MaxMagnitude = delta;
//DeadCode CSB 23/12/98		MaxDuration = 100 * delta;
//DeadCode CSB 23/12/98		MinDuration = MaxDuration / 4;		// csecs

	Frequency = 65535 - 26 * delta;		// 0 = always, 65535 = very infrequent

	MaxMagnitude = delta;
	MinMagnitude = 0;				// 1 = 1 m/s
	MaxDuration  = 500;
	MinDuration  = 250;		// csecs

}
void Atmosphere::SetMissionPress (SWord month)
{
	FP delta;
	delta = month - 6;
	if (delta < 0)
		delta = - delta;
	if (delta < 3)
	{//Summer
		Press0 = 1001 + TempVar;
	}else
	{
		Press0 = 999 - TempVar;
	}

	ContrailBandCentre = FEET2CM(32000);		//CSB 10/06/99	
	ContrailBandWidth = FEET2CM(4000);	//CSB 10/06/99	

	FP ChanceBad = 0.5 + 0.5 * FCos(month * 6.2831853 / 12.0);		//CSB 02/07/99	
	ChanceBad = 819.2 + 8192.0 * ChanceBad;						//CSB 02/07/99	
	if(Math_Lib.rnd() > ChanceBad)									//CSB 02/07/99	
	{
		Conditions = 0;
		Visibility = 4000000;
	}
	else
	{
		Visibility = 4000000;			//##lower vis possible here
		if (Temp0 < 273)
			Conditions = 2;
		else
			Conditions = 1;
	}

	if (		(Press0 < 980)
			&&	(MMC.currtime <  HR09)
		)
		MistInValley = true;
	else
		MistInValley = false;

//this will be a function of temp and press
	CloudLayer = 457200;

}
void Atmosphere::SetMissionTemp (SWord month)
{
	int maxt[12] =	{3,5,8,12,20,25,25,25,23,15,10,5};
	int mint[12] =	{-10,-10,-5,0,2,8,10,10,8,6,0,-6};
	int maxtemp, mintemp;
	FP		delta;
	FP		hr12 = SECSPERDAY/2;

	maxtemp = maxt[month];
	mintemp = mint[month];

	delta = MMC.currtime - hr12;
	if (delta < 0 )
		delta = - delta;
	delta = hr12 - delta;
	Temp0 = (maxtemp - mintemp) * delta / hr12;
	FP var = Math_Lib.rnd (7);
	var = var - 3;
	TempVar = TempVar + var;
	if (TempVar > 10)
		TempVar = 10;
	if (TempVar < -10)
		TempVar = -10;
	Temp0 = 273 + Temp0 + TempVar + mintemp;
}

void Atmosphere::SetMissionDensity()	//CSB 27/10/98
{
	Density0 = 10.1325 * Press0 * 0.001 / (287 * Temp0);
}

void Atmosphere::SetMissionConditions ()
{
	FCRD wind_at_zero, wind_at_alt;

  	SWord	month = 0;
	SWord	day = 0,year = 0;
	Math_Lib.DateFromSecs (MMC.currdate, day,month,year);

	SetMissionTemp (month);
	SetMissionPress (month);
	SetMissionDensity();

	SetMissionWind(month);
 
	SWord Latitude = 38;
	SetMissionDawnDusk(Latitude, MMC.currdate, MMC.dawntime, MMC.dusktime);
}


//------------------------------------------------------------------------------
//Procedure		Ambient
//Author		Andrew McRae
//Date			Mon 13 Jan 1997
//
//Description	Calculates atmospheric density, temperature and pressure at 
//				an altitude
//
//Inputs		height above sea level
//
//Returns	
//
//------------------------------------------------------------------------------
Bool Atmosphere::Ambient (FP ypos, FP& density, FP& temp, FP& pres)
{
	if (ypos < 0) ypos = 0;

	FP TempRatio = 1 - 0.00000023064 * ypos;
	if(ypos < 1100000) 	temp = TempRatio * 288.15;
	else				temp = 216.61;

	temp += (Temp0 - 288.15);

	density = Density0 * pow((float)TempRatio, (float)4.14);

	pres = density * 287 * temp;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		GetWindDirVel
//Author		Craig Beeston
//Date			Thu 10 Jun 1999
//
//Description	Returns the wind speed and direction at a given altitude
//
//Inputs		Alt
//
//Returns		Wind Heading and Speed
//
//------------------------------------------------------------------------------
Bool Atmosphere::GetWindDirVel (SLong alt, SWord& hdg, SWord& speed)
{
	if(!Save_Data.flightdifficulty [FD_WINDEFFECTS])
	{
		hdg = 0;
		speed = 0;
		return(TRUE);
	}

	if(alt < 1036300)	// 34,000 ft 
	{
		hdg	  = SWord(dir0 * 182.04);
		speed = 0;//wind0;
		return(TRUE);
	}
	
	if(alt > 1066800)	// 35,000 ft
	{
		hdg	  = SWord(diralt * 182.04);
		speed = (51 * windalt) / 10;
		return(TRUE);
	}
	
	SWord Dir0   = SWord(182.04 * dir0);
	SWord DirAlt = SWord(182.04 * diralt);

	SWord DeltaDir = DirAlt - Dir0;
	FP Fract = (alt - 1036300.0) / (1066800.0 - 1036300.0);
	FP fdir   = Dir0 + Fract * DeltaDir;
	FP fspeed = Fract * windalt * 5.1;
	hdg   = SWord(fdir);
	speed = SWord(fspeed);
	
	return(TRUE);
}

//------------------------------------------------------------------------------
//Procedure		GetWind
//Author		Andrew McRae
//Date			Thu 10 Apr 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------

// for contrails
Bool Atmosphere::GetWind (SLong height, SLong& x, SLong& y, SLong& z)
{
	if (Save_Data.flightdifficulty [FD_WINDEFFECTS])
	{
		SWord dir, vel;
		GetWindDirVel(height, dir, vel);
		if(!vel)
		{
			x = y = z = 0;
			return(TRUE);
		}

		SWord _sin, _cos;
		Math_Lib.high_sin_cos((Angles)dir, _sin, _cos);
		x = (-vel * _sin) / ANGLES_FRACT;
		y = 0;
		z = (-vel * _cos) / ANGLES_FRACT;
	}
	else
	{
		x = 0;
		y = 0;
		z = 0;
	}
	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		GetWind
//Author		Andrew McRae   
//Date			Thu 2 Apr 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool Atmosphere::GetWind (AirStrucPtr const ControlledAC, FCRD& wind, FP height)
{
	PMODEL pModel = ControlledAC->fly.pModel;

	if (Save_Data.flightdifficulty [FD_WINDEFFECTS])
	{
		SLong wx, wy ,wz;
		GetWind(pModel->Pos.y, wx, wy, wz);	//CSB 10/06/99	
		wind.x = wx * 0.1;
		wind.y = wy * 0.1;
		wind.z = wz * 0.1;
	}
	else
		NullVec (wind);

	if (Save_Data.flightdifficulty [FD_WINDGUSTS])
	{
		if (pModel->Gust.ACTIVE)
		{
			wind.x += pModel->Gust.Vec.x;		
			wind.y += pModel->Gust.Vec.y;		
			wind.z += pModel->Gust.Vec.z;

			// terminate ??
			pModel->Gust.Time -= (SWord)Timer_Code.FIXEDFRAMETIME;
			if (pModel->Gust.Time <= 0)
			{
				pModel->Gust.ACTIVE = FALSE;
				pModel->Gust.Time = 0;
				NullVec(pModel->Gust.Vec);
			}
		}
		else
		{
		// start a new gust ??
			if ((UWord)Math_Lib.rnd () >= Frequency)
			{
				// make new gust
				// magnitude and direction
				FP magscale = ((FP)(UWord)Math_Lib.rnd() / 65536);
				FP mag = MinMagnitude + ((MaxMagnitude - MinMagnitude) * magscale);

				pModel->Gust.Vec.x = mag * ((FP)Math_Lib.rnd() - 32768) / 32768;	//CSB 23/12/98
				pModel->Gust.Vec.y = mag * ((FP)Math_Lib.rnd() - 32768) / 32768;	//CSB 23/12/98
				pModel->Gust.Vec.z = mag * ((FP)Math_Lib.rnd() - 32768) / 32768;	//CSB 23/12/98
				// duration
				pModel->Gust.Time = MinDuration +
					(SWord)(((ULong)(MaxDuration - MinDuration) * (ULong)Math_Lib.rnd()) >> 16);
				// set active
				pModel->Gust.ACTIVE = TRUE;
			}
		}
	}

	return TRUE;
}


//------------------------------------------------------------------------------
//Procedure		SetGusts
//Author		Andrew McRae
//Date			Thu 10 Apr 1997
//
//Description	Initialise Atmosphere Gusts
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------ 
void Atmosphere::SetGusts (SWord mindur, SWord maxdur, FP minmag, FP maxmag, UWord freq)
{
	MinDuration = mindur;
	MaxDuration = maxdur;
	MinMagnitude = minmag;
	MaxMagnitude = maxmag;
	Frequency = freq;
}

//------------------------------------------------------------------------------
//Procedure		SetMissionDawnDusk
//Author		Craig Beeston
//Date			Tue 15 Dec 1998
//
//Description	Sets the dawn and dusk times based on the lattitude and the date
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------ 
void Atmosphere::SetMissionDawnDusk (SWord Latitude, ULong Date, SLong& Dawn, SLong& Dusk)
{
	ULong SecsPerDay = 24 * 60 * 60;
	UWord Hours6 = SecsPerDay / 4;

	ULong SecsPerYear = 365 * SecsPerDay;
	while(Date > SecsPerYear) Date -= SecsPerYear;

	FP DeltaTime = 2 * FPIE * ((float)Date + 9.5 * (float)SecsPerDay) / (float)SecsPerYear;
	DeltaTime  = FCos(DeltaTime);
	DeltaTime *= (90 - 23);
	DeltaTime /= 90;
	DeltaTime *= FSin(Degs2Rads(Latitude));
	DeltaTime /= FCos(Degs2Rads(Latitude));
	DeltaTime *= FSin(Degs2Rads(23));
	DeltaTime *= Hours6;
	DeltaTime += Hours6;

	FP fDawn = DeltaTime;
	FP fDusk = SecsPerDay - fDawn;

	if(fDawn < 0)
	{
		fDawn = 0;
		fDusk = SecsPerDay;
	}

	if(fDusk > SecsPerDay)
	{
		fDawn = SecsPerDay / 2;
		fDusk = SecsPerDay / 2;
	}
	Dawn = (int)fDawn;
	Dusk = (int)fDusk;
}

//------------------------------------------------------------------------------
//Procedure		Contrails
//Author		Andrew McRae
//Date			Thu 2 Apr 1998
//
//Description	duration in csecs
//				The centre of the contrail layer is set for all aircraft using SetContrails

//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool Atmosphere::Contrails (AirStrucPtr ac, ULong& duration)
{
	Bool	isactive = FALSE;

	duration = 0;

	if (ac && (ac->Status.size == AirStrucSize))								//RJS 08Apr98
	{
		PMODEL pModel = ac->fly.pModel;
		if (pModel)
		{
			FP deviation = (pModel->AmbTemp - 220) * 1500;
			deviation = 0;	//CSB 11/06/99	

			FP MinHeight = ContrailBandCentre - (ContrailBandWidth * 0.5) + deviation;//DAW 28Apr98
			FP MaxHeight = ContrailBandCentre + (ContrailBandWidth * 0.5) - deviation;//DAW 28Apr98


			if	((ac->World.Y <= MaxHeight) && (ac->World.Y >= MinHeight))
			{
				FP time =  (ac->World.Y - MinHeight) * (MaxHeight - ac->World.Y);
				time 	*= 0.000002691 * pModel->ContrailDuration;

				if (time > MAXIMUM_CONTRAIL_DURATION)
					time = MAXIMUM_CONTRAIL_DURATION;

				duration = (ULong)time;
				isactive = TRUE;
			}
		}
	}

	return isactive;
}
