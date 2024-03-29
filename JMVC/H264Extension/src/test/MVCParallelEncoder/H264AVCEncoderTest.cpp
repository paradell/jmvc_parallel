#include <cstdio>
#include "MVCParallelEncoder.h"
#include "H264AVCEncoderTest.h"
#include "EncoderCodingParameter.h"





H264AVCEncoderTest::H264AVCEncoderTest() 
  //m_pcH264AVCEncoder        ( NULL ),
  //m_pcWriteBitstreamToFile  ( NULL ),
  //m_pcEncoderCodingParameter[0]( NULL )
{
	//::memset( m_pcH264AVCEncoder,   0x00, MAX_LAYERS*sizeof(Void*) );
  ::memset( m_pcEncoderCodingParameter,   0x00, MAX_LAYERS*sizeof(Void*) );
  ::memset( m_pcH264AVCEncoder,   0x00, MAX_LAYERS*sizeof(Void*) );
  ::memset( m_apcReadYuv,   0x00, MAX_LAYERS*sizeof(Void*) );
  ::memset( m_apcWriteYuv,  0x00, MAX_LAYERS*sizeof(Void*) );
  ::memset( m_auiLumOffset, 0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiCbOffset,  0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiCrOffset,  0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiHeight,    0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiWidth,     0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiStride,    0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_aauiCropping, 0x00, MAX_LAYERS*sizeof(UInt)*4);
  //::memset( m_apcRtpPacker, 0x00, MAX_LAYERS*sizeof(Void*) );
}


H264AVCEncoderTest::~H264AVCEncoderTest()
{
}


ErrVal
H264AVCEncoderTest::create( H264AVCEncoderTest*& rpcH264AVCEncoderTest )
{
  rpcH264AVCEncoderTest = new H264AVCEncoderTest;

  ROT( NULL == rpcH264AVCEncoderTest );
  
  return Err::m_nOK;
}

ErrVal
H264AVCEncoderTest::xSetProcessingInfo	(UInt Frame,UInt MaxFrames,UInt View){

	m_apcProcessingInfo.nFrame =Frame;
    m_apcProcessingInfo.nMaxFrames=MaxFrames;
  m_apcProcessingInfo.nView = View;


	return Err::m_nOK;
}


ErrVal H264AVCEncoderTest::init( Int    argc,
                                 Char** argv )
{
  //===== define the nuimber of views =========
  UInt uiNumberOfViews = atoi(argv[3]);
  //===== create and read encoder parameters =====

  isVerbose = false;
  UInt i=0;
  UInt uiView=0;

  nFreeThread=0;


  xSetProcessingInfo(0,0,0);

  RtpPacker::create(m_apcRtpPacker);
  
 
  for(i=0;i<uiNumberOfViews;i++){
	  RNOK( EncoderCodingParameter::create( m_pcEncoderCodingParameter[i] ) );
	  itoa(i,argv[3],10);
	  
	  if( Err::m_nOK != m_pcEncoderCodingParameter[i]->init( argc, argv, m_cEncoderIoParameter.cBitstreamFilename ) )
	  {
		printf("Error en argv\n");
		m_pcEncoderCodingParameter[i]->printHelpMVC(argc, argv);
		return -3;
	  }
	  
	  m_cEncoderIoParameter.nResult = -1;
  }

   //===Create Output File writer========
  
  isVerbose = m_pcEncoderCodingParameter[0]->isVerbose();
  
  if(m_pcEncoderCodingParameter[0]->isDebug()){

	  //El output file el fem servir per comprovar que la sortida sigui correcta.
	  if(isVerbose)
		printf("Creem el fitxer output per debug\n");
	  
	  RNOKS( WriteBitstreamToFile::create   ( m_pcWriteBitstreamToOutput ) );
	  RNOKS( m_pcWriteBitstreamToOutput->init ( "c:/inputs/output_parallel.264" ) );  
  
  }

  
  //===== init instances for reading and writing yuv data =====
  if(uiNumberOfViews!=m_pcEncoderCodingParameter[0]->getNumberOfLayers()){
	printf("\nError de par�metres.\n El nombre de vistes no concorda\n.");
	exit(0);
  }
  
  
  
  for( uiView = 0; uiView < uiNumberOfViews; uiView++ )
  {
	  h264::LayerParameters&  rcLayer = m_pcEncoderCodingParameter[uiView]->getLayerParameters( uiView );
  
	  if(!m_pcEncoderCodingParameter[uiView]->isParallel()){
		RNOKS( WriteYuvToFile::create( m_apcWriteYuv[uiView], rcLayer.getOutputFilename() ) );
	  }

	  
	RNOKS( ReadYuvFile   ::create( m_apcReadYuv [uiView] ) );  

	RNOKS( m_apcReadYuv[uiView]->init( rcLayer.getInputFilename(),
                                        rcLayer.getFrameHeight  (),
                                        rcLayer.getFrameWidth   () ) ); 
  }


  for(uiView=0;uiView<uiNumberOfViews;uiView++){ 
  //===== init bitstream writer =====
	  if( m_pcEncoderCodingParameter[uiView]->getMVCmode() )
	  {
	  //SEI {
			if( m_pcEncoderCodingParameter[uiView]->getViewScalInfoSEIEnable() )
			{
			m_cWriteToBitFileTempName                 = m_cEncoderIoParameter.cBitstreamFilename[uiView] + ".temp";
			m_cWriteToBitFileName                     = m_cEncoderIoParameter.cBitstreamFilename[uiView];
			m_cEncoderIoParameter.cBitstreamFilename[uiView]  = m_cWriteToBitFileTempName;

			}
		  

		}
	  else
	  {
		m_cWriteToBitFileTempName                 = m_cEncoderIoParameter.cBitstreamFilename[uiView] + ".temp";
		m_cWriteToBitFileName                     = m_cEncoderIoParameter.cBitstreamFilename[uiView];
		m_cEncoderIoParameter.cBitstreamFilename[uiView]  = m_cWriteToBitFileTempName;		
	  }
  }

 

  //===== create encoder instance =====
  for(uiView=0;uiView<uiNumberOfViews;uiView++){ 
	RNOK( h264::CreaterH264AVCEncoder::create( m_pcH264AVCEncoder[uiView] ) );
  }


  //===== set start code =====
  m_aucStartCodeBuffer[0] = 0;
  m_aucStartCodeBuffer[1] = 0;
  m_aucStartCodeBuffer[2] = 0;
  m_aucStartCodeBuffer[3] = 1;
  m_cBinDataStartCode.reset ();
  m_cBinDataStartCode.set   ( m_aucStartCodeBuffer, 4 );

  // Extended NAL unit priority is enabled by default, since 6-bit short priority
  // is incompatible with extended 4CIF Palma test set.  Change value to false
  // to enable short ID.
   for(uiView=0;i<uiNumberOfViews;uiView++){
		m_pcEncoderCodingParameter[uiView]->setExtendedPriorityId( true );
   }

  // Example priority ID assignment: (a) spatial, (b) temporal, (c) quality
  // Other priority assignments can be created by adjusting the mapping table.
  // (J. Ridge, Nokia)

   
   for(uiView=0;uiView<uiNumberOfViews;uiView++){
	  if ( !m_pcEncoderCodingParameter[uiView]->getExtendedPriorityId() )
	  {
		UInt  uiPriorityId = 0;
		for( UInt uiLayer = 0; uiLayer < m_pcEncoderCodingParameter[i]->getNumberOfLayers(); uiLayer++ )
		{
			UInt uiBitplanes;
			if ( m_pcEncoderCodingParameter[uiView]->getLayerParameters( uiLayer ).getFGSMode() > 0 )
			{
			  uiBitplanes = MAX_QUALITY_LEVELS - 1;  
			}
			else {
			  uiBitplanes = (UInt) m_pcEncoderCodingParameter[uiView]->getLayerParameters( uiLayer ).getNumFGSLayers();
			  if ( m_pcEncoderCodingParameter[uiView]->getLayerParameters( uiLayer ).getNumFGSLayers() > (Double) uiBitplanes)
			  {
				uiBitplanes++;
			  }
			}
	 /*       for ( UInt uiTempLevel = 0; uiTempLevel <= m_pcEncoderCodingParameter[0]->getLayerParameters( uiLayer ).getDecompositionStages(); uiTempLevel++ )
			{
				for ( UInt uiQualLevel = 0; uiQualLevel <= uiBitplanes; uiQualLevel++ )
				{
					m_pcEncoderCodingParameter[0]->setSimplePriorityMap( uiPriorityId++, uiTempLevel, uiLayer, uiQualLevel );
					AOF( uiPriorityId > ( 1 << PRI_ID_BITS ) );
				}
			}
	 JVT-S036  */
		}

		m_pcEncoderCodingParameter[uiView]->setNumSimplePris( uiPriorityId );
	  }
   }

  
  return Err::m_nOK;
}




