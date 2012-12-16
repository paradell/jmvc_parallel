#include <cstdio>
#include "MVCParallelDecoder.h"
#include "DecoderParallelParameter.h"

#ifndef MSYS_WIN32
#define stricmp strcasecmp
#endif


DecoderParallelParameter::DecoderParallelParameter()
{
}

DecoderParallelParameter::~DecoderParallelParameter()
{
}


ErrVal DecoderParallelParameter::init(int argc, char** argv)
{

//  Char* pcCom;

  
  if (argc <5 || argc>7)
    RNOKS ( xPrintUsage(argv) );

  if(argv[1][0]!='-')
	  RNOKS ( xPrintUsage(argv) );
  
  switch(argv[1][1]){
	  case 'r':
		  cBitstreamFile = argv[2];
		  uiUdpPort=0;
		  isParallel=false;
		  break;
	  case 'n':
		  cBitstreamFile="null";
		  uiUdpPort=atoi(argv[2]);
		  isParallel=true;
		  break;
	  default:
		  RNOKS ( xPrintUsage(argv) );
  }

  isVerbose=false;

  if(argv[5]!=NULL)
	if(strcmp(argv[5],"-v")||strcmp(argv[5],"--verbose"))
		isVerbose=true;




  //cBitstreamFile = argv[1]; // input bitstream
  //cBitstreamFile = "c:/inputs/output_oriden.264"; // input bitstream
  //uiUdpPort = argv[1]; //input port instead of bitsream
  //uiUdpPort = 5150;
  cYuvFile       = argv[3]; // decoded output file

  uiNumOfViews   = atoi (argv[4]);
  if (argc==6) {
	  uiMaxPocDiff = (unsigned int) atoi( argv[5] );	
	  if (uiMaxPocDiff<=0)
		  uiMaxPocDiff= 1000; //MSYS_UINT_MAX;
  } else
	uiMaxPocDiff= 1000; //MSYS_UINT_MAX;
	
  //printf("Variables:\n");
  //printf("\tParllel:%d\n",isParallel);
  //printf("\tcBitstreamFile:%c%c%c%c...\n",cBitstreamFile[0],cBitstreamFile[1],cBitstreamFile[2],cBitstreamFile[3]);
  //printf("\tuiUdpPort:%d\n",uiUdpPort);
  //printf("\tuiNumOfViews:%d\n",uiNumOfViews);

 

  return Err::m_nOK;
}



ErrVal DecoderParallelParameter::xPrintUsage(char **argv)
{
	printf("usage: %s <-r/-n> <BitstreamFile/UDPport> <YuvOutputFile> <NumOfViews> <-v/--verbose>  [<maxPodDiff>] \n\n", argv[0] );
	printf("<-r> <BitstreamFile> -- Read from a local MVC coded file\n");
	printf("<-n> <UDPport>       -- Recieve a MVC RTP stream from UDP connection\n\n");
	RERRS();
}



