#include <cstdio>
#include "MVCParallelDecoder.h"
#include "H264AVCDecoderTest.h"


H264AVCDecoderTest::H264AVCDecoderTest() :
  m_pcH264AVCDecoder( NULL ),
  m_pcH264AVCDecoderSuffix( NULL ), //JVT-S036 lsj
//TMM_EC  m_pcReadBitstream( NULL ),
//TMM_EC  m_pcWriteYuv( NULL ),
  m_pcParameter( NULL ),
  m_cActivePicBufferList( ),
  m_cUnusedPicBufferList( )
{
}


H264AVCDecoderTest::~H264AVCDecoderTest()
{
}


ErrVal H264AVCDecoderTest::create( H264AVCDecoderTest*& rpcH264AVCDecoderTest )
{
  rpcH264AVCDecoderTest = new H264AVCDecoderTest;
  ROT( NULL == rpcH264AVCDecoderTest );
  return Err::m_nOK;
}


ErrVal H264AVCDecoderTest::init( DecoderParallelParameter *pcDecoderParameter, WriteYuvToFile *pcWriterYuv, ReadBitstreamFile *pcReadBitstreamFile ) //TMM_EC
{
  ROT( NULL == pcDecoderParameter );

  m_pcParameter = pcDecoderParameter;
  m_pcParameter->nResult = -1;
  
  Parallel = m_pcParameter->isParallel;
  isVerbose = m_pcParameter->isVerbose;
  finished = false;
/*TMM_EC
  RNOKS( WriteYuvToFile::create( m_pcWriteYuv, m_pcParameter->cYuvFile ) );

  ReadBitstreamFile *pcReadBitstreamFile;
  RNOKS( ReadBitstreamFile::create( pcReadBitstreamFile ) ); 
  RNOKS( pcReadBitstreamFile->init( m_pcParameter->cBitstreamFile ) );  
//*///
	m_pcWriteYuv	=	pcWriterYuv;
  
	if(!Parallel)
		m_pcReadBitstream = (ReadBitstreamIf*)pcReadBitstreamFile;

	RNOK( h264::CreaterH264AVCDecoder::create( m_pcH264AVCDecoder ) );
	m_pcH264AVCDecoder->setec( m_pcParameter->uiErrorConceal);

	RNOK( h264::CreaterH264AVCDecoder::create( m_pcH264AVCDecoderSuffix ) );  //JVT-S036 

  return Err::m_nOK;
}

ErrVal	H264AVCDecoderTest::setec( UInt uiErrorConceal)
{
	return	m_pcH264AVCDecoder->setec( uiErrorConceal);
}

ErrVal H264AVCDecoderTest::destroy()
{

  if( NULL != m_pcH264AVCDecoder )       
  {
    RNOK( m_pcH264AVCDecoder->destroy() );       
  }

  if( NULL != m_pcH264AVCDecoderSuffix )       
  {//JVT-S036 
    RNOK( m_pcH264AVCDecoderSuffix->destroy() );       
  }
/*TMM_EC
  if( NULL != m_pcWriteYuv )              
  {
    RNOK( m_pcWriteYuv->destroy() );  
  }

  if( NULL != m_pcReadBitstream )     
  {
    RNOK( m_pcReadBitstream->uninit() );  
    RNOK( m_pcReadBitstream->destroy() );  
  }
*/

//  AOF( m_cActivePicBufferList.empty() );
  
  //===== delete picture buffer =====
  PicBufferList::iterator iter;
  for( iter = m_cUnusedPicBufferList.begin(); iter != m_cUnusedPicBufferList.end(); iter++ )
  {
    delete (*iter)->getBuffer();
    delete (*iter);
  }
  for( iter = m_cActivePicBufferList.begin(); iter != m_cActivePicBufferList.end(); iter++ )
  {
    delete (*iter)->getBuffer();
    delete (*iter);
  }

  delete this;
  return Err::m_nOK;
}


