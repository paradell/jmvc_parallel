#include <cstdio>
#include "TestMachine.h"

#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <iostream>
#include <fstream>

#define NUM_THREADS 2

 UChar testPacket[20];
 UChar* updPacket;
 UChar nal_unit[4];

 UChar header1;
 UChar header2;
 UShort Sequence_num; //hauria de ser aleatori
 UInt timestamp; //l'agafem de 
 UInt ssrc; 

 int thread=0;

 boost::thread_group g;

 boost::thread workerThread[NUM_THREADS];

 boost::condition recurs_lliure[NUM_THREADS];

 using namespace std;
  //Element element;
	//DecoderElement decElement;

 void setMVCPacket(char n){
	 
	 nal_unit[0]=0x4E;
	 nal_unit[1]=0x40;
	 nal_unit[2]=0x00;
	 nal_unit[3]=0x03;

	 //dades
	testPacket[0] = 'p';
	testPacket[1] = 'r';
	testPacket[2] = 'o';
	testPacket[3] = 'v';
	testPacket[4] = 'a';
	testPacket[5] = ' ';
	testPacket[6] = 48+(n/10);
	testPacket[7] = 48+(n%10);
	testPacket[8] = ' ';

	for(int i=9;i<20;i++)
		testPacket[i]='.';

	//printf("Paquet %d = %c%c\n",n,testPacket[6],testPacket[7]);
	//system("pause");

 }

 void createMVCPackets(int n){
	 for(int i=0;i<n;i++)
		 setMVCPacket((char)n);
 }

 void printMVCPacket(){
	 printf("NAL Unit:\n");
	
	 for(int i=0;i<4;i++)
		printf("%x",nal_unit[i]);
	 

	 printf("\nData:\n");
	 for(int i=0;i<20;i++)
		printf("%c",testPacket[i]);

	 printf("\n");
	
	 
 }

UInt setRTP(){


	testPacket[0] = header1;
    testPacket[1] = (char)(header2);
	testPacket[2] =((char) ((Sequence_num & 0xFF00) >> 8));
    testPacket[3] =((char) (Sequence_num & 0x00FF));

	testPacket[4] =((char) ((timestamp & 0xFF000000) >> 24));
    testPacket[5] =((char) ((timestamp & 0x00FF0000) >> 16));
    testPacket[6] =((char) ((timestamp & 0x0000FF00) >> 8));
    testPacket[7] =((char) ((timestamp & 0x000000FF)));

    testPacket[8] =((char) ((ssrc & 0xFF000000) >> 24));
    testPacket[9] =((char) ((ssrc & 0x00FF0000) >> 16));
    testPacket[10] =((char) ((ssrc & 0x0000FF00) >> 8));
    testPacket[11] =((char) ((ssrc & 0x000000FF)));

	return 0; 

}

int preparePackets(int n){
		
	//RTP header
	header1=0x90;
	header2=0x60;
	Sequence_num =200+n; //hauria de ser aleatori
	timestamp=100+n; //l'agafem de 
	ssrc=500;

	setRTP();
	
	//NAL
	testPacket[12] = 0x4E;
	testPacket[13] = 0x40;
	testPacket[14] = 0x00;
	testPacket[15] = 0x03;
	
	//dades
	testPacket[16] = 'p';
	testPacket[17] = 'r';
	testPacket[18] = 'o';
	testPacket[19] = 'v';
	testPacket[20] = 'a';
	testPacket[21] = '_';
	testPacket[22] = 48+n;

	return 0;
}

boost::mutex io_mutex;
boost::mutex monitor;
typedef boost::mutex::scoped_lock lock;
	

void funcio(int n,int i){
	
	boost::posix_time::seconds funcTime(1);

	int prev;
	if(n!=0)
		prev=n-1;
	else
		prev=NUM_THREADS-1;

	lock lk(monitor);
	
	//if(n==NUM_THREADS-1 && i==0)//Condició inicial
	//{
	
	//	recurs_lliure[n].notify_one();
	//}

	recurs_lliure[n].notify_one(); //Notifiquem que el nostre recurs per desbloquejar antics threads
	
	
	if(!(n==0&&i==0)){// el primer thread, per la primera iteració no espera
		printf("El thread %d espera el permís del thread %d\n",n,prev);
		recurs_lliure[prev].wait(lk); //Esperem a que el thread anterior alliberi el recurs
	}
	
	
	;
	

	printf("\nEl thread %d esta 'utilitzant' el recurs per %d vegada\n\n",n,i);

	boost::this_thread::sleep(funcTime);

	//	 printf("Esperem 3 segons a iniciar tot\n");

	//	// Pretend to do something useful...
	
	//system("pause");
	/*if(n==0&&i==0)
		system("pause");*/

	recurs_lliure[n].notify_one(); //Notifiquem que el nostre recurs 

	

}

	void workerFunc(int n)
{
	
    boost::posix_time::seconds workTime(6);

    printf("Thread %d arrencat\n",n);

    // Pretend to do something useful...
   // boost::this_thread::sleep(workTime);

	//Fem el lock del recurs
	//boost::mutex::scoped_lock lock(io_mutex);
	//lock lk(monitor);
	
	
	//if(!n)//Si no és el primer, fem que esperi
	//	recurs_lliure.wait(lk);

	for(int i=0;i<5;i++){

		boost::this_thread::sleep(workTime);


		{ //Bloquejem el recurs abans de fer-lo servir
			boost::mutex::scoped_lock io_lock(io_mutex);
					//printf("El thread %d ha bloquejat el recurs\n",n);					
		}
		funcio(n,i);
	}

    printf("Thread %d finalitzat\n",n);
}

	