ErrVal
H264AVCEncoderTest::destroy()
{

 //printf("H264AVCEncoderTest::destroy()\n");
  m_cBinDataStartCode.reset();
  
 //printf("Destroy Creater Encoders\n");
  for( UInt ui = 0; ui < 2; ui++ ){
	  if( m_pcH264AVCEncoder[ui] )       
	  {
		RNOK( m_pcH264AVCEncoder[ui]->uninit() );       
		RNOK( m_pcH264AVCEncoder[ui]->destroy() ); 
		
	  }
  }
  
  //printf("Destroy m_apcWriteYuv i m_apcReadYuv\n");
  for( UInt ui = 0; ui < MAX_LAYERS; ui++ )
  {
	 if( m_pcEncoderCodingParameter[0]->isDebug() )              
    {
		
      RNOK( m_apcWriteYuv[ui]->destroy() );  
    }

    if( m_apcReadYuv[ui] )              
    {
	 
      RNOK( m_apcReadYuv[ui]->uninit() );  
      RNOK( m_apcReadYuv[ui]->destroy() );
	  
    }
	
  }

  
  for( UInt ui = 0; ui < 2; ui++ ){
	 
	  RNOK( m_pcEncoderCodingParameter[ui]->destroy());
  }
  

  for( UInt uiView = 0; uiView < MAX_LAYERS; uiView++ )
  {
	
    AOF( m_acActivePicBufferList[uiView].empty() );
    
    //===== delete picture buffer =====
    PicBufferList::iterator iter;
    for( iter = m_acUnusedPicBufferList[uiView].begin(); iter != m_acUnusedPicBufferList[uiView].end(); iter++ )
    {
      delete (*iter)->getBuffer();
      delete (*iter);
    }
    for( iter = m_acActivePicBufferList[uiView].begin(); iter != m_acActivePicBufferList[uiView].end(); iter++ )
    {
      delete (*iter)->getBuffer();
      delete (*iter);
    }
  }

  
  delete this;
  
  return Err::m_nOK;
}


ErrVal
H264AVCEncoderTest::xGetNewPicBuffer ( PicBuffer*&  rpcPicBuffer,
                                       UInt         uiLayer,
                                       UInt         uiSize )
{
  if( m_acUnusedPicBufferList[uiLayer].empty() )
  {
    rpcPicBuffer = new PicBuffer( new UChar[ uiSize ] );
  }
  else
  {
    rpcPicBuffer = m_acUnusedPicBufferList[uiLayer].popFront();
  }

  m_acActivePicBufferList[uiLayer].push_back( rpcPicBuffer );

  return Err::m_nOK;
}


ErrVal
H264AVCEncoderTest::xRemovePicBuffer( PicBufferList&  rcPicBufferUnusedList,
                                      UInt            uiView )
{
  while( ! rcPicBufferUnusedList.empty() )
  {
	//printf("! rcPicBufferUnusedList.empty() del view %d\n",uiView);
    PicBuffer* pcBuffer = rcPicBufferUnusedList.popFront();

    if( NULL != pcBuffer )
    {
	 //printf("NULL != pcBuffer\n");
      PicBufferList::iterator begin = m_acActivePicBufferList[uiView].begin();
      PicBufferList::iterator end   = m_acActivePicBufferList[uiView].end  ();
      PicBufferList::iterator iter  = std::find( begin, end, pcBuffer );

	  if( iter == end ){ // there is something wrong if the address is not in the active list
		return Err::m_nOK;
	  }
      AOT_DBG( (*iter)->isUsed() );
      m_acUnusedPicBufferList[uiView].push_back( *iter );
      m_acActivePicBufferList[uiView].erase    (  iter );
    }
  }
  //printf("View %d fet.\n",uiView);
  return Err::m_nOK;
}


ErrVal
H264AVCEncoderTest::xWrite( PicBufferList&  rcPicBufferList,
                            UInt            uiLayer )
{
  while( ! rcPicBufferList.empty() )
  {
    PicBuffer* pcBuffer = rcPicBufferList.popFront();

    Pel* pcBuf = pcBuffer->getBuffer();
    RNOK( m_apcWriteYuv[uiLayer]->writeFrame( pcBuf + m_auiLumOffset[uiLayer], 
                                              pcBuf + m_auiCbOffset [uiLayer],
                                              pcBuf + m_auiCrOffset [uiLayer],
                                              m_auiHeight           [uiLayer],
                                              m_auiWidth            [uiLayer],
                                              m_auiStride           [uiLayer] ) );
  }
  return Err::m_nOK;
}


ErrVal
H264AVCEncoderTest::xRelease( PicBufferList&  rcPicBufferList,
                              UInt            uiLayer )
{
  RNOK( xRemovePicBuffer( rcPicBufferList, uiLayer ) );
  return Err::m_nOK;
}

ErrVal	
H264AVCEncoderTest::xWriteInit		( ExtBinDataAccessor cExtBinDataAccessor,bool debug)
{
	
	//printf("\nTamany Llista de la vista: %d\n",rcViewList.size());
	
	if(debug){
		
		if(isVerbose)
			printf("Write Init per debug\n");
		///printf("Escribim el codi d'inici al view %d\n",numLayer);
		RNOK( m_pcWriteBitstreamToOutput->writePacket( &m_cBinDataStartCode ) );
		////printf("Escribim %d bytes de dades al view %d\n",LayerBuffer[numLayer].front()->byteSize(),numLayer);
		RNOK( m_pcWriteBitstreamToOutput->writePacket( &cExtBinDataAccessor ) );
	
	}
	xSend(cExtBinDataAccessor);

	return Err::m_nOK;
}


ErrVal
H264AVCEncoderTest::xWrite( ExtBinDataAccessorList& rcList,
                            UInt&                   ruiBytesInFrame
							)
{
	while( rcList.size() )
  {
    ruiBytesInFrame += rcList.front()->size() + 4;
    
	RNOK( m_pcWriteBitstreamToOutput->writePacket( &m_cBinDataStartCode ) );
    RNOK( m_pcWriteBitstreamToOutput->writePacket( rcList.front() ) );
   
	delete[] rcList.front()->data();
    delete   rcList.front();
    rcList.pop_front();
  }
  return Err::m_nOK;
	
}


ErrVal
H264AVCEncoderTest::xSend( ExtBinDataAccessor cExtBinDataAccessor)
{
	if(isVerbose)
		printf("\n\nPrimerfragment del paquet\n");

	if(isVerbose){	
		printf("Dades que enviem al xSend: ");

		  for(UInt i=0;i<cExtBinDataAccessor.size();i++)
			  printf("%X ",cExtBinDataAccessor.data()[i]);

		  printf("\n");
	}

	m_apcRtpPacker->packInit(cExtBinDataAccessor.data(),cExtBinDataAccessor.size());

	//system("pause");
	


	return Err::m_nOK;
}
 
ErrVal
H264AVCEncoderTest::xSend( ExtBinDataAccessorList& rcList) //xSend per enviar la NAL de la view 0 junt amb les seves dades
{ 
	//Agafem un Frame d'una vista i l'empaquetem en RTP.
	

 
	if(isVerbose)
		printf("Tamany de la llista: %d\n",rcList.size());

	UChar nal_unit[4];
	 
	 if(rcList.size())
	 {	
	  nal_unit[0]=rcList.front()->data()[0];
	  nal_unit[1]=rcList.front()->data()[1];
	  nal_unit[2]=rcList.front()->data()[2];
	  nal_unit[3]=rcList.front()->data()[3];
	 }
	 else{
		 if(isVerbose)
			 printf("Enviem el final de Tansmissio\n"); //Si la llista est� buida, �s que hem arribat al final o hi ha hagut un error i ho buidem tot.
		 m_apcRtpPacker->endTransmission();
	 }
 

	 
while( rcList.size() )
  {
 
	if(isVerbose)
		printf("Enviem un paquet\n");

	
	m_apcRtpPacker->pack(nal_unit,rcList.front()->data(),rcList.front()->size());
	 
	// El SSN Augmenta en 1 per cada vista. Independent de l'ordre en que es codifiquin
	// El Timestamp �s el mateix per numero de Frame
		//En cas de fragmentaci�: - MAteix timestamp, mateix SSN. Duplicare NAL + NAL especifica


	
	delete[] rcList.front()->data();
    delete   rcList.front();
	//delete[] rcViewList.front()->data();
	//delete rcViewList.front();
    
	rcList.pop_front();
	//rcViewList.pop_front();
 }
	//system("pause");

	return Err::m_nOK;
 }

