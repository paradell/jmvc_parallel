#include <cstdio>
#include <stdlib.h>
#include "H264AVCVideoIoLib.h"
#include "UDPUnixController.h"

#define IP_ADRESS inet_addr( "192.168.1.128" )
#define PORT 5150


UDPUnixController::UDPUnixController(){};


UDPUnixController::~UDPUnixController()
{
}

ErrVal
UDPUnixController::create( UDPUnixController*& rpcUDPUnixController ){
	UDPUnixController* pcUDPUnixController;

   pcUDPUnixController = new UDPUnixController;
   rpcUDPUnixController = pcUDPUnixController;
	printf("UDPUnixController::create()\n");  
	return Err::m_nOK;
}
 

ErrVal
UDPUnixController::destroy(){
	 // When your application is finished call WSACleanup.

   printf("Client: Cleaning up...\n");

   if(WSACleanup() != 0)

        printf("Client: WSACleanup() failed! Error code: %ld\n", WSAGetLastError());

   else

        printf("Client: WSACleanup() is OK\n");

	//printf("UDPUnixController::destroy()\n");  
	return Err::m_nOK;
}

ErrVal
UDPUnixController::initReciever(int confPort){
	
	sender_length = sizeof(sender_adress);
	UChar mess[20];
		// Initialize Winsock version 2.2

   if( WSAStartup(MAKEWORD(2,2), &wsaData) != 0)

   {

        printf("Server: WSAStartup failed with error %ld\n", WSAGetLastError());

        return -1;

   }

   else

         // printf("Server: The Winsock DLL status is %s.\n", wsaData.szSystemStatus);

 

     // Create a new socket to receive datagrams on.

     RecieveSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

 

     // The IPv4 family

     reciever_adress.sin_family = AF_INET;

     // Port no. 5150

     reciever_adress.sin_port = htons(confPort);

     // From all interface (0.0.0.0)

     reciever_adress.sin_addr.s_addr = htonl(INADDR_ANY);

 

   // Associate the address information with the socket using bind.

   // At this point you can receive datagrams on your bound socket.

   if (bind(RecieveSocket, &reciever_adress, sizeof(reciever_adress)) == SOCKET_ERROR)

   {

        printf("Server: bind() failed! Error:\n");

        // Close the socket

        close(RecieveSocket);



        return -1;

     }

     else

          //printf("Server: bind() is OK!\n");

 

   // Some info on the receiver side...

   getsockname(RecieveSocket, (SOCKADDR *)&reciever_adress, (int *)sizeof(reciever_adress));

 

   //printf("Server: Receiving IP(s) used: %s\n", inet_ntoa(reciever_adress.sin_addr));

   //printf("Server: Receiving port used: %d\n", htons(reciever_adress.sin_port));

   printf("Server: I\'m ready to receive a datagram...\n");


    recieve(mess,20);
	
    
   
   

  return Err::m_nOK;
}

ErrVal
UDPUnixController::initSender(char* confAdress, int confPort){


	

	// Create a new socket to receive datagrams on.

     SendingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

     if (SendingSocket == INVALID_SOCKET)

     {

          printf("Client: Error at socket()\n");

   
          // Exit with error

          return -1;

     }

     else

          printf("Client: socket() is OK!\n");


	//Define Internet adress
    reciever_adress.sin_family = AF_INET;
    reciever_adress.sin_addr.s_addr = inet_addr( confAdress );
	port = confPort;
    reciever_adress.sin_port = htons(port);

	//
	//server_length = sizeof(struct sockaddr_in);

	

	//connect /bind

	//printf("Prepare the Server\n");
	//system("pause");
	checkServer();
	system("pause");
	

	//printf("UDPUnixController::init()\n");  
	return Err::m_nOK;
 }

ErrVal
UDPUnixController::setSocket(){
	
	sender_length = sizeof(sender_adress);

	reciever_adress.sin_addr.s_addr = htonl(INADDR_ANY);

	 if(bind(RecieveSocket, (SOCKADDR*) &reciever_adress, sizeof(reciever_adress)) == SOCKET_ERROR)
    {
        printf("ServerSocket: Failed to connect\n");
		// Close the socket

        closesocket(RecieveSocket);

        // Do the clean up

        WSACleanup();

        //system("pause");
        exit(-1);
    }

	 // printf("Server: bind() is OK!\n");

	  // Some info on the receiver side...

   getsockname(RecieveSocket, (SOCKADDR *)&reciever_adress, (int *)sizeof(reciever_adress));

 

   //printf("Server: Receiving IP(s) used: %s\n", inet_ntoa(reciever_adress.sin_addr));

   //("Server: Receiving port used: %d\n", htons(reciever_adress.sin_port));

   //printf("Server: I\'m ready to receive a datagram...\n");

	 return Err::m_nOK; 
}
int
UDPUnixController::recvfromTimeOutUDP(SOCKET socket, long sec, long usec){

	// Setup timeval variable

    struct timeval timeout;

     struct fd_set fds;

 

    timeout.tv_sec = sec;

    timeout.tv_usec = usec;

    // Setup fd_set structure

    FD_ZERO(&fds);

    FD_SET(socket, &fds);

    // Return value:

    // -1: error occurred

    // 0: timed out

    // > 0: data ready to be read

    return select(0, &fds, 0, 0, &timeout);
}

ErrVal
UDPUnixController::uninit(){
	WSACleanup();
	//printf("UDPUnixController::uninit()\n");  
	return Err::m_nOK;
 }

ErrVal
UDPUnixController::checkServer(){

	
	int size=0;

	sendto(SendingSocket, "Server is Alive..?" , 20, 0, (SOCKADDR*) &reciever_adress, sizeof(reciever_adress));

		 
	return Err::m_nOK;
}

ErrVal
UDPUnixController::send(UChar* rtpPacket,int size){
	int sent=0;

	
	sent=sendto(SendingSocket,(char*) rtpPacket , size, 0, (SOCKADDR*) &reciever_adress, sizeof(reciever_adress));


	//printf("Dades que enviem al send tipus UCHAR: ");

	//  for(UInt i=0;i<min(size,30);i++)
	//	  printf("%X ",rtpPacket[i]);

	//  printf("\n");

	//printf("\n%d UDP payload bytes sent.\n\n",sent);  
	  return Err::m_nOK;
  }


int
UDPUnixController::recieve(UChar rtpPacket[], int packetSize){

	int size=0;
	 

	   size = recvfrom(RecieveSocket,(char*) rtpPacket, packetSize,

								0, (SOCKADDR *)&sender_adress, &sender_length);

	   //printf("Size = %d\n",size);
	   //system("pause");
	   if ( size > 0 )

	   {

		   //printf("Server: Total Bytes received: %d\n", size);

		  
		    /*printf("Server: The data is: ");

							   for(int i=0;i<size;i++)
								   printf("%c",rtpPacket[i]);

							   printf("\n");*/

	   }

	   else if ( size <= 0 )

			printf("Server: Connection closed\n");

	   else

			printf("Server: recvfrom() failed\n");



	 	 
		//system("pause");
	  return size;
  }


