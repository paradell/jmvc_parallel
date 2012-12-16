#include <cstdio>
#include "MVCParallelDecoder.h"
#include "H264AVCDecoderTest.h"


int
main( int argc, char** argv)
{
  H264AVCDecoderTest* pcH264AVCDecoderTest = NULL;
  DecoderParallelParameter    cParameter;

  printf("JMVC %s Decoder\n\n\n",_JMVC_VERSION_);

  RNOKRS( cParameter.init( argc, argv ), -2 );
//TMM_EC {{
	UInt	nCount	=	1;

  WriteYuvIf*                 pcWriteYuv;

  RNOKS( WriteYuvToFile::createMVC( pcWriteYuv, cParameter.cYuvFile, cParameter.uiNumOfViews ) );


  ReadBitstreamFile *pcReadBitstreamFile;

  if(!cParameter.isParallel){ //create the ReadBitstreamFile only if flag is -r 
	  RNOKS( ReadBitstreamFile::create( pcReadBitstreamFile ) ); 
	  RNOKS( pcReadBitstreamFile->init( cParameter.cBitstreamFile ) );  
  }
//TMM_EC }}
	for( UInt n = 0; n < nCount; n++ )
  {
    RNOKR( H264AVCDecoderTest::create   ( pcH264AVCDecoderTest ), -3 );
    RNOKR( pcH264AVCDecoderTest->init   ( &cParameter, (WriteYuvToFile*)pcWriteYuv, pcReadBitstreamFile ),          -4 );
    RNOKR( pcH264AVCDecoderTest->go     (),                       -5 );
    RNOKR( pcH264AVCDecoderTest->destroy(),                       -6 );
  }
//TMM_EC {{
	if( NULL != pcWriteYuv )              
  {
    RNOK( pcWriteYuv->destroy() );  
  }

	if(!cParameter.isParallel){ //If there's a File Reader 
	  if( NULL != pcReadBitstreamFile )     
	  {
		RNOK( pcReadBitstreamFile->uninit() );
		RNOK( pcReadBitstreamFile->destroy() );  
  }
	}

  printf("\n\nMVC Parallel Decoder.\n---------------------\nAleix Paradell Navarro. EPSC-UPC. Barcelona 2012\n");
//TMM_EC }}
  return 0;
}