ErrVal
H264AVCEncoderTest::xAskForSend(ExtBinDataAccessorList& rcList, UInt nView, UInt nFrame){
	
	int previousView; //refer�ncia de la frame antiga
	int nextView;

	if(nView!=0)
		previousView=nView-1;
	else
		previousView=m_pcEncoderCodingParameter[nView]->getNumberOfLayers()-1;

	
	if(nView==m_pcEncoderCodingParameter[nView]->getNumberOfLayers()-1)
		nextView=0;
	else
		nextView=nView+1;


	//Definim el lock del thread
	lock lk(monitor); 

	free_thread[nView].notify_one(); //Refresquem les condicions ja que el proc�s �s m�s lent que el RtpPacker

	if(isVerbose)
		printf("\nLa View %d dona perm�s al thread %d\n\n",nView,previousView);


	

	//if(!(nView==0&&nFrame==0)){//  For the first View, and the first Frame the thread access to RtpPacker without waiting
	while(nView!=nFreeThread){//  If the current thread has no permissions to access RTPPacker we wait until the condition is released
		if(isVerbose)
			printf("La View %d espera el perm�s de la view %d\n",nView,previousView);
		free_thread[nView].notify_one();
		free_thread[previousView].wait(lk); //Esperem a que el thread anterior alliberi el recurs
	}

	if(isVerbose)
		printf("\nLa View %d esta utilitzant el RtpPacker per la frame %d\n\n",nView,nFrame);

		//boost::posix_time::milliseconds workTime(500);
		//boost::this_thread::sleep (workTime);
		//system("pause");

	xSend(rcList);

	//Donem acc�s al seg�ent thread
	nFreeThread=nextView;
	free_thread[nView].notify_one(); //Botifiquem al seg�ent thread que pot accedir al RtpPacker

	if(isVerbose)
		printf("\nLa View %d dona perm�s al thread %d\n\n",nView,previousView);

	return Err::m_nOK;
}

/*
ErrVal
H264AVCEncoderTest::xSend( ExtBinDataAccessorList& rcList) //xSend per enviar la NAL de la view 0 separada de les dades
{ 
	//Agafem un Frame d'una vista i l'empaquetem en RTP.
 
	printf("Tamany de la llista: %d\n",rcList.size());


 while( rcList.size() )
  {
 
	UChar nal_unit[4];
  nal_unit[0]=rcList.front()->data()[0];
  nal_unit[1]=rcList.front()->data()[1];
  nal_unit[2]=rcList.front()->data()[2];
  nal_unit[3]=rcList.front()->data()[3];



	m_apcRtpPacker->pack(nal_unit,rcList.front()->data(),rcList.front()->size());
	 
	// El SSN Augmenta en 1 per cada vista. Independent de l'ordre en que es codifiquin
	// El Timestamp �s el mateix per numero de Frame
		//En cas de fragmentaci�: - MAteix timestamp, mateix SSN. Duplicare NAL + NAL especifica


	
	delete[] rcList.front()->data();
    delete   rcList.front();
	//delete[] rcViewList.front()->data();
	//delete rcViewList.front();
    
	rcList.pop_front();
	//rcViewList.pop_front();
 }
	//system("pause");

	return Err::m_nOK;
 }*/

void 
H264AVCEncoderTest::testThread(int view,UInt uiFrame, UInt uiMaxFrame,UInt uiLayer,UInt auiPicSize, UInt uiWrittenBytes, int v7, int v8){
	printf("prova de thread %d\n",view);
	boost::posix_time::milliseconds workTime(2000);
	//printf("Dades\nview: %d\tuiFrame: %d\tuiMaxFrame: %d\tuiLayer: %d\tuiPicSize: %d\tuiWrittenBytes: %d\n",view,uiFrame, uiMaxFrame, uiLayer, auiPicSize, uiWrittenBytes);
	boost::this_thread::sleep(workTime);
	printf("Final de thread %d\n",view);

}

void 
H264AVCEncoderTest::start(int view,UInt uiFrame, UInt uiMaxFrame,UInt uiLayer,UInt auiPicSize, UInt uiWrittenBytes, int v7, int v8){
	m_Thread[view] = boost::thread(&H264AVCEncoderTest::testThread, this,view, uiFrame,  uiMaxFrame, uiLayer,auiPicSize, uiWrittenBytes, v7, v8 );
	//m_Thread[view] = boost::thread(&H264AVCEncoderTest::xSend, this,cOutExtBinDataAccessorList);
	
}

void 
H264AVCEncoderTest::join(){
	
	if(isVerbose)
		printf("Fem el join de tots els threads\n");

	for(UInt i=0;i<m_pcEncoderCodingParameter[0]->getNumberOfLayers();i++)
		group_threads.add_thread(&m_Thread[i]);
	
	group_threads.join_all();

}





void
H264AVCEncoderTest::xProcessView(processingInfo	auiProcessingInfo,UInt auiPicSize, UInt uiWrittenBytes, ExtBinDataAccessorList cOutExtBinDataAccessorList, PicBuffer* apcOriginalPicBuffer, PicBuffer* apcReconstructPicBuffer, PicBufferList acPicBufferOutputList, PicBufferList acPicBufferUnusedList){

	//system("pause");
	if(isVerbose)
		printf("Frame: %d\nMaxFrames: %d\n,View: %d\n",auiProcessingInfo.nFrame,auiProcessingInfo.nMaxFrames,auiProcessingInfo.nView);
	
	for( auiProcessingInfo.nFrame = 0; auiProcessingInfo.nFrame < auiProcessingInfo.nMaxFrames; auiProcessingInfo.nFrame++ )
  {
	  //m_apcRtpPacker->increaseTimeStamp();

	  if(isVerbose)
		printf("\nFrame: %d\n",auiProcessingInfo.nFrame);
	   //system("pause");
	  
		
	  UInt  uiSkip = ( 1 << m_pcEncoderCodingParameter[auiProcessingInfo.nView]->getLayerParameters( 0 ).getTemporalResolution() );
			  //UInt  uiSkip = ( 1 << m_pcEncoderCodingParameter[uiLayer]->getLayerParameters( uiLayer ).getTemporalResolution() );
			  
			  //
			  //LLEGIM EL FRAME uiFrame PER LA VISTA uiLayer
			  //

	  if( auiProcessingInfo.nFrame % uiSkip == 0 )
			  {
				  xGetNewPicBuffer( apcReconstructPicBuffer , auiProcessingInfo.nView, auiPicSize );
				  xGetNewPicBuffer( apcOriginalPicBuffer   , auiProcessingInfo.nView, auiPicSize ) ;
				
				//printf("Reading Layer %d of frame %d\n",uiLayer,uiFrame);
				//m_apcReadYuv[uiLayer]->m_cFile.tell();
				m_apcReadYuv[auiProcessingInfo.nView]->readFrame( *apcOriginalPicBuffer + m_auiLumOffset[auiProcessingInfo.nView],
														*apcOriginalPicBuffer + m_auiCbOffset[auiProcessingInfo.nView],
														*apcOriginalPicBuffer + m_auiCrOffset[auiProcessingInfo.nView],
														m_auiHeight[auiProcessingInfo.nView] ,
														m_auiWidth[auiProcessingInfo.nView]  ,
														m_auiStride[auiProcessingInfo.nView] ) ;

				//printf("Frame %d, Layer %d, tamany original:%s\n",uiFrame,uiLayer,apcOriginalPicBuffer[uiLayer]);
				
			  }
			  else
			  {
				if(isVerbose)
					printf("Hi ha Hagut un SKIP a la part de readFrame()\n");

				apcReconstructPicBuffer  = 0;
				apcOriginalPicBuffer   = 0;		
			  }
			  

			  //
			  //PROCESSEM EL FRAME uiFrame PER LA VISTA uiLayer
			  //

			  if(isVerbose)
				  printf("View %d\t",auiProcessingInfo.nView);

			   m_pcH264AVCEncoder[auiProcessingInfo.nView]->process( cOutExtBinDataAccessorList,
											   apcOriginalPicBuffer,
											   apcReconstructPicBuffer,
											   &acPicBufferOutputList,
											   &acPicBufferUnusedList ) ;


			   //
			   //ESCRIVIM EL FRAME uiFrame PER LA VISTA uiLayer A DIFERENTS ARXIUS I BUFFERS(OUTPUT, REC, ETC...)
			   //

				//printf("Writing layer %d frame %d\n",uiLayer,uiFrame);
				UInt  uiBytesUsed = 0;
				if(m_pcEncoderCodingParameter[0]->isDebug()){
					if(isVerbose)
						printf("Write per debug\n");				
					xWrite  ( cOutExtBinDataAccessorList,uiBytesUsed) ;
				}
				else{
					{
						boost::mutex::scoped_lock io_lock(io_mutex);
						if(isVerbose)
							printf("View %d bloqueja el RtpPacker\n",auiProcessingInfo.nView);
					}
					//xSend(cOutExtBinDataAccessorList);

					if(!auiProcessingInfo.nView) //Si �s la view 0, augmentem el timestamps
						m_apcRtpPacker->increaseTimeStamp();

					/*printf("Enviem tot NAL+data\n");
					system("pause");*/

					xAskForSend(cOutExtBinDataAccessorList,auiProcessingInfo.nView,auiProcessingInfo.nFrame);
					
				}
				
				//m_apcUDPController->send("Test");
				
						
				uiWrittenBytes  += uiBytesUsed;

			  
				//printf("Releasing layer %d frame %d\n",uiLayer,uiFrame);

				
				//S'Omple els fitxers c:/inputs/rec_X.yuv
				if(!m_pcEncoderCodingParameter[0]->isParallel()){
					printf("Write per No Parallel\n");
					xWrite  ( acPicBufferOutputList, auiProcessingInfo.nView ) ;
				}
				else
				{
					xRelease( acPicBufferOutputList, auiProcessingInfo.nView ) ;
				}
				//printf("Fem el xRelease del view %d\n",uiLayer);
				xRelease( acPicBufferUnusedList, auiProcessingInfo.nView ) ;
				//printf("Tamany del Buffer de REC[%d]=%d\n",uiLayer,acPicBufferOutputList[uiLayer].size());
				
		//}//endif
		

		
  }//endfor frame

}


