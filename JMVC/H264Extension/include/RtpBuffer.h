#if !defined(AFX_RTPBUFFER_H__CBFF413E_29A2_B38C_97F1_44E35C83507D__INCLUDED_)
#define AFX_RTPBUFFER_H__CBFF413E_29A2_B38C_97F1_44E35C83507D__INCLUDED_


#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_ELEMENTS 500
#define DATA_ELEMENT_SIZE 1500


#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif

typedef struct{
	UChar data[DATA_ELEMENT_SIZE];
	UInt size;
	bool MarkerBit;
	UShort Sequence_num; 
	UInt timestamp; 
}Element; 

typedef struct{
	int start;
	int end;
	int elements;
	Element element[MAX_ELEMENTS];
}Buffer;


class H264AVCVIDEOIOLIB_API RtpBuffer
{
private:
	UShort lastPoppedSeqNum;
	Buffer buffer;
	ErrVal refreshElements();
	ErrVal resetElement(Element* element);
	Bool OutOfRange(int pos);
	
	Element InputTest(int pos);

	bool	checkFrag(UChar NAL);

	bool	checkStartBit(UChar NALfrag);

	bool	checkEndBit(UChar NALfrag);

protected:
	RtpBuffer();
	virtual ~RtpBuffer();
	
public:

  static ErrVal create( RtpBuffer*& rpcRtpBuffer );
  ErrVal destroy();

  ErrVal init();
  ErrVal uninit();

  ErrVal PushElement(Element element);
  Element PopElement();

  ErrVal	printSeqNumList();

  bool CheckBufferFragments();
  bool isEmpty();
  bool isFull();

  Element processPacket(UChar* rtpPacket , UInt packetSize);

  ErrVal test();

  int printRtpBuffer();



};
#endif