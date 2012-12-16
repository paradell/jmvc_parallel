#if !defined(AFX_RTPHEADER_H__CBFF413E_29A2_B38C_97F1_44E35C83507D__INCLUDED_)
#define AFX_RTPHEADER_H__CBFF413E_29A2_B38C_97F1_44E35C83507D__INCLUDED_

#include <sys/stat.h>
#include <sys/types.h>
//#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <stdint.h>

#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif

typedef struct {
	UChar header1; //header1=0x90; //Valor que pot variar de 0x90 si no hi ha padding o 0xB0 si sí que n'hi ha
  /*char version=2; //versió actual del RTP
  bool padding = X; //1 si conté padding, 0 si no
  bool special_header = 1; // si que n'hi ha
  char CC =0; //hi ha una sola font de sincro / data*/
	UChar header2; //8 bits including -> MPPPPPPP where M = markerbit and P=payload type
  //bool Markerbit; (1 si el el ultim paquet del mateix timestamps. 0 si no ho és.)
  //char payload type; //no payload type is set. From 96(0x60) - 127(0x80) -> dymanic RTP type
	UShort Sequence_num; //hauria de ser aleatori
	UInt timestamp; //l'agafem de 
	UInt ssrc; //numero aleatori
}rtp_4175base_header;


class H264AVCVIDEOIOLIB_API RtpHeader
{
protected:
	
public:

	RtpHeader();
	virtual ~RtpHeader();

  static ErrVal create( RtpHeader*& RtpHeader );
  ErrVal destroy();

  ErrVal init();
  ErrVal uninit();

  UChar * getRTPHeader();
  ErrVal setRTPHeader(bool padding, bool markerbit);

  ErrVal increaseTimestamp(int period);
  ErrVal increaseSequenceNumber();

  UInt getTimestamp();
  UInt getSequenceNumber();


private:
	

   //capçalera completa RTP per H264
  rtp_4175base_header rtpHeader; //12 bytes de tamany
  UChar header_string[12]; //variable de 12*8=96 bits
  

	ErrVal xJoinRTPHEader(); //Agafa les dades que tnim del tipus rtp_4175base_header i les passem a UChar header_string[12]
 
 
};
#endif