int main(int argc, char* argv[])
{
    std::cout << "main: startup" << std::endl;

	for(int i=0;i<NUM_THREADS;i++){
		workerThread[i] = boost::thread (workerFunc,i);
		g.add_thread(&workerThread[i]);
	}
	
    std::cout << "main: waiting for thread" << std::endl;

	//system("pause");
	//recurs_lliure[NUM_THREADS-1].notify_one();
	g.join_all();

    std::cout << "main: done" << std::endl;
	
	for(int i=0;i<NUM_THREADS;i++)
		g.remove_thread(&workerThread[i]);
	
	system("pause");
    return 0;
}
	




//int
//main( int argc, char** argv)   
//{
//  printf("Màquina de prova\n\n");
//
//  ofstream myfile;
//
//  int numpack=0;
//  updPacket=NULL;
// 
//  UChar* dades;
//
//  bool Encoder = false;
//  //Element element;
//
// 
//  
//  RtpPacker*               pcRtpPacker = NULL;
//  
//
//  //MVCBuffer*               m_apcMVCBuffer = NULL;
//  //RtpBuffer*               m_apcRtpBuffer = NULL;
//  //UDPController*			m_apcUDPServer = NULL;
//
// 
//  //UDPController::create(m_apcUDPServer);
//  //m_apcUDPServer->initReciever(5500);
//	//system("pause");
//
//	//m_apcUDPServer->setSocket();
//	
//	//m_apcUDPServer->recieve(updPacket,1500);
//
//  RtpPacker::create(pcRtpPacker);
//
//
//  setMVCPacket(14);
//
//  if(Encoder){
//
//	printf("Arranquem el Encoder\n");
//	
//	//UDPController::create(m_apcUDPServer);
//	//m_apcUDPServer->initSender("127.0.0.1",5150);
//
//	
//	pcRtpPacker->init("127.0.0.1",5150,true);
//
//	system("pause");
//	//exit(0);
//
//	//m_apcUDPServer->send("Server Alive?",13);
//
//	for(int i=0;i<10;i++){
//		setMVCPacket(i);
//		//preparePackets(packets[i]);
//		pcRtpPacker->pack(nal_unit,testPacket,20);
//		//m_apcUDPServer->send(testPacket,24);
//		system("pause");
//	}
//
//	printf("Segona Onada");
//	system("pause");
//	for(int i=0;i<10;i++){
//		setMVCPacket(i+10);
//		//preparePackets(packets[i]);
//		pcRtpPacker->pack(nal_unit,testPacket,20);
//		//m_apcUDPServer->send(testPacket,24);
//		system("pause");
//	}
//
//
//		
//
//
//
//	
//	//Prepare 10 packets and send
//  }
//  else
//  {
//	 printf("Arranquem el Decoder\n");
//	//myfile.open ("rebuda.txt");
//	 system("pause");
//	pcRtpPacker->init(NULL,5150,false);
//
//  
//	//exit(0);
//	  //pcRtpPacker->test();
//
//  
//  
//
//  
//
//	printf("Preparats per rebre paquets\n");
//
//	  while(numpack<4){
//		printf("\n\nPaquet: %d\n",numpack);
//		pcRtpPacker->recieveRTP();
//
//		pcRtpPacker->unpack();
//
//		dades = pcRtpPacker->popMVCdata();
//		//myfile << dades;
//
//		for(int i=0;i<10;i++)
//			printf("%X ",dades[i]);
//
//		printf("\n");
//		
//		numpack++;
//	  }
//
//	   // myfile.close();
///*
//	  numpack=0;
//
//	  while(numpack<10){
//		  printf("Paquet: %d\n",numpack);
//		
//		pcRtpPacker->unpack();
//
//		dades=pcRtpPacker->popMVCdata();
//		
//		printf("Dada rebuda: \n");
//
//		for(int i=0;i<4;i++)
//			printf("%x ",dades[i]);
//		
//		for(int i=4;i<24;i++)
//			printf("%c",dades[i]);
//
//		printf("\n");
//		
//		numpack++;
//
//		//system("pause");
//	  }
//
//	printf("Segona onada\n");
//	  system("pause");
//
//	  numpack=0;
//	 while(numpack<10){
//		  printf("Paquet: %d\n",numpack);
//		pcRtpPacker->recieveRTP();
//
//		/*pcRtpPacker->unpack();
//
//		dades=pcRtpPacker->popMVCdata();
//		
//		printf("Dada rebuda: \n");
//
//		for(int i=0;i<4;i++)
//			printf("%x ",dades[i]);
//		
//		for(int i=4;i<24;i++)
//			printf("%c",dades[i]);
//
//		printf("\n");
//		
//		numpack++;
//	  }
//
//	  numpack=0;
//
//	  while(numpack<10){
//		  printf("Paquet: %d\n",numpack);
//		
//		pcRtpPacker->unpack();
//
//		dades=pcRtpPacker->popMVCdata();
//		
//		printf("Dada rebuda: \n");
//
//		for(int i=0;i<4;i++)
//			printf("%x ",dades[i]);
//		
//		for(int i=4;i<24;i++)
//			printf("%c",dades[i]);
//
//		printf("\n");
//		
//		numpack++;
//
//		//system("pause");
//	  }
//
//	system("pause");
//
//  }
//  */
//  /*
//system("pause");
// 
//  MVCBuffer::create(m_apcMVCBuffer);
//  m_apcMVCBuffer->init();
//  system("pause");
//
//  RtpBuffer::create(m_apcRtpBuffer);
//  m_apcRtpBuffer->init();
//  system("pause");
//
// // m_apcRtpBuffer->test();
//
//  for(int i=0;i<3;i++){
//	  preparePackets(i);
//	  printf("Paquet: ");
//	  for(int j=0;i<20;j++)
//		printf("%c",testPacket[j]);
//	  printf("\n");
//	  element=m_apcRtpBuffer->processPacket(testPacket,20);
//	  m_apcRtpBuffer->PushElement(element);
//  }
//
//  m_apcRtpBuffer->printRtpBuffer();
//
//  for(int i=0;i<3;i++){
//	//Pop from RTPBuffer
//	element=m_apcRtpBuffer->PopElement();
//	//Check Markerbit
//	if(!element.MarkerBit){
//	//Join packets (if necessary)
//		//Eliminar la NAL de fragmentació
//		//Editar el type de la primera NAL
//		//La resta de paquets concatenar sense cap NAL
//	}
//	//Push into MVCBuffer
//	decElement.data=element.data;
//	m_apcMVCBuffer->PushElement(decElement);
//  }
//  m_apcRtpBuffer->printRtpBuffer();
//  //m_apcMVCBuffer->test();
//  //m_apcRtpBuffer->test();
//
//  //pcRtpPacker->m_apcRtpBuffer->PushElement(element);
//  */
//}
//	printf("\n\nProva finalitzada\n");
//
//  return 0;
//}


