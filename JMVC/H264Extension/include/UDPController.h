#if !defined(AFX_UDPCONTROLLER_H__CBFF413E_29A2_B38C_97F1_44E35C83507D__INCLUDED_)
#define AFX_UDPVPNTROLLER_H__CBFF413E_29A2_B38C_97F1_44E35C83507D__INCLUDED_

#pragma comment(lib, "ws2_32.lib")

#include <sys/stat.h>
#include <sys/types.h>
//#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "inet.h"

#include <winsock2.h>

#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif

class H264AVCVIDEOIOLIB_API UDPController
{

protected:
	UDPController();
	virtual ~UDPController();

public:
  static ErrVal create( UDPController*& UDPController );
  ErrVal destroy();

  ErrVal initSender(char* confAdress, int confPort);
  ErrVal initReciever(int confPort);

  ErrVal setSocket();
  ErrVal uninit();

  ErrVal checkServer();
  ErrVal send(UChar* rtpPacket,int size);
  int recieve(UChar rtpPacket[], int packetSize);

private:
	//DAta
	WSADATA wsaData;

	SOCKET SendingSocket;

	SOCKET RecieveSocket;

	SOCKADDR_IN sender_adress;
	SOCKADDR_IN reciever_adress;

	int port;

	int reciever_length;
	int sender_length;

	int recvfromTimeOutUDP(SOCKET socket, long sec, long usec);



};

#endif