void
H264AVCEncoderTest::waitForThreadFinished(int view){
	m_Thread[view].join();
}

void
H264AVCEncoderTest::processView(processingInfo	auiProcessingInfo,UInt auiPicSize, UInt uiWrittenBytes, ExtBinDataAccessorList cOutExtBinDataAccessorList, PicBuffer* apcOriginalPicBuffer, PicBuffer* apcReconstructPicBuffer, PicBufferList acPicBufferOutputList, PicBufferList acPicBufferUnusedList){

	m_Thread[m_apcProcessingInfo.nView] = boost::thread(&H264AVCEncoderTest::xProcessView,
										this,
										auiProcessingInfo,
										auiPicSize, 
										uiWrittenBytes, 
										cOutExtBinDataAccessorList, 
										apcOriginalPicBuffer, 
										apcReconstructPicBuffer,
										acPicBufferOutputList,
										acPicBufferUnusedList);

}




ErrVal
H264AVCEncoderTest::xRelease( ExtBinDataAccessorList& rcList )
{
  while( rcList.size() )
  {
    delete[] rcList.front()->data();
    delete   rcList.front();
    rcList.pop_front();
  }
  return Err::m_nOK;
}


ErrVal
H264AVCEncoderTest::go()
{
  UInt                    uiWrittenBytes[MAX_LAYERS];
  const UInt              uiMaxFrame              = m_pcEncoderCodingParameter[0]->getTotalFrames();
  UInt                    uiNumViews             =  /*(m_pcEncoderCodingParameter[0]->getMVCmode() ? 1 :*/ m_pcEncoderCodingParameter[0]->getNumberOfLayers();
  UInt                    uiFrame=0;
  UInt                    uiView;
  UInt                    uiLayer;
  UInt                    auiMbX                  [MAX_LAYERS];
  UInt                    auiMbY                  [MAX_LAYERS];
  UInt                    auiPicSize              [MAX_LAYERS];
  PicBuffer*              apcOriginalPicBuffer    [MAX_LAYERS];//original pic
  PicBuffer*              apcReconstructPicBuffer [MAX_LAYERS];
  PicBufferList			  acPicBufferOutputList   [MAX_LAYERS];//rec pic
  PicBufferList           acPicBufferUnusedList   [MAX_LAYERS];
  ExtBinDataAccessorList  cOutExtBinDataAccessorList[MAX_LAYERS];
  Bool                    bMoreSets;

   //Buffers per evitar escriure a disc ELS DEFINIREM DINS DEL GO()
  ExtBinDataAccessorList	LayerBuffer[MAX_LAYERS];
  ExtBinDataAccessorList	StartCodeBuffer[MAX_LAYERS];
 
  UInt i=0;
  UInt j=0;

  //===== initialization =====
  for(uiView=0;uiView<uiNumViews;uiView++){ 
	RNOK( m_pcH264AVCEncoder[uiView]->init( m_pcEncoderCodingParameter[uiView] ) ); 
  }
  

  string ip_adress = m_pcEncoderCodingParameter[0]->getIPAdress();
  char adress[15];
  strcpy(adress,ip_adress.c_str());
  
  m_apcRtpPacker->init(adress,m_pcEncoderCodingParameter[0]->getUDPPort(),true);
  
  m_apcRtpPacker->setPeriod((int)m_pcEncoderCodingParameter[0]->getMaximumFrameRate());


  if(isVerbose)
	printf("Inici go()\n");

  //===== write parameter sets =====
  
  for(i=0;i<uiNumViews;i++){
	  //printf("Iteracio: %d\n",j);
	  for( bMoreSets = true; bMoreSets;  )
	  {
		 //printf("Moresets\n");
		  
		UChar   aucParameterSetBuffer[1000];
		BinData cBinData;
		cBinData.reset();
		cBinData.set( aucParameterSetBuffer, 1000 );

		ExtBinDataAccessor cExtBinDataAccessor;
		cBinData.setMemAccessor( cExtBinDataAccessor );

		
		//Pot estar aqui el problema dels fitxers d'entrada?
			
		RNOK( m_pcH264AVCEncoder[i]      ->writeParameterSets( &cExtBinDataAccessor, bMoreSets) );
		
		//bMoreSets=true;
		//RNOK( m_pcH264AVCEncoder[1]      ->writeParameterSets( &cExtBinDataAccessor, bMoreSets) );

			if( m_pcH264AVCEncoder[i]->getScalableSeiMessage()&& i==0)
				{
				//printf("getScalableSeiMessage a Moresets per a Encoder %d\n",j);
				for(j=0;j<uiNumViews;j++){
					//RNOK( m_pcWriteBitstreamToFile[i]->writePacket       ( &m_cBinDataStartCode ) );
					//RNOK( m_pcWriteBitstreamToFile[i]->writePacket       ( &cExtBinDataAccessor ) );
					uiWrittenBytes[j] += 4 + cExtBinDataAccessor.size();
				}
				xWriteInit(cExtBinDataAccessor,m_pcEncoderCodingParameter[0]->isDebug());
				/*RNOK( m_pcWriteBitstreamToOutput->writePacket       ( &m_cBinDataStartCode ) );
				RNOK( m_pcWriteBitstreamToOutput->writePacket       ( &cExtBinDataAccessor ) );*/
				//OutputBuffer.push_back(&cExtBinDataAccessor);
				//if(i==2){bMoreSets=false;
			
		}
		cBinData.reset();
	  }
   bMoreSets = true;
  }

  //m_pcH264AVCEncoder[1]=m_pcH264AVCEncoder[0]; //PER AIx� APUNTA AL MATEIX PICENCODER ??????
  if(isVerbose)
	printf("Final de bulce moreSets\n------------------------\n");

//JVT-W080, PDS SEI message
  if( m_pcEncoderCodingParameter[1]->getMVCmode() && m_pcEncoderCodingParameter[1]->getPdsEnable() ){
	  if(isVerbose)
		  printf("\nJVT-W080, PDS SEI per la view 1\n");
  }
	if( m_pcEncoderCodingParameter[0]->getMVCmode() && m_pcEncoderCodingParameter[0]->getPdsEnable() )
	{
		if(isVerbose)
			printf("JVT-W080, PDS SEI\n");
		//write SEI message
		UChar   aucParameterSetBuffer[1000];
		BinData cBinData;
		cBinData.reset();
		cBinData.set( aucParameterSetBuffer, 1000 );

		ExtBinDataAccessor cExtBinDataAccessor;
		cBinData.setMemAccessor( cExtBinDataAccessor );

		const UInt uiSPSId = 0; //currently only one SPS with SPSId = 0
		//UInt uiNumView       = m_pcEncoderCodingParameter[0]->SpsMVC.getNumViewMinus1()+1;
		UInt* num_refs_list0_anc = new UInt [uiNumViews];
		UInt* num_refs_list1_anc = new UInt [uiNumViews];
		UInt* num_refs_list0_nonanc = new UInt [uiNumViews];
		UInt* num_refs_list1_nonanc = new UInt [uiNumViews];

		for(uiView = 0; uiView < uiNumViews; uiView++ )
		{
			num_refs_list0_anc[uiView]    = m_pcEncoderCodingParameter[0]->SpsMVC.getNumAnchorRefsForListX( m_pcEncoderCodingParameter[0]->SpsMVC.getViewCodingOrder()[uiView], 0 );
			num_refs_list1_anc[uiView]    = m_pcEncoderCodingParameter[0]->SpsMVC.getNumAnchorRefsForListX( m_pcEncoderCodingParameter[0]->SpsMVC.getViewCodingOrder()[uiView], 1 );
			num_refs_list0_nonanc[uiView] = m_pcEncoderCodingParameter[0]->SpsMVC.getNumNonAnchorRefsForListX( m_pcEncoderCodingParameter[0]->SpsMVC.getViewCodingOrder()[uiView], 0 );
			num_refs_list1_nonanc[uiView] = m_pcEncoderCodingParameter[0]->SpsMVC.getNumNonAnchorRefsForListX( m_pcEncoderCodingParameter[0]->SpsMVC.getViewCodingOrder()[uiView], 1 );		  
		}
//#define HELP_INFOR
#ifdef  HELP_INFOR
		printf("\n");
		for( UInt i = 0; i < uiNumView; i++ )
		{
			printf(" num_refs_list0_anchor: %d\tnum_refs_list0_nonanchor: %d\n num_refs_list1_anchor: %d\tnum_refs_list1_nonanchor: %d\n", num_refs_list0_anc[i], num_refs_list1_anc[i], num_refs_list0_nonanc[i], num_refs_list1_nonanc[i] );
		}
#endif

		UInt uiInitialPDIDelayAnc = m_pcEncoderCodingParameter[0]->getPdsInitialDelayAnc();
		UInt uiInitialPDIDelayNonAnc = m_pcEncoderCodingParameter[0]->getPdsInitialDelayNonAnc();

		if( uiInitialPDIDelayAnc < 2 )
			uiInitialPDIDelayAnc  = 2;
		if( uiInitialPDIDelayNonAnc < 2 )
			uiInitialPDIDelayNonAnc  = 2;

		for(uiView = 0; uiView < uiNumViews; uiView++ )
		{	
			if(isVerbose)
				printf("writePDSSEIMessage for view [%d]\n",uiView);
			
			RNOK( m_pcH264AVCEncoder[uiView]->writePDSSEIMessage( &cExtBinDataAccessor
			                                           , uiSPSId
			                                           , uiNumViews
			                                           , num_refs_list0_anc
																								 , num_refs_list1_anc
			                                           , num_refs_list0_nonanc
																								 , num_refs_list1_nonanc
																								 , uiInitialPDIDelayAnc
																								 , uiInitialPDIDelayNonAnc
																								) 
			);
		}

		delete[] num_refs_list0_anc;
		delete[] num_refs_list1_anc;
		delete[] num_refs_list0_nonanc;
		delete[] num_refs_list1_nonanc;
		num_refs_list0_anc = NULL;
		num_refs_list1_anc = NULL;
		num_refs_list0_nonanc = NULL;
		num_refs_list1_nonanc = NULL;
	  
		if( m_pcEncoderCodingParameter[0]->getCurentViewId() == m_pcEncoderCodingParameter[0]->SpsMVC.m_uiViewCodingOrder[0] )
		{

			if(isVerbose)
				printf("m_pcEncoderCodingParameter[0]->getCurentViewId() == m_pcEncoderCodingParameter[0]->SpsMVC.m_uiViewCodingOrder[0]\n");
			
			for(uiView=0;uiView<uiNumViews;uiView++){
				//RNOK( m_pcWriteBitstreamToFile[i]->writePacket       ( &m_cBinDataStartCode ) );
				//RNOK( m_pcWriteBitstreamToFile[i]->writePacket       ( &cExtBinDataAccessor ) );
				uiWrittenBytes[uiView] += 4 + cExtBinDataAccessor.size();
			}
			xWriteInit(cExtBinDataAccessor,m_pcEncoderCodingParameter[0]->isDebug());
			//RNOK( m_pcWriteBitstreamToOutput->writePacket       ( &m_cBinDataStartCode ) );
			//RNOK( m_pcWriteBitstreamToOutput->writePacket       ( &cExtBinDataAccessor ) );
			//OutputBuffer.push_back(&cExtBinDataAccessor);
				
			
		}

		cBinData.reset();
	}
//~JVT-W080
  //SEI {
	if( m_pcEncoderCodingParameter[1]->getMultiviewSceneInfoSEIEnable() ) // SEI JVT-W060
  {
	  // Multiview scene information sei message
	  if(isVerbose)
		  printf("getMultiviewSceneInfoSEIEnable a la view 1\n");
	}
  if( m_pcEncoderCodingParameter[0]->getMultiviewSceneInfoSEIEnable() ) // SEI JVT-W060
  {
	  // Multiview scene information sei message
	  if(isVerbose)
		  printf("getMultiviewSceneInfoSEIEnable\n");
	  
	  UChar aucParameterSetBuffer[1000];
      BinData cBinData;
      cBinData.reset();
      cBinData.set( aucParameterSetBuffer, 1000 );
      ExtBinDataAccessor cExtBinDataAccessor;
      cBinData.setMemAccessor( cExtBinDataAccessor );
	  RNOK( m_pcH264AVCEncoder[0] ->writeMultiviewSceneInfoSEIMessage( &cExtBinDataAccessor ) );
	  //Hardocejat a 2, caldr� fer-ho amb NUMLayers
		for(i=0;i<uiNumViews;i++){
		  //RNOK( m_pcWriteBitstreamToFile[i]->writePacket( &m_cBinDataStartCode ) );
		  //RNOK( m_pcWriteBitstreamToFile[i]->writePacket( &cExtBinDataAccessor ) );
		  uiWrittenBytes[i] += 4 + cExtBinDataAccessor.size();
		}
		xWriteInit(cExtBinDataAccessor,m_pcEncoderCodingParameter[0]->isDebug());
		//RNOK( m_pcWriteBitstreamToOutput->writePacket( &m_cBinDataStartCode ) );
		//RNOK( m_pcWriteBitstreamToOutput->writePacket( &cExtBinDataAccessor ) );
		//OutputBuffer.push_back(&cExtBinDataAccessor);
	  
	  cBinData.reset();
  }

  if( m_pcEncoderCodingParameter[1]->getMultiviewAcquisitionInfoSEIEnable() ) // SEI JVT-W060
  {
	  // Multiview acquisition information sei message
	  if(isVerbose)
		  printf("getMultiviewAcquisitionInfoSEIEnable a la view 1\n");
  }

  if( m_pcEncoderCodingParameter[0]->getMultiviewAcquisitionInfoSEIEnable() ) // SEI JVT-W060
  {
	  // Multiview acquisition information sei message
	  if(isVerbose)
		  printf("getMultiviewAcquisitionInfoSEIEnable\n");
	  
	  UChar aucParameterSetBuffer[1000];
      BinData cBinData;
      cBinData.reset();
      cBinData.set( aucParameterSetBuffer, 1000 );
      ExtBinDataAccessor cExtBinDataAccessor;
      cBinData.setMemAccessor( cExtBinDataAccessor );
	  RNOK( m_pcH264AVCEncoder[0] ->writeMultiviewAcquisitionInfoSEIMessage( &cExtBinDataAccessor ) );
	  //Hardocejat a 2, caldr� fer-ho amb NUMLayers
	for(i=0;i<uiNumViews;i++){
	  //RNOK( m_pcWriteBitstreamToFile[i]->writePacket( &m_cBinDataStartCode ) );
	  //RNOK( m_pcWriteBitstreamToFile[i]->writePacket( &cExtBinDataAccessor ) );
	  uiWrittenBytes[i] += 4 + cExtBinDataAccessor.size();
	}
	xWriteInit(cExtBinDataAccessor,m_pcEncoderCodingParameter[0]->isDebug());
	 //RNOK( m_pcWriteBitstreamToOutput->writePacket( &m_cBinDataStartCode ) );
	 //RNOK( m_pcWriteBitstreamToOutput->writePacket( &cExtBinDataAccessor ) );
	 //OutputBuffer.push_back(&cExtBinDataAccessor);
	  
	  
	  cBinData.reset();
  }

  if( m_pcEncoderCodingParameter[1]->getNestingSEIEnable() && m_pcEncoderCodingParameter[1]->getSnapshotEnable() 
	  && m_pcEncoderCodingParameter[1]->getCurentViewId() == 0 )
  {
   // add nesting sei message for view0
	  if(isVerbose)
		  printf("getNestingSEIEnable a la view 1\n");
  }

  if( m_pcEncoderCodingParameter[0]->getNestingSEIEnable() && m_pcEncoderCodingParameter[0]->getSnapshotEnable() 
	  && m_pcEncoderCodingParameter[0]->getCurentViewId() == 0 )
  {
   // add nesting sei message for view0
	 if(isVerbose)
		 printf("getNestingSEIEnable\n");
      UChar aucParameterSetBuffer[1000];
      BinData cBinData;
      cBinData.reset();
      cBinData.set( aucParameterSetBuffer, 1000 );
      ExtBinDataAccessor cExtBinDataAccessor;
      cBinData.setMemAccessor( cExtBinDataAccessor );
	  RNOK( m_pcH264AVCEncoder[0] ->writeNestingSEIMessage( &cExtBinDataAccessor ) );
	  //Hardocejat a 2, caldr� fer-ho amb NUMLayers
	for(i=0;i<uiNumViews;i++){
	  //RNOK( m_pcWriteBitstreamToFile[i]->writePacket( &m_cBinDataStartCode ) );
	  //RNOK( m_pcWriteBitstreamToFile[i]->writePacket( &cExtBinDataAccessor ) );
	  uiWrittenBytes[i] += 4 + cExtBinDataAccessor.size();
	}
	xWriteInit(cExtBinDataAccessor,m_pcEncoderCodingParameter[0]->isDebug());
	//RNOK( m_pcWriteBitstreamToOutput->writePacket( &m_cBinDataStartCode ) );
	//RNOK( m_pcWriteBitstreamToOutput->writePacket( &cExtBinDataAccessor ) );
	//OutputBuffer.push_back(&cExtBinDataAccessor);
	  
	  cBinData.reset();
  }
//SEI }

  //===== determine parameters for required frame buffers =====
  for( uiLayer = 0; uiLayer < uiNumViews; uiLayer++ )
  {
    //auiMbX        [uiLayer] = m_pcEncoderCodingParameter[0]->getLayerParameters( uiLayer ).getFrameWidth () >> 4;
    //auiMbY        [uiLayer] = m_pcEncoderCodingParameter[0]->getLayerParameters( uiLayer ).getFrameHeight() >> 4;
    auiMbX        [uiLayer] = m_pcEncoderCodingParameter[uiLayer]->getLayerParameters( uiLayer ).getFrameWidthInMbs();
    auiMbY        [uiLayer] = m_pcEncoderCodingParameter[uiLayer]->getLayerParameters( uiLayer ).getFrameHeightInMbs();
    m_aauiCropping[uiLayer][0]     = 0;
    m_aauiCropping[uiLayer][1]     = m_pcEncoderCodingParameter[uiLayer]->getLayerParameters( uiLayer ).getHorPadding      ();
    m_aauiCropping[uiLayer][2]     = 0;
    m_aauiCropping[uiLayer][3]     = m_pcEncoderCodingParameter[uiLayer]->getLayerParameters( uiLayer ).getVerPadding      ();
    
	if(!m_pcEncoderCodingParameter[uiLayer]->isParallel()){
		m_apcWriteYuv[uiLayer]->setCrop(m_aauiCropping[uiLayer]);
	}

    UInt  uiSize            = ((auiMbY[uiLayer]<<4)+2*YUV_Y_MARGIN)*((auiMbX[uiLayer]<<4)+2*YUV_X_MARGIN);
    auiPicSize    [uiLayer] = ((auiMbX[uiLayer]<<4)+2*YUV_X_MARGIN)*((auiMbY[uiLayer]<<4)+2*YUV_Y_MARGIN)*3/2;
    m_auiLumOffset[uiLayer] = ((auiMbX[uiLayer]<<4)+2*YUV_X_MARGIN)* YUV_Y_MARGIN   + YUV_X_MARGIN;  
    m_auiCbOffset [uiLayer] = ((auiMbX[uiLayer]<<3)+  YUV_X_MARGIN)* YUV_Y_MARGIN/2 + YUV_X_MARGIN/2 + uiSize; 
    m_auiCrOffset [uiLayer] = ((auiMbX[uiLayer]<<3)+  YUV_X_MARGIN)* YUV_Y_MARGIN/2 + YUV_X_MARGIN/2 + 5*uiSize/4;
    m_auiHeight   [uiLayer] =   auiMbY[uiLayer]<<4;
    m_auiWidth    [uiLayer] =   auiMbX[uiLayer]<<4;
    m_auiStride   [uiLayer] =  (auiMbX[uiLayer]<<4)+ 2*YUV_X_MARGIN;

	//printf("Dades dels parametres del Layer %d: auiMbX=%d auiMbY=%d\n",uiLayer,auiMbX,auiMbY);
  }

 
  

  //
  //Auqui ja pot anar la primera escriptura a oputput.264
  //

  //===== loop over frames =====
  printf("\n---------------------\nPreparation of the Encoder is finsihed. Let's start with the encoding.\n---------------------\n");
  printf("Total Frames: %d\n---------------------\n\n",uiMaxFrame);
system("pause");
printf("\n");

 // boost::thread workerThread(&H264AVCEncoderTest::xProcessingThread);
 // boost::thread workerThread(&xProcessingThread);

  //workerThread.join();

  //start(0,uiFrame,uiMaxFrame,uiLayer,auiPicSize[uiLayer],uiWrittenBytes[uiLayer],7,8);
  //start(1);
  //join(0);
  //join(1);

    for( uiLayer = 0; uiLayer < uiNumViews; uiLayer++ )
    {
		if(isVerbose)
			printf("Prova de processar la view %d en una funcio a part.\n",uiLayer);
		
		xSetProcessingInfo(uiFrame,uiMaxFrame,uiLayer);
		processView(m_apcProcessingInfo,auiPicSize[uiLayer],uiWrittenBytes[uiLayer],cOutExtBinDataAccessorList[uiLayer],apcOriginalPicBuffer[uiLayer],apcReconstructPicBuffer[uiLayer],acPicBufferOutputList[uiLayer],acPicBufferUnusedList[uiLayer]);
		
		//system("pause");
	  }

	//printf("pause");
	join();
	

	if(isVerbose){
		printf("Han acabat els dos threads\n ");
		printf("pause");
	}
  
//Tot aix� va dins del thread processView()
 // for( uiFrame = 0; uiFrame < uiMaxFrame; uiFrame++ )
 // {
	//  m_apcRtpPacker->increaseTimeStamp();
	//  printf("\nFrame: %d\n",uiFrame);
	//   //system("pause");
	//  
 //   //===== get picture buffers and read original pictures =====
 //   for( uiLayer = 0; uiLayer < uiNumViews; uiLayer++ )
 //   {
	//	//if(uiFrame>=3||uiLayer==0){ //La condici� uiFrame>2 hem de deshardcodejar-ho per RecPicBuffer->uiMaxFramesInDPB
	//		//printf("\n\n//////////\nFIns a la frame 2 no imprimirem la Layer %d\n///////////\n\n",uiLayer);
	//	
	//	
	//		  UInt  uiSkip = ( 1 << m_pcEncoderCodingParameter[uiLayer]->getLayerParameters( 0 ).getTemporalResolution() );
	//		  //UInt  uiSkip = ( 1 << m_pcEncoderCodingParameter[uiLayer]->getLayerParameters( uiLayer ).getTemporalResolution() );
	//		  
	//		  //
	//		  //LLEGIM EL FRAME uiFrame PER LA VISTA uiLayer
	//		  //

	//		  if( uiFrame % uiSkip == 0 )
	//		  {
	//			RNOK( xGetNewPicBuffer( apcReconstructPicBuffer [uiLayer], uiLayer, auiPicSize[uiLayer] ) );
	//			RNOK( xGetNewPicBuffer( apcOriginalPicBuffer    [uiLayer], uiLayer, auiPicSize[uiLayer] ) );
	//			
	//			//printf("Reading Layer %d of frame %d\n",uiLayer,uiFrame);
	//			//m_apcReadYuv[uiLayer]->m_cFile.tell();
	//			RNOK( m_apcReadYuv[uiLayer]->readFrame( *apcOriginalPicBuffer[uiLayer] + m_auiLumOffset[uiLayer],
	//													*apcOriginalPicBuffer[uiLayer] + m_auiCbOffset [uiLayer],
	//													*apcOriginalPicBuffer[uiLayer] + m_auiCrOffset [uiLayer],
	//													m_auiHeight [uiLayer],
	//													m_auiWidth  [uiLayer],
	//													m_auiStride [uiLayer] ) );

	//			//printf("Frame %d, Layer %d, tamany original:%s\n",uiFrame,uiLayer,apcOriginalPicBuffer[uiLayer]);
	//			
	//		  }
	//		  else
	//		  {
	//			if(isVerbose)
	//				printf("Hi ha Hagut un SKIP a la part de readFrame()\n");

	//			apcReconstructPicBuffer [uiLayer] = 0;
	//			apcOriginalPicBuffer    [uiLayer] = 0;		
	//		  }
	//		  

	//		  //
	//		  //PROCESSEM EL FRAME uiFrame PER LA VISTA uiLayer
	//		  //

	//		  if(isVerbose)
	//			  printf("View %d\t",uiLayer);

	//		   RNOK( m_pcH264AVCEncoder[uiLayer]->process( cOutExtBinDataAccessorList[uiLayer],
	//										   apcOriginalPicBuffer[uiLayer],
	//										   apcReconstructPicBuffer[uiLayer],
	//										   &acPicBufferOutputList[uiLayer],
	//										   &acPicBufferUnusedList[uiLayer] ) );


	//		   //
	//		   //ESCRIVIM EL FRAME uiFrame PER LA VISTA uiLayer A DIFERENTS ARXIUS I BUFFERS(OUTPUT, REC, ETC...)
	//		   //

	//			//printf("Writing layer %d frame %d\n",uiLayer,uiFrame);
	//			UInt  uiBytesUsed = 0;
	//			if(m_pcEncoderCodingParameter[0]->isDebug()){
	//				if(isVerbose)
	//					printf("Write per debug\n");				
	//				RNOK( xWrite  ( cOutExtBinDataAccessorList[uiLayer],uiBytesUsed) );
	//			}
	//			else{
	//				RNOK(xSend(cOutExtBinDataAccessorList[uiLayer]));
	//				
	//			}
	//			
	//			//m_apcUDPController->send("Test");
	//			
	//					
	//			uiWrittenBytes[uiLayer]   += uiBytesUsed;

	//		  
	//			//printf("Releasing layer %d frame %d\n",uiLayer,uiFrame);

	//			
	//			//S'Omple els fitxers c:/inputs/rec_X.yuv
	//			if(!m_pcEncoderCodingParameter[0]->isParallel()){
	//				printf("Write per No Parallel\n");
	//				RNOK( xWrite  ( acPicBufferOutputList[uiLayer], uiLayer ) );
	//			}
	//			else
	//			{
	//				RNOK( xRelease( acPicBufferOutputList[uiLayer], uiLayer ) );
	//			}
	//			//printf("Fem el xRelease del view %d\n",uiLayer);
	//			RNOK( xRelease( acPicBufferUnusedList[uiLayer], uiLayer ) );
	//			//printf("Tamany del Buffer de REC[%d]=%d\n",uiLayer,acPicBufferOutputList[uiLayer].size());
	//			
	//	//}//endif
	//	
	//	
	//		
	//}//endfor vista

	//		//
	//		//Despr�s de processar dues vistes. Augmentem el Timestamp
	//		//

	//
	//
	//	
 // }//endfor frame


  //escriure a output al final de tot
  //while(LayerBuffer[0].size()&&LayerBuffer[1].size())
  //{
	 // for( uiLayer = 0; uiLayer < uiNumViews; uiLayer++ )
	 // {
		//  
	 // }
  //}
	
  //Comparem tamany del buffer de Output

  if(isVerbose){
	  printf("Tamany del Buffer de REC[0]=%d\n",acPicBufferOutputList[0].size());
	  printf("Tamany del Buffer de REC[1]=%d\n",uiLayer,acPicBufferOutputList[1].size());
	  printf("Tamany del Buffer de acPicBufferUnusedList[0]=%d\n",acPicBufferUnusedList[0].size());
	  printf("Tamany del Buffer de acPicBufferUnusedList[1]=%d\n\n",acPicBufferUnusedList[1].size());
  }

    //Enviar missatge de final de Transmissi�
  /*printf("Enviem el final de Transmissi�\n");
  
  m_apcRtpPacker->endTransmission();*/

  //===== finish encoding =====
  for( uiLayer = 0; uiLayer < uiNumViews; uiLayer++ )
  {
	  UInt  uiNumCodedFrames = 0;
	  Double  dHighestLayerOutputRate = 0.0;
	  if(isVerbose)
		  printf("Finishing encoding view %d\n",uiLayer);

	  RNOK( m_pcH264AVCEncoder[uiLayer]->finish( cOutExtBinDataAccessorList[uiLayer],
										acPicBufferOutputList,
										acPicBufferUnusedList,
										uiNumCodedFrames,
										dHighestLayerOutputRate ) );


	  //===== write and release NAL unit buffers =====
	  if(m_pcEncoderCodingParameter[0]->isDebug()){
		  RNOK( xWrite  ( cOutExtBinDataAccessorList[uiLayer], uiWrittenBytes[uiLayer]) );
	  }
	  else{
		  RNOK(xSend(cOutExtBinDataAccessorList[uiLayer]));
	  }

  

  }

 
  


  //printf("Tamany del Buffer de REC[0]=%d\n",acPicBufferOutputList[0].size());
  //printf("Tamany del Buffer de REC[1]=%d\n",acPicBufferOutputList[1].size());
  //printf("Tamany del Buffer de acPicBufferUnusedList[0]=%d\n",acPicBufferUnusedList[0].size());
  //printf("Tamany del Buffer de acPicBufferUnusedList[1]=%d\n",acPicBufferUnusedList[1].size());

  for( uiLayer = 0; uiLayer < uiNumViews; uiLayer++ )
  {
	  if(isVerbose)
		  printf("Release dels auxiliars\n");
	//printf("Tamany del Buffer de REC[%d]=%d\n",uiLayer,acPicBufferOutputList[uiLayer].size());

	//printf("Releasing Oputput and Unused Buffers for view %d\n",uiLayer);
    if(!m_pcEncoderCodingParameter[0]->isParallel()){
		RNOK( xWrite  ( acPicBufferOutputList[uiLayer], uiLayer ) );
		//printf("xWrite fet\n");
	}
	
	//printf("Tamany del Buffer de acPicBufferUnusedList[%d]=%d\n",uiLayer,acPicBufferUnusedList[uiLayer].size());
    RNOK( xRelease( acPicBufferUnusedList[uiLayer], uiLayer ) );
	if(isVerbose)
		printf("xRelease[%d] fet\n",uiLayer);
  }

  //printf("Set parameters");
  //===== set parameters and output summary =====
  m_cEncoderIoParameter.nFrames = uiFrame;
  m_cEncoderIoParameter.nResult = 0;

  for( uiLayer = 0; uiLayer < uiNumViews; uiLayer++ ){
	  if(isVerbose)
		  printf("Segon bucle\n");
	if( ! m_pcEncoderCodingParameter[uiLayer]->getMVCmode() )
	{
		if(isVerbose)
			printf("Entrem al bucle\n");
		//printf("m_pcEncoderCodingParameter[uiLayer]->getMVCmode()\n");
		UChar   aucParameterSetBuffer[1000];
		BinData cBinData;
		cBinData.reset();
		cBinData.set( aucParameterSetBuffer, 1000 );

		ExtBinDataAccessor cExtBinDataAccessor;
		cBinData.setMemAccessor( cExtBinDataAccessor );
		m_pcH264AVCEncoder[uiLayer]->SetVeryFirstCall();
		RNOK( m_pcH264AVCEncoder[uiLayer]      ->writeParameterSets( &cExtBinDataAccessor, bMoreSets) );
		//Hardocejat a 2, caldr� fer-ho amb NUMLayers
		
		for(i = 0; i < uiNumViews; i++){
			//RNOK( m_pcWriteBitstreamToFile[uiLayer]->writePacket       ( &m_cBinDataStartCode ) );
			//RNOK( m_pcWriteBitstreamToFile[uiLayer]->writePacket       ( &cExtBinDataAccessor ) );
			uiWrittenBytes[i] += 4 + cExtBinDataAccessor.size();
		}
		
		xWriteInit(cExtBinDataAccessor,m_pcEncoderCodingParameter[0]->isDebug());
		//RNOK( m_pcWriteBitstreamToOutput->writePacket       ( &m_cBinDataStartCode ) );
		//RNOK( m_pcWriteBitstreamToOutput->writePacket       ( &cExtBinDataAccessor ) );
	
		cBinData.reset();
	}
  }
//SEI {
  for( uiLayer = 0; uiLayer < uiNumViews; uiLayer++ ){
	    
	  if( m_pcEncoderCodingParameter[uiLayer]->getViewScalInfoSEIEnable() )
	  {
		  
		 //printf("m_pcEncoderCodingParameter[uiLayer]->getViewScalInfoSEIEnable()\n");
		//view scalability information sei message
		 UChar   aucParameterSetBuffer[1000];
		 BinData cBinData;
		 cBinData.reset();
		 cBinData.set( aucParameterSetBuffer, 1000 );

		 ExtBinDataAccessor cExtBinDataAccessor;
		 cBinData.setMemAccessor( cExtBinDataAccessor );
		 RNOK( m_pcH264AVCEncoder[uiLayer]->writeViewScalInfoSEIMessage( &cExtBinDataAccessor ) );
		 //Hardocejat a 2, caldr� fer-ho amb NUMLayers
		for(i = 0; i < uiNumViews; i++){
		 //RNOK( m_pcWriteBitstreamToFile[uiLayer]->writePacket       ( &m_cBinDataStartCode ) );
		 //RNOK( m_pcWriteBitstreamToFile[uiLayer]->writePacket       ( &cExtBinDataAccessor ) );
		 uiWrittenBytes[i] += 4 + cExtBinDataAccessor.size();
		}
		xWriteInit(cExtBinDataAccessor,m_pcEncoderCodingParameter[0]->isDebug());
		 //RNOK( m_pcWriteBitstreamToOutput->writePacket       ( &m_cBinDataStartCode ) );
		 //RNOK( m_pcWriteBitstreamToOutput->writePacket       ( &cExtBinDataAccessor ) );
		 
		 cBinData.reset();

	  }
  }
//SEI }
  
  if( m_pcEncoderCodingParameter[0]->isDebug() )
  {
	   
	 //printf("m_pcWriteBitstreamToOutput\n");
    /*RNOK( m_pcWriteBitstreamToFile[0]->uninit() );  
    RNOK( m_pcWriteBitstreamToFile[0]->destroy() );
	RNOK( m_pcWriteBitstreamToFile[1]->uninit() );  
    RNOK( m_pcWriteBitstreamToFile[1]->destroy() );*/
	RNOK( m_pcWriteBitstreamToOutput->uninit() );  
	//printf("m_pcWriteBitstreamToOutput-uninit()\n");
    RNOK( m_pcWriteBitstreamToOutput->destroy() );
	//printf("m_pcWriteBitstreamToOutput-destroy()\n");
  }

//SEI {
  if(isVerbose)
	  printf("Check m_pcEncoderCodingParameter[0]->getViewScalInfoSEIEnable()\n");
  
  if( m_pcEncoderCodingParameter[0]->getViewScalInfoSEIEnable() )
  {
	    if(isVerbose)
			printf("ViewScalableDealing\n");
	  //printf("m_pcEncoderCodingParameter[0]->getViewScalInfoSEIEnable()\n");
    RNOK    ( ViewScalableDealing() );
  }
//SEI }
   if(isVerbose)
	   printf("Check m_pcEncoderCodingParameter[0]->getMVCmode()\n");
  if( ! m_pcEncoderCodingParameter[0]->getMVCmode() )
  {
	  
	  if(isVerbose)
		  printf("ScalableDealing\n");
	  //printf("m_pcEncoderCodingParameter[0]->getMVCmode()\n");
	RNOK	( ScalableDealing() );
  }

  //==== TAncar Assemlber =====
	//RNOKR( pcAssembler->destroy (),               -6 );



if(!m_pcEncoderCodingParameter[0]->isDebug())
	m_apcRtpPacker->destroy();


  return Err::m_nOK;
}