// Copyright (C) 2001-2003
// William E. Kempf
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//#include <iostream>
//#include <vector>
//#include <boost/utility.hpp>
//#include <boost/thread/condition.hpp>
//#include <boost/thread/thread.hpp>
//
//class bounded_buffer : private boost::noncopyable
//{
//public:
//    typedef boost::mutex::scoped_lock lock;
//
//    bounded_buffer(int n) : begin(0), end(0), buffered(0), circular_buf(n) { }
//
//    void send (int m) {
//        lock lk(monitor);
//        while (buffered == circular_buf.size())
//            buffer_not_full.wait(lk);
//        circular_buf[end] = m;
//        end = (end+1) % circular_buf.size();
//        ++buffered;
//        buffer_not_empty.notify_one();
//    }
//	
//    int receive() {
//        lock lk(monitor);
//        while (buffered == 0)
//            buffer_not_empty.wait(lk);
//        int i = circular_buf[begin];
//        begin = (begin+1) % circular_buf.size();
//        --buffered;
//        buffer_not_full.notify_one();
//        return i;
//    }
//
//private:
//    int begin, end, buffered;
//    std::vector<int> circular_buf;
//    boost::condition buffer_not_full, buffer_not_empty;
//    boost::mutex monitor;
//};
//
//bounded_buffer buf(2);
//
//boost::mutex io_mutex;
//
//void sender() {
//    int n = 0;
//    while (n < 1000000) {
//        buf.send(n);
//        if(!(n%10000))
//        {
//            boost::mutex::scoped_lock io_lock(io_mutex);
//            printf("sent: %d\n", n);
//        }
//        ++n;
//    }
//    buf.send(-1);
//}
//
//void receiver() {
//    int n;
//    do {
//        n = buf.receive();
//        if(!(n%10000))
//        {
//            boost::mutex::scoped_lock io_lock(io_mutex);
//            printf("received: %d\n", n);
//        }
//    } while (n != -1); // -1 indicates end of buffer
//    buf.send(-1);
//}
//
//int main(int, char*[])
//{
//    boost::thread thrd1(&sender);
//    boost::thread thrd2(&receiver);
//    boost::thread thrd3(&receiver);
//    boost::thread thrd4(&receiver);
//    thrd1.join();
//    thrd2.join();
//    thrd3.join();
//    thrd4.join();
//    return 0;
//}