#include <cstdio>
#include <stdlib.h>
#include "H264AVCVideoIoLib.h"
#include "RtpBuffer.h"

RtpBuffer::RtpBuffer(){};


RtpBuffer::~RtpBuffer()
{
}


ErrVal
RtpBuffer::create( RtpBuffer*& rpcRtpBuffer ){
	RtpBuffer* pcRtpBuffer;

   pcRtpBuffer = new RtpBuffer;
   rpcRtpBuffer = pcRtpBuffer;
	//printf("RtpBuffer::create()\n");  
	return Err::m_nOK;
}
 

ErrVal
RtpBuffer::destroy(){
	//printf("RtpHeader::destroy()\n");  
	return Err::m_nOK;
}

ErrVal
RtpBuffer::init(){
	
	buffer.start=0;
	buffer.end=0;
	buffer.elements=0;

	lastPoppedSeqNum=0;

	for(int i=0;i<MAX_ELEMENTS;i++)
		resetElement(&buffer.element[i]);
		

	//printf("\nInit del Buffer\n");

	//test();
	return Err::m_nOK;
}

ErrVal
RtpBuffer::uninit(){
	return Err::m_nOK;
}

ErrVal
RtpBuffer::refreshElements(){
	if(buffer.start<buffer.end) //No es dona la volta a la cua
		buffer.elements = buffer.end - buffer.start;
	else
		buffer.elements = MAX_ELEMENTS - (buffer.start-buffer.end); 

	return Err::m_nOK;
}

ErrVal
RtpBuffer::PushElement(Element element){

	int difference=1;
	int pos=0;
	//printf("Valors de la cua RTP abans del push: start:%d, end:%d, elements:%d\n",buffer.start,buffer.end,buffer.elements);

	if(buffer.elements>0)
		if(buffer.end>0)
			difference=element.Sequence_num-buffer.element[buffer.end-1].Sequence_num;
		else
			difference=element.Sequence_num-buffer.element[MAX_ELEMENTS-1].Sequence_num;


	//printf("Difference = %d\n",difference);
	//Cal ordenar un cop es posen
	if(difference==1){ //Si no hi ha hagut cap desordre al enviar paquets

		buffer.element[buffer.end]=element;
		
		if(buffer.end<MAX_ELEMENTS-1)
			buffer.end++;
		else
			buffer.end=0;
		
		if(buffer.elements<MAX_ELEMENTS){
			buffer.elements++;
			return Err::m_nOK;
		}
		else{
			printf("ERROR: Fora del buffer !!\n");
			return Err::m_nEndOfBuffer;			
		}
	}
	else{ // Hi ha hagut algun desordre a l'enviament
		pos=buffer.end+difference-1;

		// El paquet rebut és anterior al ultim de la cua. No hem de modificar el END ni el ELEMENTS
		if(difference<0){// El paquet rebut és anterior al ultim de la cua i la posició on posar el element dona la volta a la cua . Cal modificar la posició pos
			//printf("paquet anterior\n");
			//printf("pos=%d\n",pos);
			if(pos<0){
				pos=MAX_ELEMENTS+pos;
				
			}

			//printf("Out of range:%d",OutOfRange(pos));
			if(OutOfRange(pos)){ //Si la posició és anterior al primer element de la cua hem de reduir el START i augmentar el ELEMENTS
				//Només si no dona la volta
				buffer.start=pos;
				refreshElements();
			}
		}

		// El paquet és posterior al ultim. Augmentem en consonància el END i el ELEMENTS
		if(difference>0){ 
		
		//	printf("paquet posterior\n");
			if(pos>=MAX_ELEMENTS) // La nova posició dóna la volta a la cua
				pos-=MAX_ELEMENTS;
			
			
			//actualitzem END i ELEMENTS
			
			if(buffer.end+=difference<MAX_ELEMENTS)
				buffer.end+=difference-1;
			else
				buffer.end=buffer.end+difference-MAX_ELEMENTS;



			if(buffer.elements+difference<=MAX_ELEMENTS)
				buffer.elements+=difference;
			else{
				return Err::m_nEndOfBuffer;
		//		printf("Fora del buffer\n");
			}


		}
		
		//printf("posem el paquet desordenat a la pos= %d\n",pos);

		buffer.element[pos]=element;

		
		
	}
	
	//printf("Valors de la cua despres del push: start:%d, end:%d, elements:%d\n",buffer.start,buffer.end,buffer.elements);
	
	return Err::m_nOK;
	}

bool
RtpBuffer::isEmpty(){

	if(0 < buffer.elements)
		return false;
	else
		return true;
}

bool
RtpBuffer::isFull(){

	if(buffer.elements < MAX_ELEMENTS)
		return false;
	else
		return true;
}

ErrVal
RtpBuffer::printSeqNumList(){

	//
	printf("List of SEQ NUM: ");

	if(buffer.elements==0)
		printf("[...]");
	else
		if(buffer.start<buffer.end){
			for(int i=buffer.start;i<buffer.end;i++)
				printf("[%d]",buffer.element[i].Sequence_num);
		}
		else{
			for(int i=0;i<buffer.end;i++)
				printf("[%d]",buffer.element[i].Sequence_num);

			printf("[..]···[..]");

			for(int i=buffer.start;i<MAX_ELEMENTS;i++)
				printf("[%d]",buffer.element[i].Sequence_num);
		}


	printf("\n");


	return Err::m_nOK;
}