ErrVal H264AVCDecoderTest::xGetNewPicBuffer ( PicBuffer*& rpcPicBuffer, UInt uiSize )
{
  if( m_cUnusedPicBufferList.empty() )
  {
    rpcPicBuffer = new PicBuffer( new UChar[ uiSize ] );
  }
  else
  {
    rpcPicBuffer = m_cUnusedPicBufferList.popFront();
  }

  m_cActivePicBufferList.push_back( rpcPicBuffer );
  return Err::m_nOK;
}


ErrVal H264AVCDecoderTest::xRemovePicBuffer( PicBufferList& rcPicBufferUnusedList )
{
  while( ! rcPicBufferUnusedList.empty() )
  {
    PicBuffer* pcBuffer = rcPicBufferUnusedList.popFront();

    if( NULL != pcBuffer )
    {
      PicBufferList::iterator  begin = m_cActivePicBufferList.begin();
      PicBufferList::iterator  end   = m_cActivePicBufferList.end  ();
      PicBufferList::iterator  iter  = std::find( begin, end, pcBuffer );
    
      AOT( pcBuffer->isUsed() )
      m_cUnusedPicBufferList.push_back( pcBuffer );
	  if (iter!=end)
	      m_cActivePicBufferList.erase    (  iter );
    }
  }

  // hwsun, fix meomory for field coding
  for(int i = (m_cActivePicBufferList.size() - m_pcH264AVCDecoder->getMaxEtrDPB() * 4); i > 0; i--)
  {
    PicBuffer* pcBuffer = m_cActivePicBufferList.popFront();    
    if( NULL != pcBuffer )
      m_cUnusedPicBufferList.push_back( pcBuffer );
  }

  return Err::m_nOK;
}

ErrVal
H264AVCDecoderTest::xRecievePacket		(BinData*& rpcBinData,Bool& rbEOS ){
	
	


	if(isVerbose)
		printf("Intentem fer un popMVCdata\n");

	
	
	

	while(m_pcRtpPacker->isMvcBufferEmpty()){
			if(isVerbose)
				printf("\n---------------\nCUA MVC BUIDA\n------------------\n\n");
		
			scoped_lock mvcLock(mvc_mutex);
			mvcSignal.wait(mvcLock);
		}

	UInt size = m_pcRtpPacker->packetSize();
	
	if(isVerbose)
		printf("Tamany: %d\n", size);


	ROT( NULL == ( rpcBinData = new BinData ) );

		

      rpcBinData->set( new UChar[size], size );


	 
	  ROT( NULL == rpcBinData->data() );
  


	  memcpy(rpcBinData->data(),m_pcRtpPacker->popMVCdata(),size);

	  if(!size) //Si ens arriba un paquet buit, es final de Stream
		  rbEOS=true;

	


	  return Err::m_nOK;
}



//ErrVal
//H264AVCDecoderTest::xRecievePacket		( RtpPacker* packer, BinData*& rpcBinData,Bool& rbEOS ){
//
//	UInt size = packer->packetSize();
//	scoped_lock mvcLock(mvc_mutex);
//		
//	  if(isVerbose)
//		  printf("Tamany: %d\n", size);
//
//
//	  ROT( NULL == ( rpcBinData = new BinData ) );
//
//		
//
//      rpcBinData->set( new UChar[size], size );
//
//
//	  ROT( NULL == rpcBinData->data() );
//  
//
//	  while (packer->isMvcBufferEmpty())
//		  mvcSignal.wait(mvcLock);
//
//	    
//	  memcpy(rpcBinData->data(),packer->popMVCelement().data,size);
//
//	  
//	  if(isVerbose){
//		printf("Dades que tenim a rpcBinData->data(): ");
//
//		for(UInt i=0;i<min(size,30);i++)
//			printf("%X ",rpcBinData->data()[i]);
//
//		printf("\n");
//	  }
//
//	return Err::m_nOK;
//}

ErrVal H264AVCDecoderTest::xReleasePacket		( BinData* pcBinData ){
	
	ROFRS( pcBinData, Err::m_nOK );
	pcBinData->deleteData();
	delete pcBinData;
	
	return Err::m_nOK;
}

