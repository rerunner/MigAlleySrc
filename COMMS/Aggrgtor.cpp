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
//Filename       aggrgtor.cpp
//System         
//Author         Andy McMaster
//Date           Wed 29 Apr 1998
//Description    
//------------------------------------------------------------------------------
//#include "afx.h" //RERUN
#include	"aggrgtor.h"
//RERUN#include	"stdio.h"
#include	"comms.h"
#include	"stub3d.h"

extern void	AggTracer(char* str);

//------------------------------------------------------------------------------
//Procedure		AggregatorGetPackets
//Author		Andy McMaster
//Date			Thu 20 Mar 1997
//
//
//Inputs		
//
//
//------------------------------------------------------------------------------
void Aggregator::AggregatorGetPackets(Bool reset, Bool timercall)
{
	LPACPACKET RecPacket;
 	HRESULT	res;
	ULong	n,m;
	myDPID	from;
	DWORD	BufferLen;
	char* Buffer;
	bool	SendResult;

	static Bool	buffer=FALSE;	// are all players synched. if so buffer packets
	static int time=0;				// number of calls by timer, use to determine timeout
	static int clearbuffertime=0;	// try to clear buffers by sending extra agg packets every other agg frame
	static AGGSENDPACKET allpackets;	// aggregated packet

	if (reset || ResetFlag)
	{
// clear all packets for me

		while (ReceiveMessage(Buffer,BufferLen,from)==DP_OK)
		{
		}

		ResendStatus=0;
		ResendPacket=0;
		InitAggregator();
		InitReservePackets();
		InitAggArrayBuffer(&allpackets);
		ResetAcks();
		time=0;
		clearbuffertime=0;
		buffer=FALSE;
		ResetFlag=false;
	}
	else
	if (Running)
	{
// calls from timer are every 1/50 sec. Use to determine timeout
		if (timercall)
		{
			time++;
			clearbuffertime++;
		}
		BufferLen=2048;
// get all packets from Q
		do
		{
			res=ReceiveMessage(Buffer,BufferLen,from);

			if (res==DP_OK)
			{
				if (BufferLen==sizeof(ACPACKET))
				{
					for (n=0;n<MAXPLAYERS;n++)
					{
						if (DPlay::H2H_Player[n].dpid==from)
							break;
					}

					if (n!=MAXPLAYERS)
					{
// got a delta packet from player
						RecPacket=(LPACPACKET)Buffer;
// check for ack'ed packets 
						if (RecPacket->IDCode!=PIDC_DUMMYPACKET)
						{
							CheckAck(RecPacket->Ack1,RecPacket->Count,n);
							CheckAck(RecPacket->Ack2,RecPacket->Count,n);
							UpdateAckBuffer();
						}
// put packet in buffer
// check packets in buffer and only add to buffer if this packet number 
// has not already been received - should get rid of huge number of resent packets problem...
						Bool add=TRUE;
						ULong p=0;
						if (ReservePackets[n].NumEntries())
						{
							ReservePackets[n].SetTempCurr();
							do
							{
								if (ReservePackets[n].GetTemp()->Count==RecPacket->Count
								&& ReservePackets[n].GetTemp()->IDCode!=PIDC_PACKETERROR)
								{
									add=FALSE;
									p++;
									break;
								}
								ReservePackets[n].UpdateTemp();
							}
							while (ReservePackets[n].GetTemp()!=ReservePackets[n].GetNext());
						}

						if (add)
						{
// if reserves are full then overwrite oldest packet. If this causes a problem then
// a player will have to initiate a resynch

							if (ReservePackets[n].NumEntries()==BUFFERLENGTH)
							{
								ReservePackets[n].GetCurr()->IDCode=PIDC_PACKETERROR;
								ReservePackets[n].UpdateCurr();
							}

							_DPlay.CopyBuffer((char*)ReservePackets[n].GetNext(),Buffer,BufferLen);
							ReservePackets[n].AddEntryAndUpdateNext();
						}
					}
					else
					{
// who cares - not a player
					}
				}
				else
				{
// who cares - not a delta packet
// need to check for resync init
					if (from==0)
					{
// ignore system message
					}
					else
					{
						PCOMMONDATA pack=(PCOMMONDATA)Buffer;
						if (pack->PacketID==PID_NEEDRESYNC)
						{
// resync has occurred. Stop aggregator and reset (just in case reset doesnt arrive!!)
// clear all packets for me
							while (ReceiveMessage(Buffer,BufferLen,from)==DP_OK)
							{
							}

							buffer=FALSE;
							InitAggregator();
							InitReservePackets();
							InitAggArrayBuffer(&allpackets);
							ResetAcks();
							time=0;
						}
						else
						{
// ignore non resync 
						}
					}
				}
			}
		}
		while (res==DP_OK);

// OK, see who we need a packet from and check buffer
		ULong oldrecbits;
		do
		{
			for (n=0;n<MAXPLAYERS;n++)
			{
				if (!(RecBits&(1<<n)) && ReservePackets[n].NumEntries()>0)
				{
	// if we havent got a packet for this player yet and there is one in buffer......
					RecPacket=ReservePackets[n].GetCurr();
	// wahay, a needed packet...maybe
					if (!buffer)
					{
	// still require dummy/init packets in frame order
						Bool gotOK=FALSE;
						do
						{
							RecPacket=ReservePackets[n].GetCurr();
							if (RecPacket->Count==AggFrameCount)
							{
	// if we have found packet for this frame copy and continue
								RecBits|=(1<<n);
								_DPlay.CopyBuffer((char*)&allpackets.player[n],(char*)RecPacket,sizeof(ACPACKET));
								gotOK=TRUE;
							}
							else
							{
	// if this packet is wrong check buffer until dummy/init packets are exhausted
								ReservePackets[n].SetTempCurr();
								ReservePackets[n].UpdateTemp();

								if (ReservePackets[n].GetTemp()->IDCode==PIDC_DUMMYPACKET
								|| ReservePackets[n].GetTemp()->IDCode==PIDC_INITPACK)
								{
									ReservePackets[n].GetCurr()->IDCode=PIDC_PACKETERROR;
									ReservePackets[n].UpdateCurr();
								}
								else
								{
	// no more dummy packets
									gotOK=TRUE;
								}
							}
						}
						while (!gotOK);
					}
					else
					{
	// we are in 'real packet mode' dont care what frame count on packet is......
						RecBits|=(1<<n);
						_DPlay.CopyBuffer((char*)&allpackets.player[n],(char*)RecPacket,sizeof(ACPACKET));
					}
					ReservePackets[n].GetCurr()->IDCode=PIDC_PACKETERROR;
					ReservePackets[n].UpdateCurr();
				}
			}
	// is it time to send packet
			oldrecbits=RecBits;
//if a player drops out then for this frame recbits may have that players
//bit set. If use >= then this doesnt matter!
// NO. need to be equal, its bits set not number of players
			if (time==_DPlay.RateDivider || RecBits==_DPlay.ActivePlayers)
			{
				HRESULT res;
				if (time==_DPlay.RateDivider)
				{
					time=0;
				}
				allpackets.Count=AggFrameCount;
				if (buffer)
				{
	// hey, if we're already synched dont send any agregated packets that are empty....
					if (RecBits!=0)
					{
						SendResult=MakeAndSendPacket(&allpackets,buffer);
						SetAcks(&allpackets);
						AggBuffer.AddEntryAndUpdateNext();
					}
					else
					{
						// EMPTY AGG PACKET
					}
				}
				else
				{
	// not synched yet, gotta send every 1/50th - even empty packets
					SendResult=MakeAndSendPacket(&allpackets,buffer);
					m=0;
					for (n=0;n<MAXPLAYERS;n++)
					{
						if (allpackets.player[n].IDCode==PIDC_INITPACK)
						{
							m++;
						}
 					}

					if (m==_DPlay.CurrPlayers)
					{
						buffer=TRUE;
					}
				}

				RecBits=0;
				AggFrameCount++;
// when all players are synched and acking packets, treat AggFrame 0 as an empty packet
// so that when a player sends ack 0 it doesnt update buffers
				if (buffer && !AggFrameCount)
					AggFrameCount=1;

				InitAggArrayBuffer(&allpackets);
			}
		}
		while (oldrecbits==_DPlay.ActivePlayers && buffer);

		if (buffer)
		{
// only resend packets or clear buffers if we are REAL game mode, not whilst
// in resync!!!!!!!!!!!!!
	// took out to get 8 player network going
	// determine if resend is necessary 
			ULong num=AggBuffer.NumEntries();
			if (ResendPacket!=AggBuffer.GetCurr()->aggpack.Count)
			{
				ResendPacket=AggBuffer.GetCurr()->aggpack.Count;
				ResendStatus=0;
			}
			if ((num>BUFFERLENGTH/4 && ResendStatus==0)
			|| (num>BUFFERLENGTH/2 && ResendStatus<2))	 
			{
				ResendStatus++;
				HRESULT res;
				SendResult=ResendAggPacket();
			}
			else if (clearbuffertime==((_DPlay.RateDivider*2)-(_DPlay.RateDivider/2)))
			{
	// in between every other agg packet
	// if not resending a packet check if buffers have any entries and if so try 
	// and clear by sending an extra packet a frame.
				for (n=0;n<MAXPLAYERS;n++)
				{
			 		if (ReservePackets[n].NumEntries() && !(RecBits&1<<n))
					{
						RecPacket=ReservePackets[n].GetCurr();
						_DPlay.CopyBuffer((char*)&allpackets.player[n],(char*)RecPacket,sizeof(ACPACKET));
						RecBits|=1<<n;
						ReservePackets[n].GetCurr()->IDCode=PIDC_PACKETERROR;
						ReservePackets[n].UpdateCurr();
					}
				}
				if (RecBits)
				{
					allpackets.Count=AggFrameCount;
					SendResult=MakeAndSendPacket(&allpackets,buffer);

					AggFrameCount++;							//AMM 12Jan00
					if (!AggFrameCount)							//AMM 12Jan00
						AggFrameCount=1;						//AMM 12Jan00

					RecBits=0;
					if (AggBuffer.NumEntries()==BUFFERLENGTH)
					{
						ResetAcks();
						AggBuffer.UpdateCurr();
						UpdateAckBuffer();
					}

					SetAcks(&allpackets);
					AggBuffer.AddEntryAndUpdateNext();
					InitAggArrayBuffer(&allpackets);
				}
			}
		}
	}
	else
	{
// receive and junk all messages to aggregator as we are not ready for them yet!!!!
		while (ReceiveMessage(Buffer,BufferLen,from)==DP_OK)
		{
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		InitAggArrayBuffer
//Author		Andy McMaster
//Date			Tue 25 Mar 1997
//
//Description	Initialises the aggregators array buffer to error values
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Aggregator::InitAggArrayBuffer(LPAGGSENDPACKET pack)
{
	ULong n;

	for(n=0;n<MAXPLAYERS;n++)
	{
		memset(&pack->player[n],0,sizeof(ACPACKET));
		pack->player[n].IDCode=PIDC_PACKETERROR;
	}
}


//------------------------------------------------------------------------------
//Procedure		ServerGetPackets
//Author		Andy McMaster
//Date			Mon 9 Feb 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Aggregator::ServerGetPackets(Bool reset)
{
//Deadcode
}

//------------------------------------------------------------------------------
//Procedure		SetAcks
//Author		Andy McMaster
//Date			Wed 14 Jan 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Aggregator::SetAcks(LPAGGSENDPACKET packet)
{
	for (ULong n=0;n<MAXPLAYERS;n++)
	{
// want acks from all active players for all packets thankyouverymuch
			if (_DPlay.ActivePlayers&1<<n)
				AggBuffer.GetNext()->Acks[n]=FALSE;
			else
				AggBuffer.GetNext()->Acks[n]=TRUE;
	}
}

//------------------------------------------------------------------------------
//Procedure		CreateAggregator
//Author		Andy McMaster
//Date			Mon 9 Feb 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Aggregator::CreateAggregator()
{
//Deadcode 
	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		RunAggregator
//Author		Andy McMaster
//Date			Mon 9 Feb 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Aggregator::RunAggregator()
{
//Deadcode
}

//------------------------------------------------------------------------------
//Procedure		InitAggregator
//Author		Andy McMaster
//Date			Wed 29 Apr 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Aggregator::InitAggregator()
{
	ULong n;
	AggFrameCount=0;										

	AggBuffer.InitBuffer();
	AggBuffer.SetTempCurr();
	do
	{
		for (n=0;n<MAXPLAYERS;n++)
		{
			AggBuffer.GetTemp()->Acks[n]=FALSE;
		}

		AggBuffer.UpdateTemp();
	}
	while (AggBuffer.GetTemp()!=AggBuffer.GetCurr());
}

//------------------------------------------------------------------------------
//Procedure		StopResetAggregator
//Author		Andy McMaster
//Date			Mon 15 Sep 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Aggregator::StopResetAggregator()
{
	ResetFlag=TRUE;
}

//------------------------------------------------------------------------------
//Procedure		ResetAcks
//Author		Andy McMaster
//Date			Wed 15 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Aggregator::ResetAcks()
{
	ULong n;

	for (n=0;n<MAXPLAYERS;n++)
	{
		AggBuffer.GetCurr()->Acks[n]=FALSE;
	}
}

//------------------------------------------------------------------------------
//Procedure		CheckAck
//Author		Andy McMaster
//Date			Thu 13 Aug 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Aggregator::CheckAck(UByte val, UByte count,UByte pos)
{
	LPAGGBUFF temp;

	if (val!=0)
	{
		AggBuffer.SetTempCurr();
		while (AggBuffer.GetTemp()!=AggBuffer.GetNext())
		{
			temp=AggBuffer.GetTemp();
			if (temp->aggpack.Count==val)
			{
				temp->Acks[pos]=TRUE;
				break;
			}
			AggBuffer.UpdateTemp();
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		UpdateAckBuffer
//Author		Andy McMaster
//Date			Thu 20 Aug 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Aggregator::UpdateAckBuffer()
{
	Bool update=TRUE;
	ULong n;

	while (update)
	{
		update=TRUE;

		for (n=0;n<MAXPLAYERS;n++)
		{
			if (AggBuffer.GetCurr()->Acks[n]==FALSE)
			{
				update=FALSE;
				break;
			}
		}

		if (update)
		{
			ResetAcks();
			AggBuffer.UpdateCurr();
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		InitReservePackets
//Author		Andy McMaster
//Date			Mon 17 Aug 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Aggregator::InitReservePackets()
{
	ULong n;

	for (n=0;n<MAXPLAYERS;n++)
	{
		ReservePackets[n].InitBuffer();
		ReservePackets[n].SetTempCurr();
		do 
		{
			ReservePackets[n].GetTemp()->IDCode=PIDC_PACKETERROR;//AMM 28Jul99
			ReservePackets[n].UpdateTemp();
		}
		while (ReservePackets[n].GetTemp()!=ReservePackets[n].GetCurr());
	}
}

UByte	Aggregator::GetAggFrameCount()
{
	return AggFrameCount;
}

//------------------------------------------------------------------------------
//Procedure		ReceiveMessage
//Author		Andy McMaster
//Date			Thu 13 May 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
HRESULT	Aggregator::ReceiveMessage(char*&buf,ULong& buflen,ULong& from)
{
	HRESULT res;

	ULong to=_DPlay.aggID,temp;
	from=0;

	while (true)
	{
		temp=AggPacketLength;
		res=_DPlay.lpDP4->Receive(&from,&to,DPRECEIVE_TOPLAYER,AggPacketPtr,&temp);

		if (res==DPERR_BUFFERTOOSMALL)
		{
			delete [] AggPacketPtr;
			AggPacketPtr=new char[temp];
			AggPacketLength=temp;
			if (!AggPacketPtr)
				_Error.EmitSysErr("Out of memory");
		}
		else if (res==DP_OK)
		{
			buf=AggPacketPtr;
			buflen=temp;
			return res;
		}
		else break;
	}
	return res;
}

bool Aggregator::MakeAndSendPacket(LPAGGSENDPACKET packet, Bool buffer)
{
	ULong NumPlayers=0, n, PacketSize;
	bool ok;
	HRESULT res;
	AGGSENDPACKET p;
	UByteP TempPtr=(UByteP)&p;

	PacketSize=sizeof(UByte); // count
	PacketSize+=sizeof(ULong); // player in packet

	*((ULong*)TempPtr)=RecBits;
	TempPtr+=sizeof(ULong);
	*TempPtr=AggFrameCount;
	TempPtr+=sizeof(UByte);

// start filling in players packets
	for (n=0; n<MAXPLAYERS; n++)
	{
		if (RecBits & 1<<n)
		{
// copy player from packet to Packet
			memcpy(TempPtr,&packet->player[n],sizeof(ACPACKET));
			TempPtr+=sizeof(ACPACKET);
			PacketSize+=sizeof(ACPACKET);
			NumPlayers++;
		}
	}

	res=_DPlay.lpDP4->SendEx(_DPlay.aggID,_DPlay.playergroupID,DPSEND_ASYNC|DPSEND_NOSENDCOMPLETEMSG ,&p,PacketSize,0,0,NULL,NULL);

	//RERUN if (!res || res==DPERR_PENDING)
	if ((res == 0) || res == DPERR_PENDING) //RERUN
		ok=true;
	else
		ok=false;

// copy packet into buffer if not in sync phase
	if (buffer)
	{
// if we have filled up sent buffer then overwrite last entry.
// If packet has really not been received by a player then it will cause them to 
// initaite a resynch quite soon....!!!!

		if (AggBuffer.NumEntries()==BUFFERLENGTH)
		{
			ResetAcks();
			AggBuffer.UpdateCurr();
			UpdateAckBuffer();
		}

		AggBuffer.GetNext()->NumPlayersInPacket=NumPlayers;

		memcpy((char*)&AggBuffer.GetNext()->aggpack,(char*)&p,sizeof(AGGSENDPACKET));
	}

	return ok;
}

bool Aggregator::ResendAggPacket()
{
	ULong PacketSize;
	HRESULT res;
	bool	ok;

	PacketSize=sizeof(ULong);
	PacketSize+=sizeof(UByte);
	PacketSize+=sizeof(ACPACKET) * AggBuffer.GetCurr()->NumPlayersInPacket;

	res=_DPlay.lpDP4->SendEx(_DPlay.aggID,_DPlay.playergroupID,DPSEND_ASYNC|DPSEND_NOSENDCOMPLETEMSG ,&((AggBuffer.GetCurr())->aggpack),PacketSize,0,0,NULL,NULL);

	//RERUN if (!res || res==DPERR_PENDING)
	if ((res == 0) || res == DPERR_PENDING) //RERUN
		ok=true;
	else
		ok=false;

	return ok;
}

CON		Aggregator::Aggregator()
{
 	AggFrameCount=0;
	RecBits = 0;
	Running=FALSE;
	ResetFlag=false;
	ResendStatus=0;
	ResendPacket=0;
	AggPacketPtr=NULL;
	AggPacketLength=0;
}

CON		Aggregator::~Aggregator()
{
	delete [] AggPacketPtr;
	AggPacketPtr=NULL;
	AggPacketLength=0;
}
