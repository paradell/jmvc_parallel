#if !defined(AFX_RTPPACKER_H__CBFF413E_29A2_B38C_97F1_44E35C83507D__INCLUDED_)
#define AFX_RTPPACKER_H__CBFF413E_29A2_B38C_97F1_44E35C83507D__INCLUDED_

#include <sys/stat.h>
#include <sys/types.h>
//#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif

class H264AVCCOMMONLIB_API RtpPacker{

protected:
	RtpPacker();
	virtual ~RtpPacker();

public:
  static ErrVal create( RtpPacker*& RtpPacker );
  ErrVal destroy();

  ErrVal init();
  ErrVal uninit();

  ErrVal pack();
  ErrVal unpack();

private:
  ErrVal xFragment();
  ErrVal xDefragment();
};

#endif