ErrVal
H264AVCEncoderTest::ScalableDealing()
{
  FILE *ftemp = fopen( m_cWriteToBitFileTempName.c_str(), "rb" );
  FILE *f     = fopen( m_cWriteToBitFileName.c_str    (), "wb" );

	UChar pvBuffer[4];

	fseek( ftemp, SEEK_SET, SEEK_END );
	long lFileLength = ftell( ftemp );

	long lpos = 0;
	long loffset = -5;	//start offset from end of file
	Bool bMoreSets = true;
	do {
		fseek( ftemp, loffset, SEEK_END);
		fread( pvBuffer, 1, 4, ftemp );
		if( pvBuffer[0] == 0 && pvBuffer[1] == 0 && pvBuffer[2] == 0 && pvBuffer[3] == 1)
		{
			bMoreSets = false;
			lpos = abs( loffset );
		}
		else
		{
			loffset --;
		}
	} while( bMoreSets );

	fseek( ftemp, loffset, SEEK_END );

	UChar *pvChar = new UChar[lFileLength];
	fread( pvChar, 1, lpos, ftemp );
	fseek( ftemp, 0, SEEK_SET );
	fread( pvChar+lpos, 1, lFileLength-lpos, ftemp);
	fclose(ftemp);
	fflush(ftemp);
	fwrite( pvChar, 1, lFileLength, f);	
	delete pvChar;
	fclose(f);
	fflush(f);
	RNOK( remove( m_cWriteToBitFileTempName.c_str() ) ); 

	return Err::m_nOK;
}


