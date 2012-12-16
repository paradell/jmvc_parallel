#include <cstdio>
#include "MVCParallelEncoder.h"
#include "H264AVCEncoderTest.h"
#include "H264AVCCommonLib/CommonBuffers.h"

//#include "MVCBStreamAssembler.h"


int
main( int argc, char** argv)   
{
  printf("JMVC %s Encoder (running on a %d-bit system)\n\n",_JMVC_VERSION_, sizeof(void*)*8);

  if(argc==4){

	printf("\n---------------------\nMVC Parallel Encoder.\n---------------------\nGoing to encode %s views\n---------------------\n\n",argv[3]);

  H264AVCEncoderTest*               pcH264AVCEncoderTest = NULL;
  RNOK( H264AVCEncoderTest::create( pcH264AVCEncoderTest ) );
  RNOKS( pcH264AVCEncoderTest->init   ( argc, argv ) );
  RNOK ( pcH264AVCEncoderTest->go     () );
  RNOK ( pcH264AVCEncoderTest->destroy() );
  }
  else{

	  printf("Error de parametres:\n---------\nMVCParallelEncoder -vf [FITXER DE CONFIGURACIO] [NOMBRE DE VISTES]\n");
	  return 0;
  }

		printf("\n\nMVC Parallel Encoder.\n---------------------\nAleix Paradell Navarro. EPSC-UPC. Barcelona 2012\n");

  return 0;
}
