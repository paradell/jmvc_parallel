#ifndef __H264AVCENCODERTEST_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79
#define __H264AVCENCODERTEST_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79

#include <algorithm>
#include <list>

#include "WriteBitstreamToFile.h"
#include "ReadYuvFile.h"
#include "WriteYuvToFile.h"
#include "RtpPacker.h"
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>


//#include "UDPController.h"




class EncoderCodingParameter;



typedef struct
{
  UInt    uiNumberOfLayers;
  std::string cBitstreamFilename[MAX_LAYERS];
  Int     nResult; 
  UInt    nFrames;
} EncoderIoParameter;

typedef struct
{
	UInt nView;
	UInt nFrame;
	UInt nMaxFrames;
}processingInfo;


 
class H264AVCEncoderTest  
{
private:
	H264AVCEncoderTest();
	virtual ~H264AVCEncoderTest();

public:
  static ErrVal create( H264AVCEncoderTest*& rpcH264AVCEncoderTest );
  
  ErrVal init     ( Int     argc,
                    Char**  argv );
  ErrVal go       ();
  ErrVal destroy  ();
  ErrVal ScalableDealing ();
  ErrVal ViewScalableDealing ();//SEI LSJ

  void start(int view,UInt uiFrame, UInt uiMaxFrame,UInt uiLayer,UInt auiPicSize, UInt uiWrittenBytes, int v7, int v8);
  void join();

  void processView(			processingInfo	auiProcessingInfo,
							UInt auiPicSize,
							UInt uiWrittenBytes,
							ExtBinDataAccessorList cOutExtBinDataAccessorList,
							PicBuffer* apcOriginalPicBuffer,
							PicBuffer* apcReconstructPicBuffer,
							PicBufferList acPicBufferOutputList,
							PicBufferList acPicBufferUnusedList);

  void waitForThreadFinished(int view);
  
private:
  typedef boost::mutex::scoped_lock lock;

  boost::thread m_Thread[MAX_LAYERS];
  boost::thread_group group_threads;
  boost::mutex io_mutex;
  boost::mutex monitor;
  boost::condition free_thread[MAX_LAYERS];

  ErrVal  xGetNewPicBuffer( PicBuffer*&             rpcPicBuffer,
                            UInt                    uiLayer,
                            UInt                    uiSize );
  ErrVal  xRemovePicBuffer( PicBufferList&          rcPicBufferUnusedList,
                            UInt                    uiLayer );

  ErrVal	xWrite			( ExtBinDataAccessorList& rcList,
                            UInt&                   ruiBytesInFrame
							);

  ErrVal	xWriteInit		( ExtBinDataAccessor rcList,
							bool debug
							);

  ErrVal  xRelease			( ExtBinDataAccessorList& rcList );

  void		xProcessView(	processingInfo	auiProcessingInfo,
							UInt auiPicSize,
							UInt uiWrittenBytes,
							ExtBinDataAccessorList cOutExtBinDataAccessorList,
							PicBuffer* apcOriginalPicBuffer,
							PicBuffer* apcReconstructPicBuffer,
							PicBufferList acPicBufferOutputList,
							PicBufferList acPicBufferUnusedList );

  void testThread(int view,UInt uiFrame, UInt uiMaxFrame,UInt uiLayer,UInt auiPicSize, UInt uiWrittenBytes, int v7, int v8);
  
  ErrVal  xWrite          ( PicBufferList&          rcList,
                            UInt                    uiLayer );
  ErrVal  xRelease        ( PicBufferList&          rcList,
                            UInt                    uiLayer );

  ErrVal xAskForSend	( ExtBinDataAccessorList& rcList, 
							UInt nView, 
							UInt nFrame);

 ErrVal  xSend				(ExtBinDataAccessorList& rcList);
 ErrVal  xSend				( ExtBinDataAccessor cExtBinDataAccessor);

 ErrVal	xSetProcessingInfo	(UInt Frame,UInt MaxFrames,UInt View);
  

protected:
  EncoderIoParameter            m_cEncoderIoParameter;
  EncoderCodingParameter*       m_pcEncoderCodingParameter[MAX_LAYERS];
  h264::CreaterH264AVCEncoder*  m_pcH264AVCEncoder[MAX_LAYERS];
  //WriteBitstreamToFile*         m_pcWriteBitstreamToFile[MAX_LAYERS];
  WriteBitstreamToFile*         m_pcWriteBitstreamToOutput;
  WriteYuvIf*                   m_apcWriteYuv           [MAX_LAYERS];
  ReadYuvFile*                  m_apcReadYuv            [MAX_LAYERS];
  RtpPacker*					m_apcRtpPacker;

  UInt							nFreeThread; //This variable tells what thread have access to RtpPacker

  processingInfo				m_apcProcessingInfo;

  Bool							isVerbose;
  //UDPController*				m_apcUDPController;

  //Buffers per evitar escriure a disc ELS DEFINIREM DINS DEL GO()
  //ExtBinDataAccessorList	LayerBuffer[MAX_LAYERS];
  //ExtBinDataAccessorList	OutputBuffer;

  PicBufferList                 m_acActivePicBufferList [MAX_LAYERS];
  PicBufferList                 m_acUnusedPicBufferList [MAX_LAYERS];
  UInt                          m_auiLumOffset          [MAX_LAYERS];
  UInt                          m_auiCbOffset           [MAX_LAYERS];
  UInt                          m_auiCrOffset           [MAX_LAYERS];
  UInt                          m_auiHeight             [MAX_LAYERS];
  UInt                          m_auiWidth              [MAX_LAYERS];
  UInt                          m_auiStride             [MAX_LAYERS];
  UInt                          m_aauiCropping          [MAX_LAYERS][4];

  UChar                         m_aucStartCodeBuffer[5];
  BinData                       m_cBinDataStartCode;
  std::string                   m_cWriteToBitFileName;
  std::string                   m_cWriteToBitFileTempName;
};




#endif //__H264AVCENCODERTEST_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79