//SEI {
ErrVal
H264AVCEncoderTest::ViewScalableDealing()
{
  FILE *ftemp = fopen( m_cWriteToBitFileTempName.c_str(), "rb" );
  FILE *f     = fopen( m_cWriteToBitFileName.c_str    (), "wb" );

	UChar pvBuffer[4];

	fseek( ftemp, SEEK_SET, SEEK_END );
	long lFileLength = ftell( ftemp );

	long lpos = 0;
	long loffset = -5;	//start offset from end of file
	Bool bMoreSets = true;
	do {
		fseek( ftemp, loffset, SEEK_END);
		fread( pvBuffer, 1, 4, ftemp );
		if( pvBuffer[0] == 0 && pvBuffer[1] == 0 && pvBuffer[2] == 0 && pvBuffer[3] == 1)
		{
			bMoreSets = false;
			lpos = abs( loffset );
		}
		else
		{
			loffset --;
		}
	} while( bMoreSets );

	fseek( ftemp, loffset, SEEK_END );

	UChar *pvChar = new UChar[lFileLength];
	fread( pvChar, 1, lpos, ftemp );
	fseek( ftemp, 0, SEEK_SET );
	fread( pvChar+lpos, 1, lFileLength-lpos, ftemp);
	fclose(ftemp);
	fflush(ftemp);
	fwrite( pvChar, 1, lFileLength, f);	
	delete pvChar;
	fclose(f);
	fflush(f);
	RNOK( remove( m_cWriteToBitFileTempName.c_str() ) ); 

	return Err::m_nOK;
}

//SEI }