bool
RtpBuffer::checkFrag(UChar NAL){

	if((NAL&0x1F)!=28)
		return false;
	else
		return true;
}

bool
RtpBuffer::checkStartBit(UChar NALfrag){

	if(!((NALfrag&0x80)>>7))
		return false;
	else
		return true;
}

bool
RtpBuffer::checkEndBit(UChar NALfrag){

	if(!((NALfrag&0x40)>>6))
		return false;
	else
		return true;
}

bool
RtpBuffer::CheckBufferFragments(){
	//bool AllFragments = true;
	
	UShort seqNum;
	bool mBit=false;
	//UChar NAL;
	//UChar NalFrag;

	//printf("seqNum=%d\n",buffer.element[buffer.start].Sequence_num);

	if(lastPoppedSeqNum==0){
		//printf("No s'ha fet cap Pop\n");
		//printf("Forcem el SeqNum\n");
		lastPoppedSeqNum = buffer.element[buffer.start].Sequence_num - 1 ;
		//printf("lastPoppedSeqNum=%d\n",lastPoppedSeqNum);
		//printf("seqNum=%d\n",buffer.element[buffer.start].Sequence_num);

	}

	seqNum = lastPoppedSeqNum;
	//printf("lastPoppedSeqNum=%d\n",lastPoppedSeqNum);

	if(isEmpty())
		return false;

	//seqNum = buffer.element[buffer.start].Sequence_num; //guardem el Seq Number del primer

	//Comprovar que no estigui fragmentat

	//mBit = buffer.element[buffer.start].MarkerBit;

	UInt i = 0;
	while(mBit){
		mBit = buffer.element[buffer.start+i].MarkerBit;
		
		//printf("(%d)lastPoppedSeqNum=%d\n",i,seqNum);

		if((buffer.element[buffer.start+i].Sequence_num - 1)!=seqNum){
			//printf("Last SeqNum = %d, seqNum=%d\n",seqNum,buffer.element[buffer.start+i].Sequence_num);
			//lastPoppedSeqNum = seqNum;
			//exit(0);
			return false;
		}

		seqNum++;
		i++;

	}

	//lastPoppedSeqNum = seqNum;
	return true;

}

Element
RtpBuffer::PopElement(){
//	printf("Valors de la cua RTP abans del pop: start:%d, end:%d, elements:%d\n",buffer.start,buffer.end,buffer.elements);
	//printf("lastPoppedSeqNum = %d\n",lastPoppedSeqNum);

	while(isEmpty()){} //esperem a que la cua tingui elements

	if(lastPoppedSeqNum==0){
		//printf("No s'ha fet cap Pop\n");
		//Forcem el SeqNum
		lastPoppedSeqNum = buffer.element[buffer.start].Sequence_num - 1 ;

	}

	Element element;

	resetElement(&element);

	if(!buffer.element[buffer.start].Sequence_num)
		return element;
	//Comprovar que el ultim paquet tret del buffer és l'immediatament anterior al que volem treure ara
	//printf("Comprovar que el ultim paquet tret del buffer és l'immediatament anterior al que volem treure ara\n");
	//printf("lastPoppedSeqNum = %d, PacketSeqNum= %d,TimeStamp = %d\n",lastPoppedSeqNum,buffer.element[buffer.start].Sequence_num,buffer.element[buffer.start].timestamp);
	//while(buffer.element[buffer.start].Sequence_num!=lastPoppedSeqNum+1){}
	//if(buffer.element[buffer.start].Sequence_num!=lastPoppedSeqNum+1){
		//printSeqNumList();
	//	printf("S'ha perdut o desordenat un paquet\n");
	//}
	//else{
		//Actualitzem el lastPoppedSeqNum
		lastPoppedSeqNum = buffer.element[buffer.start].Sequence_num;
		
		element= buffer.element[buffer.start];

		resetElement(&buffer.element[buffer.start]);

		//printf("ACtualitzem buffer.start\n");
		if(buffer.start<MAX_ELEMENTS-1)
			buffer.start++;
		else
			buffer.start=0;

		buffer.elements--;
	//}

//	printf("Valors de la cua RTP despres del pop: start:%d, end:%d, elements:%d\n",buffer.start,buffer.end,buffer.elements);
	return element;
}

ErrVal
RtpBuffer::resetElement(Element* element){

		element->MarkerBit=false;
		element->Sequence_num=0;
		element->timestamp=0;
		element->data[0]=NULL;
		element->size=0;

		return Err::m_nOK;

}

Bool
RtpBuffer::OutOfRange(int pos){

	//casos
	//Volta o no
	if(buffer.start<buffer.end){ //No s'ha donat la volta
		if(buffer.start<pos&&pos<buffer.end)
			return false;
		else
			return true;
	}
	else{ //Si que s'ha donat
		if(buffer.start>pos||pos>buffer.end)
			return false;
		else
			return true;
	}
	
	
	return false;
}

