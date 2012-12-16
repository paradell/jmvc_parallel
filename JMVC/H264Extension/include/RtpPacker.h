#if !defined(AFX_RTPPACKER_H__CBFF413E_29A2_B38C_97F1_44E35C83507D__INCLUDED_)
#define AFX_RTPPACKER_H__CBFF413E_29A2_B38C_97F1_44E35C83507D__INCLUDED_

#include <sys/stat.h>
#include <sys/types.h>
//#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <stdint.h>
#include "RtpHeader.h"
#include "UDPController.h"
//#include "UDPBoost.h"
#include "RtpBuffer.h"
#include "MVCBuffer.h"
//#include <boost/thread.hpp>

#define MAX_VIEWS 8

#define HEADER_SIZE 12
#define INET_HEADER_SIZE 28
#define NAL_HEADER_SIZE 4
#define NAL_FRAG_HEADER_SIZE 5
#define MTU 1500
#define DATA_SIZE MTU-44
//#define DATA_SIZE 1400

#define FRAGMENTED_TYPE 28

#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif

typedef struct{
	char start_bit; //start bit -> indica si porta el primer fragment
	char end_bit; //end bit -> indica si porta el ultim fragment
	char reserved_bit; //reserved bit -> si el valor es 0, serà ignorat
	UChar type; //Ha de dur el valor type de la NAL genèrica i posar valor 28 al valor type de la NAL generica
}fragment_NAL_unit;

typedef struct{
	char forbidden_bit; // bit -> Sempre a 0
	char nri_bits; //
	UChar type; //Valor de la NAL, si és de fragmentació, es posa a 28
}NAL_unit;


class H264AVCVIDEOIOLIB_API RtpPacker
{

protected:
	RtpPacker();
	virtual ~RtpPacker();

	UDPController*	m_apcUDPController;
	RtpBuffer*	m_apcRtpBuffer;
	MVCBuffer* m_apcMVCBuffer;

public:
  static ErrVal create( RtpPacker*& RtpPacker );
  ErrVal destroy();


  ErrVal init(char* confAdress, int confPort,bool Encoder);
  ErrVal uninit();

  ErrVal packInit(UChar * data, int size);  //Empaqueta paquets que no facin referència a MVC. L'utilitzem per a les dades de capçalera 264
  ErrVal pack(UChar * nal_unit, UChar * data, int size); //Empaqueta dades NAL amb MVC
  ErrVal unpack(); //Desempaqueta el primer paquet del buffer de RTP

  ErrVal endTransmission(); //

  ErrVal increaseTimeStamp();
  ErrVal increaseSeqNum();

  ErrVal setPeriod(int freq);

  ErrVal recieveRTP(); //Rep i processa els paquets RTP que rep
  DecoderElement popMVCelement(); //Treu un element de la cua de MVC
  UChar* popMVCdata(); //Treu només les dades del paquet de la cua MVC
  UInt packetSize();

  int test();
    
  bool isMvcBufferEmpty();
  bool isRtpBufferEmpty();
  bool isRtpBufferFull();

private:

  UChar dataRTP[MTU];

  int fragment_index;
  bool fragmented;

  bool first_view;

  fragment_NAL_unit pcFragmentNALUnit;
  NAL_unit pcStandardNALUnit;

  UChar  pcNALFragUnit;
  UChar  pcNALUnit;

  RtpHeader pcRtpHeader;
  UChar nal_unit_fragmented[NAL_FRAG_HEADER_SIZE];
  UChar rtpData[2*MTU];
  int rtpPacketSize;

  int period;

  UChar * xgetRTPHeader();
  ErrVal xsetRTPHeader(bool padding, bool markerbit);
   

  int xInsertHeader(UChar * data, int size);
  int xInsertData(UChar * data, int size);
  int xInsertData(UChar * nal_unit, UChar * data, int size);
  
  ErrVal xRTPSingle(UChar * nal_unit,UChar * data, int size);
  ErrVal xRTPnotMVC(UChar * data, int size);

  ErrVal xFragment(UChar * nal_unit,UChar * data, int FullSize); //En el cas que el paquet a enviar sigui superior a la MTU, el fragmenta
  ErrVal xDefragment(); //Ajunta fragments rebuts i els processa .

  ErrVal xEditFragmentNAL(UChar * nal_unit);
  ErrVal xSetStandardNAL(UChar nal_unit);
  ErrVal xSetFragmentNAL(UChar nal_unit);

  UChar xgetNalUnit(); //
  UChar xgetNalFragUnit();


};

#endif