ErrVal H264AVCDecoderTest::resetElement(DecoderElement* element){

	element->data[0] = NULL;
	element->size=0;

	return Err::m_nOK;
}

void H264AVCDecoderTest::xRecieveRTP(){

	while(!finished){
		//scoped_lock lk(rtp_mutex);
		//printf("Esperem el paquet, fragemntat o no\n");

		//if(m_pcRtpPacker->isRtpBufferFull())
			//rtpSignal.wait(lk);
			//ESPERAR LA NOTIFICACIÖ DEL RECIEVER


		//boost::mutex::scoped_lock io_lock(io_mutex);

		m_pcRtpPacker->recieveRTP();
		
		//Un cop ja no està buit, ho notifiquem
		rtpSignal.notify_one();

		//printf("Tots els fragments han arribat\n");
	}

	printf("Thread de xRecieveRTP ha acabat\n");
}

void H264AVCDecoderTest::xUnPack(){

	while(!finished){
	
		scoped_lock rtpLock(rtp_mutex);
		//scoped_lock mvcLock(mvc_mutex);

		while(m_pcRtpPacker->isRtpBufferEmpty()){
			//printf("\n---------------\nCUA RTP BUIDA\n------------------\n\n");
			rtpSignal.wait(rtpLock);
		}
			//ESPERAR LA NOTIFICACIÖ DEL RECIEVER
		

		m_pcRtpPacker->unpack();

		//boost::mutex::scoped_lock io_lock(io_mutex);

		//Un cop ja no està ple, ho notifiquem
		//printf("Hem eliminat un paquet de la RTP Buffer\n");
		rtpSignal.notify_one();

		//També notifquem que hi ha un paquet nou a MVCBuffer
		//printf("Hem posat un paquet de la MVC Buffer\n");
		mvcSignal.notify_one();

	}

	printf("Thread de xUnPack ha acabat\n");

}



