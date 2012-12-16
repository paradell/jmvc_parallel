#ifndef __H264AVCDECODERTEST_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79
#define __H264AVCDECODERTEST_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79


#include "ReadBitstreamFile.h"
#include "WriteYuvToFile.h"
#include "RtpPacker.h"
#include "boost\thread.hpp"
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>


#define MAX_REFERENCE_FRAMES 15
#define MAX_B_FRAMES         15

#include <algorithm>
#include <list>

#include "DecoderParallelParameter.h"



class H264AVCDecoderTest  
{
protected:
	H264AVCDecoderTest();
	virtual ~H264AVCDecoderTest();

public:
  static ErrVal create( H264AVCDecoderTest*& rpcH264AVCDecoderTest );
  ErrVal init( DecoderParallelParameter *pcDecoderParameter, WriteYuvToFile *pcWriteYuv, ReadBitstreamFile *pcReadBitstreamFile );//TMM_EC
  ErrVal go();
  ErrVal destroy();
  ErrVal setec( UInt uiErrorConceal);//TMM_EC

  ErrVal setCrop();

  bool finished;

  typedef boost::mutex::scoped_lock scoped_lock;

  boost::thread RecieveThread;
  boost::thread UnpackThread;

  boost::thread popMvcThread;

  boost::condition rtpSignal; //Condició que controla el accés al buffer RTP
  boost::condition mvcSignal; //Condició que controla el accés al buffer MVC

  boost::mutex rtp_mutex; //mutex que controla el accés al buffer RTP
  boost::mutex mvc_mutex; //mutex que controla el accés al buffer MVC

  boost::mutex io_mutex; //mutex general
  

private:
  void	xRecieveRTP			();
  void	xUnPack				();

protected:
	//void xRecievePacket();
	//void xUnPack();
  ErrVal xGetNewPicBuffer ( PicBuffer*& rpcPicBuffer, UInt uiSize );
  ErrVal xRemovePicBuffer ( PicBufferList& rcPicBufferUnusedList );

  ErrVal resetElement(DecoderElement* element);



  //ErrVal xRecievePacket		( RtpPacker* packer, BinData*& rpcBinData,Bool& rbEOS );
  ErrVal xRecievePacket		( BinData*& rpcBinData,Bool& rbEOS );
  
  ErrVal xReleasePacket		( BinData* pcBinData );

protected:
  h264::CreaterH264AVCDecoder*   m_pcH264AVCDecoder;
  h264::CreaterH264AVCDecoder*   m_pcH264AVCDecoderSuffix; //JVT-S036 lsj
  ReadBitstreamIf*            m_pcReadBitstream;
  WriteYuvIf*                 m_pcWriteYuv;
  WriteYuvIf*				  m_pcWriteSnapShot;   //SEI LSJ
  DecoderParallelParameter*           m_pcParameter;
  RtpPacker*				  m_pcRtpPacker;
	
  PicBufferList               m_cActivePicBufferList;
  PicBufferList               m_cUnusedPicBufferList;

  bool Parallel;
  bool isVerbose;
  
};

#endif //__H264AVCDECODERTEST_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79
