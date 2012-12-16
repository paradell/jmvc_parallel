#include <cstdio>
#include <stdlib.h>
#include "H264AVCVideoIoLib.h"
#include "UDPController.h"

#define IP_ADRESS inet_addr( "192.168.1.128" )
#define PORT 5150


UDPController::UDPController(){};


UDPController::~UDPController()
{
}

ErrVal
UDPController::create( UDPController*& rpcUDPController ){
	UDPController* pcUDPController;

   pcUDPController = new UDPController;
   rpcUDPController = pcUDPController;
	//printf("UDPController::create()\n");  
	return Err::m_nOK;
}
 

ErrVal
UDPController::destroy(){
	 // When your application is finished call WSACleanup.

   //printf("Client: Cleaning up...\n");

   if(WSACleanup() != 0)
        printf("Client: WSACleanup() failed! Error code: %ld\n", WSAGetLastError());

   	//printf("UDPController::destroy()\n");  
	return Err::m_nOK;
}

ErrVal
UDPController::initReciever(int confPort){
	
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

 

     if (RecieveSocket == INVALID_SOCKET)

     {

          printf("Server: Error at socket(): %ld\n", WSAGetLastError());

          // Clean up

          WSACleanup();

          // Exit with error

          return -1;

     }

    
 

     // Set up a SOCKADDR_IN structure that will tell bind that we

     // want to receive datagrams from all interfaces using port 5150.

 

     // The IPv4 family

     reciever_adress.sin_family = AF_INET;

     // Port no. 5150

     reciever_adress.sin_port = htons(confPort);

     // From all interface (0.0.0.0)

     reciever_adress.sin_addr.s_addr = htonl(INADDR_ANY);

 

   // Associate the address information with the socket using bind.

   // At this point you can receive datagrams on your bound socket.

   if (bind(RecieveSocket, (SOCKADDR *)&reciever_adress, sizeof(reciever_adress)) == SOCKET_ERROR)

   {

        printf("Server: bind() failed! Error: %ld.\n", WSAGetLastError());

        // Close the socket

        closesocket(RecieveSocket);

        // Do the clean up

        WSACleanup();

        // and exit with error

        return -1;

     }

     else

          //printf("Server: bind() is OK!\n");

 

   // Some info on the receiver side...

   getsockname(RecieveSocket, (SOCKADDR *)&reciever_adress, (int *)sizeof(reciever_adress));

 

   //printf("Server: Receiving IP(s) used: %s\n", inet_ntoa(reciever_adress.sin_addr));

   //printf("Server: Receiving port used: %d\n", htons(reciever_adress.sin_port));

   //printf("Server: I\'m ready to receive a datagram...\n");


   int SelectTiming = recvfromTimeOutUDP(RecieveSocket, 10, 0); //Set the timeout for 10 secs and 00 usecs
    
	 switch (SelectTiming)

        {

             case 0:

                 // Timed out, do whatever you want to handle this situation

                 printf("Server: Timeout waiting the client...\n");
				 return Err::m_nERR;

                 break;

             case -1:

                 // Error occurred, maybe we should display an error message?

                // Need more tweaking here and the recvfromTimeOutUDP()...

                 printf("Server: Some error encountered with code number: %ld\n", WSAGetLastError());
				 return Err::m_nERR;

                 break;

             default:
				 recieve(mess,20);
				 //printf("Message recieved: %s",mess);
				 //printf("Peer connected\n");
				/// sendto(RecieveSocket, "Yes, Server is Ready..." , 23, 0, (SOCKADDR*) &sender_adress, sender_length);
	 }

    
   
   

  return Err::m_nOK;
}

ErrVal
UDPController::initSender(char* confAdress, int confPort){


	if( WSAStartup(MAKEWORD(2,2), &wsaData) != 0)

     {

          printf("Client: WSAStartup failed with error %ld\n", WSAGetLastError());

          // Clean up

          WSACleanup();

          // Exit with error

          return -1;

     }

     else

          //printf("Client: The Winsock DLL status is %s.\n", wsaData.szSystemStatus);


	// Create a new socket to receive datagrams on.

     SendingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

     if (SendingSocket == INVALID_SOCKET)

     {

          printf("Client: Error at socket(): %ld\n", WSAGetLastError());

          // Clean up

          WSACleanup();

          // Exit with error

          return -1;

     }

    


	//Define Internet adress
    reciever_adress.sin_family = AF_INET;
    reciever_adress.sin_addr.s_addr = inet_addr( confAdress );
	port = confPort;
    reciever_adress.sin_port = htons(port);

	//
	//server_length = sizeof(struct sockaddr_in);

	

	//connect /bind

	//printf("Prepare the Server\n");
	system("pause");
	checkServer();
	

	//printf("Encoding preparation done, checking the reciever...\n");
	//system("pause");
	

	//printf("UDPController::init()\n");  
	return Err::m_nOK;
 }

ErrVal
UDPController::setSocket(){
	
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
UDPController::recvfromTimeOutUDP(SOCKET socket, long sec, long usec){

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
UDPController::uninit(){
	WSACleanup();
	//printf("UDPController::uninit()\n");  
	return Err::m_nOK;
 }

ErrVal
UDPController::checkServer(){

	
	int size=0;

	sendto(SendingSocket, "Server is Alive..?" , 20, 0, (SOCKADDR*) &reciever_adress, sizeof(reciever_adress));

		 
	return Err::m_nOK;
}

ErrVal
UDPController::send(UChar* rtpPacket,int size){
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
UDPController::recieve(UChar rtpPacket[], int packetSize){

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

			printf("Server: Connection closed with error code: %ld\n",

						WSAGetLastError());

	   else

			printf("Server: recvfrom() failed with error code: %d\n",

					WSAGetLastError());



	   // Some info on the sender side

	   getpeername(RecieveSocket, (SOCKADDR *)&sender_adress, &sender_length);

	//   printf("Server: Sending IP used: %s\n", inet_ntoa(sender_adress.sin_addr));

	//   printf("Server: Sending port used: %d\n\n", htons(sender_adress.sin_port));    
			
	
	
	//system("pause");

	//Just to debug
	//sendto(RecieveSocket, "packet recieved OK" , 18, 0, (SOCKADDR*) &sender_adress, sender_length);

	 
		//system("pause");
	  return size;
  }