ErrVal H264AVCDecoderTest::go()
{
  PicBuffer*    pcPicBuffer = NULL;
  PicBufferList cPicBufferOutputList; 
  PicBufferList cPicBufferUnusedList;
  PicBufferList cPicBufferReleaseList;

  UInt      uiMbX           = 0;
  UInt      uiMbY           = 0;
  UInt      uiNalUnitType   = 0;
  UInt      uiSize          = 0;
  UInt      uiLumOffset     = 0;
  UInt      uiCbOffset      = 0;
  UInt      uiCrOffset      = 0;
  UInt      uiFrame;
  
  Bool      bEOS            = false;
  Bool      bYuvDimSet      = false;


  // HS: packet trace
  UInt   uiMaxPocDiff = m_pcParameter->uiMaxPocDiff;
  UInt   uiLastPoc    = MSYS_UINT_MAX;
  UChar* pcLastFrame  = 0;
  UInt   uiPreNalUnitType = 0;

  cPicBufferOutputList.clear();
  cPicBufferUnusedList.clear();

  //DecoderElement element; //cal definir el tamany
  
  //resetElement(&element);
  
  BinData* pcBinData; // Cal definir el tamany


  RNOK( m_pcH264AVCDecoder->init(true, (DecoderParameter*)m_pcParameter) ); 

  if(Parallel){
	RNOK (RtpPacker::create(m_pcRtpPacker));

	//inicialitzem les cues i el socketUDP com a reciever. //Espera 10 segons a que el client es connecti
	m_pcRtpPacker->init(NULL,m_pcParameter->uiUdpPort,false);
  }

	 Int  iPos=0;

  
	 Bool bToDecode = false; //JVT-P031

	 //Creem dos threads per rebre i desempaquetar

	 if(Parallel){

		  
			  //resetElement(&element);
			  //Thread 0 - Anar rebent del socket de UDP
		RecieveThread =  boost::thread (&H264AVCDecoderTest::xRecieveRTP,this);

			  

		  
			  //Thread 1 - Anar passant de RtpBuffer a MVCBuffer quan estigui prou ple.  
		UnpackThread= boost::thread (&H264AVCDecoderTest::xUnPack,this);

		
	  }

	 
	 
	
  for( uiFrame = 0; ( uiFrame <= /*488*/MSYS_UINT_MAX && ! bEOS); )
  {
	if(isVerbose)
		printf("uiFrame = %d\n",uiFrame);
    BinDataAccessor cBinDataAccessor;

   
//    Bool bFinishChecking;

	if(!Parallel)
		RNOK( m_pcReadBitstream->getPosition(iPos) );

    //JVT-P031
    Bool bFragmented = false;
    Bool bDiscardable = false;
    Bool bStart = false;
    Bool bFirst = true;
    UInt uiTotalLength = 0;
#define MAX_FRAGMENTS 10 // hard-coded
    BinData* pcBinDataTmp[MAX_FRAGMENTS];
    BinDataAccessor cBinDataAccessorTmp[MAX_FRAGMENTS];
    UInt uiFragNb, auiStartPos[MAX_FRAGMENTS], auiEndPos[MAX_FRAGMENTS];
	Bool bConcatenated = false; //FRAG_FIX_3
    Bool bSkip  = false;  // Dong: To skip unknown NAL unit types
    uiFragNb = 0;
    bEOS = false;
    pcBinData = 0;



	  //if(Parallel){

		 // //while fragmentat
			//  //resetElement(&element);
			//  //Thread 0 - Anar rebent del socket de UDP
			//  //boost::thread RecieveThread(&m_pcRtpPacker->recieveRTP);

			//	xRecieveRTP();
		 // //m_pcRtpPacker->recieveRTP();

			//  

		 // 
			//  //Thread 1 - Anar passant de RtpBuffer a MVCBuffer quan estigui prou ple.  
			//  xUnPack();
	  //}



//    BinData* pcBinData;
//    BinDataAccessor cBinDataAccessor;
//
//    Int  iPos;
////    Bool bFinishChecking;
//
//    RNOK( m_pcReadBitstream->getPosition(iPos) );
//
//    //JVT-P031
//    Bool bFragmented = false;
//    Bool bDiscardable = false;
//    Bool bStart = false; 
//    Bool bFirst = true;
//    UInt uiTotalLength = 0;
//#define MAX_FRAGMENTS 10 // hard-coded
//    BinData* pcBinDataTmp[MAX_FRAGMENTS];
//    BinDataAccessor cBinDataAccessorTmp[MAX_FRAGMENTS];
//    UInt uiFragNb, auiStartPos[MAX_FRAGMENTS], auiEndPos[MAX_FRAGMENTS];
//	Bool bConcatenated = false; //FRAG_FIX_3
//    Bool bSkip  = false;  // Dong: To skip unknown NAL unit types
//    uiFragNb = 0;
//    bEOS = false;
//    pcBinData = 0;

    while(!bStart && !bEOS)
    {


		if(!Parallel)
			  if(bFirst)
			  {
				  RNOK( m_pcReadBitstream->setPosition(iPos) );
				  bFirst = false;
			  }

	  	  	  
	  if(!Parallel){
		  RNOK( m_pcReadBitstream->extractPacket( pcBinDataTmp[uiFragNb], bEOS ) );
	  }
	  else{
		  /*element = m_pcRtpPacker->popMVCdata();
		  		    printf("Dades rebudes abans de crear xRecievePacket: ");

				for(UInt i=0;i<element.size;i++)
				  printf("%X ",element.data[i]);

			  printf("\n");*/

		  //RNOK( xRecievePacket( m_pcRtpPacker,pcBinDataTmp[uiFragNb], bEOS ) );
		  
				
		 // popMvcThread = boost::thread (&H264AVCDecoderTest::xRecievePacket,this,pcBinDataTmp[uiFragNb], bEOS);

		  //popMvcThread.join();
		  RNOK( xRecievePacket( pcBinDataTmp[uiFragNb], bEOS ) );
		  //system("pause");
	  }


	  if(isVerbose){
		   printf("Dades que tenim a pcBinDataTmp[uiFragNb]->data(): ");

		   for(UInt i=0;i<min(30,pcBinDataTmp[uiFragNb]->size());i++)
			  printf("%X ",pcBinDataTmp[uiFragNb]->data()[i]);

		  printf("\n");
	  }



//TMM_EC {{
			if( !bEOS && ((pcBinDataTmp[uiFragNb]->data())[0] & 0x1f )== 0x0b)
			{
				bEOS=true;
				uiNalUnitType= uiPreNalUnitType;
        
				RNOK( xReleasePacket( pcBinDataTmp[uiFragNb] ) );
				pcBinDataTmp[uiFragNb] = new BinData;
				uiTotalLength	=	0;
				pcBinDataTmp[uiFragNb]->set( new UChar[uiTotalLength], uiTotalLength );
			}
//TMM_EC }}

      pcBinDataTmp[uiFragNb]->setMemAccessor( cBinDataAccessorTmp[uiFragNb] );

      bSkip = false;
      // open the NAL Unit, determine the type and if it's a slice get the frame size

      RNOK( m_pcH264AVCDecoder->initPacket( &cBinDataAccessorTmp[uiFragNb], 
                                            uiNalUnitType, uiMbX, uiMbY, uiSize,  true, 
		  false, //FRAG_FIX_3
//		  bStart, auiStartPos[uiFragNb], auiEndPos[uiFragNb], bFragmented, bDiscardable ) );
		  bStart, auiStartPos[uiFragNb], auiEndPos[uiFragNb], bFragmented, bDiscardable, this->m_pcParameter->getNumOfViews(), bSkip ) );

      uiTotalLength += auiEndPos[uiFragNb] - auiStartPos[uiFragNb];

      // Dong: Skip unknown NAL units
      if( bSkip )
      {
        printf("Unknown NAL unit type: %d\n", uiNalUnitType);
        uiTotalLength -= (auiEndPos[uiFragNb] - auiStartPos[uiFragNb]);
      }
      else if(!bStart)
      {
        ROT( bEOS) ; //jerome.vieron@thomson.net
        uiFragNb++;
      }
      else
      {
        if(pcBinDataTmp[0]->size() != 0)
        {
          pcBinData = new BinData;
          pcBinData->set( new UChar[uiTotalLength], uiTotalLength );
          // append fragments
          UInt uiOffset = 0;
          for(UInt uiFrag = 0; uiFrag<uiFragNb+1; uiFrag++)
          {
              memcpy(pcBinData->data()+uiOffset, pcBinDataTmp[uiFrag]->data() + auiStartPos[uiFrag], auiEndPos[uiFrag]-auiStartPos[uiFrag]);
              uiOffset += auiEndPos[uiFrag]-auiStartPos[uiFrag];
              //RNOK( m_pcReadBitstream->releasePacket( pcBinDataTmp[uiFrag] ) );
			  RNOK( xReleasePacket( pcBinDataTmp[uiFragNb] ) );
			  pcBinDataTmp[uiFrag] = NULL;
              if(uiNalUnitType != 6) //JVT-T054
              m_pcH264AVCDecoder->decreaseNumOfNALInAU();
			  //FRAG_FIX_3
			  if(uiFrag > 0) 
				  bConcatenated = true; //~FRAG_FIX_3
          }
          
          pcBinData->setMemAccessor( cBinDataAccessor );
          bToDecode = false;
          if((uiTotalLength != 0) && (!bDiscardable || bFragmented))
          {
              //FRAG_FIX
			if( (uiNalUnitType == 20) || (uiNalUnitType == 21) || (uiNalUnitType == 1) || (uiNalUnitType == 5) )
            {
                uiPreNalUnitType=uiNalUnitType;
                RNOK( m_pcH264AVCDecoder->initPacket( &cBinDataAccessor, uiNalUnitType, uiMbX, uiMbY, uiSize, 
					//uiNonRequiredPic, //NonRequired JVT-Q066
                    false, bConcatenated, //FRAG_FIX_3
					bStart, auiStartPos[uiFragNb+1], auiEndPos[uiFragNb+1], 
//                    bFragmented, bDiscardable) );
                    bFragmented, bDiscardable, this->m_pcParameter->getNumOfViews(), bSkip) );
            }

        else if( uiNalUnitType == 14 )
          {
			  if(isVerbose)
				printf("uiNalUnitType == 14\n");

			uiPreNalUnitType=uiNalUnitType;
            RNOK( m_pcH264AVCDecoder->initPacket( &cBinDataAccessor, uiNalUnitType, uiMbX, uiMbY, uiSize, 
					//uiNonRequiredPic, //NonRequired JVT-Q066
			false, bConcatenated, //FRAG_FIX_3
					bStart, auiStartPos[uiFragNb+1], auiEndPos[uiFragNb+1], 
//                    bFragmented, bDiscardable) );
			bFragmented, bDiscardable,this->m_pcParameter->getNumOfViews(), bSkip) );
                    
              }
              else
                  m_pcH264AVCDecoder->initPacket( &cBinDataAccessor );
              bToDecode = true;

              if( uiNalUnitType == 14 )
                bToDecode = false;
          }
        }
      }
	  
    }

    //~JVT-P031

//NonRequired JVT-Q066{
	if(m_pcH264AVCDecoder->isNonRequiredPic())
		continue;
//NonRequired JVT-Q066}


// JVT-Q054 Red. Picture {
  RNOK( m_pcH264AVCDecoder->checkRedundantPic() );
  if ( m_pcH264AVCDecoder->isRedundantPic() )
    continue;
// JVT-Q054 Red. Picture }



  if(bToDecode)//JVT-P031
  {
    // get new picture buffer if required if coded Slice || coded IDR slice
    pcPicBuffer = NULL;
    
    if( uiNalUnitType == 1 || uiNalUnitType == 5 || uiNalUnitType == 20 || uiNalUnitType == 21 )
    {
		if(isVerbose)
			printf("xGetNewPicBuffer\n");
      
		RNOK( xGetNewPicBuffer( pcPicBuffer, uiSize ) );

      if( ! bYuvDimSet )
      {
        UInt uiLumSize  = ((uiMbX<<3)+  YUV_X_MARGIN) * ((uiMbY<<3)    + YUV_Y_MARGIN ) * 4;
        uiLumOffset     = ((uiMbX<<4)+2*YUV_X_MARGIN) * YUV_Y_MARGIN   + YUV_X_MARGIN;  
        uiCbOffset      = ((uiMbX<<3)+  YUV_X_MARGIN) * YUV_Y_MARGIN/2 + YUV_X_MARGIN/2 + uiLumSize; 
        uiCrOffset      = ((uiMbX<<3)+  YUV_X_MARGIN) * YUV_Y_MARGIN/2 + YUV_X_MARGIN/2 + 5*uiLumSize/4;
        bYuvDimSet = true;

        // HS: decoder robustness
        pcLastFrame = new UChar [uiSize];
        ROF( pcLastFrame );
      }
    }
    
	//AQUI JA HEM HAGUT DE FER EL RECIEVE/UNPACK/POPMVC

    // decode the NAL unit
	if(isVerbose)
		printf("\n\nm_pcH264AVCDecoder->process\n\n");
    
	RNOK( m_pcH264AVCDecoder->process( pcPicBuffer, cPicBufferOutputList, cPicBufferUnusedList, cPicBufferReleaseList ) );

	// ROI DECODE ICU/ETRI
	m_pcH264AVCDecoder->RoiDecodeInit();

	setCrop();//lufeng: support frame cropping

    // picture output
    while( ! cPicBufferOutputList.empty() )
    {
		if(isVerbose)
			printf("! cPicBufferOutputList.empty()\n");

//JVT-V054    
      if(!m_pcWriteYuv->getFileInitDone() )
      {
		  if(isVerbose)
			  printf("!m_pcWriteYuv->getFileInitDone\n");
		  
		  //UInt *vcOrder = m_pcH264AVCDecoder->getViewCodingOrder();
		  UInt *vcOrder = m_pcH264AVCDecoder->getViewCodingOrder_SubStream();
		  if(vcOrder == NULL)//lufeng: in order to output non-MVC seq
          {
			  //UInt order=0;
			  m_pcH264AVCDecoder->addViewCodingOrder();
			  //vcOrder = m_pcH264AVCDecoder->getViewCodingOrder();
			  vcOrder = m_pcH264AVCDecoder->getViewCodingOrder_SubStream();
		  }
       		m_pcWriteYuv->xInitMVC(m_pcParameter->cYuvFile, vcOrder, m_pcParameter->getNumOfViews()); // JVT-AB024 modified remove active view info SEI  			
      }

        PicBuffer* pcPicBufferTmp = cPicBufferOutputList.front();
      cPicBufferOutputList.pop_front();
        if( pcPicBufferTmp != NULL )
      {
        // HS: decoder robustness
          while( uiLastPoc + uiMaxPocDiff < (UInt)pcPicBufferTmp->getCts() )
			{
				if(isVerbose)
					printf("m_pcWriteYuv->writeFrame del primer IF\n");

			  RNOK( m_pcWriteYuv->writeFrame( pcLastFrame + uiLumOffset, 
											  pcLastFrame + uiCbOffset, 
											  pcLastFrame + uiCrOffset,
											   uiMbY << 4,
											   uiMbX << 4,
											  (uiMbX << 4)+ YUV_X_MARGIN*2 ) );
			 
			  uiFrame   ++;
			  uiLastPoc += uiMaxPocDiff;
			}

		  
          if(m_pcParameter->getNumOfViews() > 0)
			{
			  UInt view_cnt;
			  
				for (view_cnt=0; view_cnt < m_pcParameter->getNumOfViews(); view_cnt++){
					//UInt tmp_order=m_pcH264AVCDecoder->getViewCodingOrder()[view_cnt];
				  UInt tmp_order=m_pcH264AVCDecoder->getViewCodingOrder_SubStream()[view_cnt];
					if ((UInt)pcPicBufferTmp->getViewId() == tmp_order)
					break;
					}
				

				//Es fa efectiu el print a l'arxiu de sortida YUV
			  RNOK( m_pcWriteYuv->writeFrame( *pcPicBufferTmp + uiLumOffset, 
                                              *pcPicBufferTmp + uiCbOffset, 
                                              *pcPicBufferTmp + uiCrOffset,
                                              uiMbY << 4,
                                              uiMbX << 4,
                                              (uiMbX << 4)+ YUV_X_MARGIN*2,
                                              //(UInt)pcPicBufferTmp->getViewId(),
											   view_cnt) ); 
			}
		else
			RNOK( m_pcWriteYuv->writeFrame( *pcPicBufferTmp + uiLumOffset, 
                                        *pcPicBufferTmp + uiCbOffset, 
                                        *pcPicBufferTmp + uiCrOffset,
                                         uiMbY << 4,
                                         uiMbX << 4,
                                        (uiMbX << 4)+ YUV_X_MARGIN*2 ) );

			uiFrame++;
      
    
        // HS: decoder robustness
        uiLastPoc = (UInt)pcPicBufferTmp->getCts();
        ::memcpy( pcLastFrame, *pcPicBufferTmp+0, uiSize*sizeof(UChar) );
      }
    }
   } 
    RNOK( xRemovePicBuffer( cPicBufferReleaseList ) );
    RNOK( xRemovePicBuffer( cPicBufferUnusedList ) );
    if( pcBinData )
    {
      RNOK( xReleasePacket( pcBinData ) );
      pcBinData = 0;
    }
	//printf("\n\nUn altre FRAME\n\n");
  }
  finished = true;
  printf("\n %d frames decoded\n", uiFrame );

  delete [] pcLastFrame; // HS: decoder robustness
  
  RNOK( m_pcH264AVCDecoder->uninit( true ) );
  
  m_pcParameter->nFrames  = uiFrame;
  m_pcParameter->nResult  = 0;

  return Err::m_nOK;
}





ErrVal H264AVCDecoderTest::setCrop()
{
	UInt uiCrop[4];
	m_pcH264AVCDecoder->setCrop(uiCrop);
	m_pcWriteYuv->setCrop(uiCrop);
	return Err::m_nOK;
}
