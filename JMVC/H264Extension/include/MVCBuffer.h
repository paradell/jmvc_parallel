#if !defined(AFX_MVCBUFFER_H__CBFF413E_29A2_B38C_97F1_44E35C83507D__INCLUDED_)
#define AFX_MVCBUFFER_H__CBFF413E_29A2_B38C_97F1_44E35C83507D__INCLUDED_

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#define MAX_MVC_ELEMENTS 100
#define DATA_MVC_ELEMENT_SIZE 15000



#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif

//Buffer d'elements de MVC
typedef struct{
	UChar data[DATA_MVC_ELEMENT_SIZE]; //Aquesta data conté Nal + (NALfrag) + NALext + dataMVC
	UInt size;
}DecoderElement;



//Buffer d'elements de MVC
typedef struct{
	int start;
	int end;
	int elements;
	DecoderElement element[MAX_MVC_ELEMENTS];
}DecoderBuffer;




class H264AVCVIDEOIOLIB_API MVCBuffer
{
private:
	DecoderBuffer buffer;
	
protected:
	MVCBuffer();
	virtual ~MVCBuffer();
	
public:

  ErrVal test();

  static ErrVal create( MVCBuffer*& rpcMVCBuffer );
  ErrVal destroy();

  ErrVal init();
  ErrVal uninit();

  Bool isEmpty();

  UInt popBufferSize();
  UInt popSize();

  ErrVal PushElement(DecoderElement element);
  DecoderElement PopElement();

  ErrVal printMvcList();

  UChar* popElementData();

  void  resetElement(DecoderElement *element); //reinicia els valors del element.


};
#endif