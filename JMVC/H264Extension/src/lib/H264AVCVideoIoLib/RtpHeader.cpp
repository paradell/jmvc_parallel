#include <cstdio>
#include <stdlib.h>
#include "H264AVCVideoIoLib.h"
#include "RtpHeader.h"

//#include "net_formats.h"

RtpHeader::RtpHeader(){};


RtpHeader::~RtpHeader()
{
}

ErrVal
RtpHeader::create( RtpHeader*& rpcRtpHeader ){
	RtpHeader* pcRtpHeader;

   pcRtpHeader = new RtpHeader;
   rpcRtpHeader = pcRtpHeader;
	//printf("RtpHeader::create()\n");  
	return Err::m_nOK;
}
 

ErrVal
RtpHeader::destroy(){
	//printf("RtpHeader::destroy()\n");  
	return Err::m_nOK;
}

ErrVal
RtpHeader::init(){

  rtpHeader.header1 = 0x80;//Valor que pot variar de 0x80 si no hi ha padding o 0xA0 si sí que n'hi ha
  /*char version=2; //versió actual del RTP
  bool padding = 0;
  bool special_header = 0; // si que n'hi ha
  char CC =0; //hi ha una sola font de sincro / data*/

  rtpHeader.header2 = 0x60; //8 bits including -> MPPPPPPP where M = markerbit and P=payload type
  //bool Markerbit; (1 si hi ha fragmentació. 0 si no ho és.)
  //char payload type; //no payload type is set. From 96(0x60) - 127(0x80) -> dymanic RTP type

  rtpHeader.Sequence_num=3434; //hauria de ser aleatori

  rtpHeader.timestamp=2121; //l'agafem de 
  rtpHeader.ssrc=6565; //numero aleatori

  for(int i=0;i<12;i++)
	  header_string[i]=0x00;
  
  //capçalera completa RTP per H264
  setRTPHeader(false,false);

  //printf("Valor Seq Number: %d\n",rtpHeader.Sequence_num);
  //printf("Valor Timestamp: %d\n",rtpHeader.timestamp);
  //increaseSequenceNumber();
  //increaseTimestamp(160);
  //printf("Valor Seq Number: %d\n",rtpHeader.Sequence_num);
  //printf("Valor Timestamp: %d\n",rtpHeader.timestamp);

setRTPHeader(false,false);

//printf("La Header que queda es: %s\n",getRTPHeader());  





	return Err::m_nOK;
 }

ErrVal
RtpHeader::uninit(){
//	printf("RtpHeader::uninit()\n");  
	return Err::m_nOK;
 }

ErrVal
RtpHeader::setRTPHeader(bool padding, bool markerbit){
	//Concatenar els bits de header

	//passem màscares de Markerbit i del bit de Padding
	
	if(padding)
		rtpHeader.header1 |= 0x20; //Posem a 1 el bit de Padding
	else
		rtpHeader.header1 &= ~0x20; //Posem a 0 el bit de Padding

	if(markerbit)
		rtpHeader.header2 |= 0x80; //Posem a 1 el bit de Marker
	else
		rtpHeader.header2 &= ~0x80; //Posem a 0 el bit de Marker

	//printf("Valors de les capçaleres:\n header1=%X\n header2=%X\n Seq number =%X\n timestamp =%X\n ssrc=%X\n\n",rtpHeader.header1,rtpHeader.header2,rtpHeader.Sequence_num,rtpHeader.timestamp,rtpHeader.ssrc);
	//printf("queda una cadena de ");
	
	//concatenem
	xJoinRTPHEader();



	/*printf("Valor de la header concatenada:\n");
	for(int j=0;j<12;j++)
		printf(" %X ",(UChar)header_string[j]);
	printf("\n");*/
	//printf("%s\n", int_to_binary_string(header_string));*/
	//system("pause");

	//transformar en char*
	//printf("RtpHeader::setRTPHeader()\n");  
	  return Err::m_nOK;
  }

UChar *
RtpHeader::getRTPHeader(){
	
	return header_string;
	//return "test";
  }
UInt
RtpHeader::getTimestamp(){
	return rtpHeader.timestamp;
}

UInt
RtpHeader::getSequenceNumber(){
	return rtpHeader.Sequence_num;
}

ErrVal
RtpHeader::xJoinRTPHEader(){

	header_string[0] = rtpHeader.header1;
    header_string[1] = (char)(rtpHeader.header2);
	header_string[2] =((char) ((rtpHeader.Sequence_num & 0xFF00) >> 8));
    header_string[3] =((char) (rtpHeader.Sequence_num & 0x00FF));

	header_string[4] =((char) ((rtpHeader.timestamp & 0xFF000000) >> 24));
    header_string[5] =((char) ((rtpHeader.timestamp & 0x00FF0000) >> 16));
    header_string[6] =((char) ((rtpHeader.timestamp & 0x0000FF00) >> 8));
    header_string[7] =((char) ((rtpHeader.timestamp & 0x000000FF)));

    header_string[8] =((char) ((rtpHeader.ssrc & 0xFF000000) >> 24));
    header_string[9] =((char) ((rtpHeader.ssrc & 0x00FF0000) >> 16));
    header_string[10] =((char) ((rtpHeader.ssrc & 0x0000FF00) >> 8));
    header_string[11] =((char) ((rtpHeader.ssrc & 0x000000FF)));
	
	return Err::m_nOK;
}



ErrVal
RtpHeader::increaseTimestamp(int period){
	rtpHeader.timestamp+=period;
	xJoinRTPHEader();
	return Err::m_nOK;
}

ErrVal
RtpHeader::increaseSequenceNumber(){
	rtpHeader.Sequence_num++;
	xJoinRTPHEader();
	return Err::m_nOK;
}