Element
RtpBuffer::InputTest(int pos){
	Element input;
	input.data[0]='p';
	input.data[1]='r';
	input.data[2]='r';
	input.data[3]='o';
	input.data[4]='v';
	input.data[5]='v';
	input.data[6]=48+pos;

	input.MarkerBit=true;
	input.timestamp=150;
	input.Sequence_num=120+pos;

	return input;
}


ErrVal
RtpBuffer::test(){

	UChar packet[20];
	
	//RTP header
	packet[0] = 0x90;
	packet[1] = 0x80;
	packet[2] = 0x0d;
	packet[3] = 0x6A;
	packet[4] = 0x0;
	packet[5] = 0x0;
	packet[6] = 0x08;
	packet[7] = 0x49;
	packet[8] = 0x0;
	packet[9] = 0x0;
	packet[10] = 0x19;
	packet[11] = 0xa5;

	//dades
	packet[12] = 'p';
	packet[13] = 'r';
	packet[14] = 'o';
	packet[15] = 'v';
	packet[16] = 'a';
	packet[17] = 'R';
	packet[18] = 'T';
	packet[19] = 'P';

	processPacket(packet,20);

	system("pause");
	return 0;

	for(int i=0;i<8;i++)		
		PushElement(InputTest(i));
	

	for(int i=0;i<MAX_ELEMENTS;i++)
		printf("Num de seq del paquet[%d]: %d\n",i,buffer.element[i].Sequence_num);

	system("pause");
	for(int i=0;i<3;i++)
		PopElement();
	

	printf("\n");

	for(int i=0;i<MAX_ELEMENTS;i++)
		printf("Num de seq del paquet[%d]: %d\n",i,buffer.element[i].Sequence_num);

	system("pause");
	
		PushElement(InputTest(9));	
		PushElement(InputTest(8));	
	

	printf("\n");

		for(int i=0;i<MAX_ELEMENTS;i++)
		printf("Num de seq del paquet[%d]: %d\n",i,buffer.element[i].Sequence_num);


	system("pause");


	for(int i=0;i<6;i++)
		PopElement();
	

	printf("\n");

	for(int i=0;i<MAX_ELEMENTS;i++)
		printf("Num de seq del paquet[%d]: %d\n",i,buffer.element[i].Sequence_num);

	system("pause");

	PushElement(InputTest(5));	
	PushElement(InputTest(7));
	PushElement(InputTest(8));
	PushElement(InputTest(6));
	

	printf("\n");

		for(int i=0;i<MAX_ELEMENTS;i++)
		printf("Num de seq del paquet[%d]: %d\n",i,buffer.element[i].Sequence_num);


	system("pause");


	PopElement();
	for(int i=0;i<MAX_ELEMENTS;i++)
		printf("Num de seq del paquet[%d]: %d\n",i,buffer.element[i].Sequence_num);


	system("pause");



	//exit(0);
		return Err::m_nOK;
}

Element
RtpBuffer::processPacket(UChar* rtpPacket , UInt packetSize){

	//Legim els 12 primers bytes i guardem 
	
	Element input;
	resetElement(&input);
	if((rtpPacket[1]&0x80)>>7)
		input.MarkerBit = true;
	else
		input.MarkerBit = false;

	input.Sequence_num = (UShort)(rtpPacket[2]<<8)+rtpPacket[3];
	input.timestamp = (rtpPacket[4]<<24)+(rtpPacket[5]<<16)+(rtpPacket[6]<<8)+rtpPacket[7];
	
	//printf("packetSize = %d",packetSize);

	//printf("rtrtpPacket: \n");

	//for(UInt i=0;i<packetSize;i++)
	//	printf("%c",rtpPacket[i]);

	//printf("\n");
	for(UInt i=0;i+12<packetSize;i++){
		//printf("input=%c\nrtpPacket=%c\n",input.data[i],rtpPacket[i+12]);
		input.data[i] = rtpPacket[i+12];		
	}

	//tamany de les dades sense capçalera RTP
	input.size=packetSize-12;
		/*printf("input.data: \n");

	for(UInt i=0;i<packetSize-12;i++)
		printf("%c",input.data[i]);
	
	printf("\n");*/
	//input.data = rtpPacket[12++];
	

	//printf("Valor de la header:\nMarkerbit=%d\nSeqNumber=%d\nTimestamp=%d\nDades=%s\n",input.MarkerBit,input.Sequence_num,input.timestamp,input.data);

	return input;
}

int
RtpBuffer::printRtpBuffer(){
	
	printf("RTP Buffer\n");

	printf("Valors de la cua: start:%d, end:%d, elements:%d\n",buffer.start,buffer.end,buffer.elements);

	printf("Elements:\n");
	
	for(int i=0;i<buffer.elements;i++)
		printf("Element %d:\nMarkerbit=%d\nSeqNumber=%d\nTimestamp=%d\nDades=%s\n",i,buffer.element[i].MarkerBit,buffer.element[i].Sequence_num,buffer.element[i].timestamp,buffer.element[i].data);
	
	
	return 0;
}