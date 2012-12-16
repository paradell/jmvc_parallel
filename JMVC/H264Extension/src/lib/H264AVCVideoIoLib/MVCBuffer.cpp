
#include <cstdio>
#include <stdlib.h>
#include "H264AVCVideoIoLib.h"
#include "MVCBuffer.h"
//#include "RtpBuffer.

MVCBuffer::MVCBuffer(){};


MVCBuffer::~MVCBuffer()
{
}


ErrVal
MVCBuffer::create( MVCBuffer*& rpcMVCBuffer ){
	MVCBuffer* pcMVCBuffer;

   pcMVCBuffer = new MVCBuffer;
   rpcMVCBuffer = pcMVCBuffer;
	//printf("MVCBuffer::create()\n");  
	return Err::m_nOK;
}
 

ErrVal
MVCBuffer::destroy(){
	//printf("MVCBuffer::destroy()\n");  
	return Err::m_nOK;
}

ErrVal
MVCBuffer::init(){
		
	buffer.start=0;
	buffer.end=0;
	buffer.elements=0;

	for(int i=0;i<MAX_MVC_ELEMENTS;i++)
		buffer.element[i].data[0]=NULL;
		

	//printf("\nInit del Buffer\n");


	//printf("MVCBuffer::init()\n");
	//test();
	return Err::m_nOK;
}

ErrVal
MVCBuffer::uninit(){
	//("MVCBuffer::uninit()\n");  
	return Err::m_nOK;
}

UInt
MVCBuffer::popSize(){
	return buffer.element[buffer.start].size;

}

ErrVal
MVCBuffer::PushElement(DecoderElement element){

	buffer.element[buffer.end]=element;
		
		if(buffer.end<MAX_MVC_ELEMENTS-1)
			buffer.end++;
		else
			buffer.end=0;
		
		if(buffer.elements<MAX_MVC_ELEMENTS){
			buffer.elements++;
			return Err::m_nOK;
		}
		else{
			printf("Fora del buffer\n");
			return Err::m_nEndOfBuffer;			
		}

	return Err::m_nOK;
}

ErrVal
MVCBuffer::printMvcList(){

	//
	printf("List of MVC Buffer: ");


	if(buffer.elements==0)
		printf("[...]");
	else
		if(buffer.start<buffer.end){
			for(int i=buffer.start;i<buffer.end;i++){
				printf("[");
				for(int j=0;j<5;j++)
					printf(" %X ",buffer.element[i].data[j]);
				printf("- (%d bytes)]",buffer.element[i].size);
			}
		}
		else{
			for(int i=0;i<buffer.end;i++)
				printf("[%d]",buffer.element[i].size);

			printf("[..]···[..]");

			for(int i=buffer.start;i<MAX_MVC_ELEMENTS;i++)
				printf("[%d]",buffer.element[i].size);
		}


	printf("\n");


	return Err::m_nOK;
}



UInt
MVCBuffer::popBufferSize(){

	return buffer.elements;
}
Bool
MVCBuffer::isEmpty(){
	if(buffer.elements>0)
		return false;
	else
		return true;

}

void
MVCBuffer::resetElement(DecoderElement *element){

	element->size=0;
	element->data[0]=NULL;

	
}

UChar*
MVCBuffer::popElementData(){

	UInt pos = buffer.start;

	DecoderElement element = PopElement();

	resetElement(&buffer.element[pos]);
	

	return element.data;
}

DecoderElement
MVCBuffer::PopElement(){

	DecoderElement element = buffer.element[buffer.start];
	buffer.element[buffer.start].data[0]=NULL;
	buffer.element[buffer.start].size=0;


	if(buffer.start<MAX_MVC_ELEMENTS-1)
		buffer.start++;
	else
		buffer.start=0;

	buffer.elements--;
	return element;
}

ErrVal
MVCBuffer::test(){

	DecoderElement testElements;
	UChar* testdata=NULL;

	
	testElements.data[0]='t';
	testElements.data[1]='e';
	testElements.data[2]='s';
	testElements.data[3]='t';
	testElements.data[4]='-';
	

	printf("\nElements per entrar a la cua\n");
	for(int i=0;i<10;i++)
		printf("Element %d: %s\n",i,testElements.data);
	

	for(int i=0;i<4;i++)		
		PushElement(testElements);

	printf("\nElements dins la cua\n");
	for(int i=0;i<MAX_MVC_ELEMENTS;i++)
		printf("Element %d: %s\n",i,buffer.element[i].data);

	printf("Dades de la cua MVC: start=%d, end=%d, elements=%d\n",buffer.start, buffer.end, buffer.elements);



	PopElement();
	
	PopElement();
	

	printf("\nElements dins la cua\n");
	for(int i=0;i<MAX_MVC_ELEMENTS;i++)
		printf("Element %d: %s\n",i,buffer.element[i].data);

	printf("Dades de la cua MVC: start=%d, end=%d, elements=%d\n",buffer.start, buffer.end, buffer.elements);


	for(int i=0;i<4;i++)		
		PushElement(testElements);

	printf("\nElements dins la cua\n");
	for(int i=0;i<MAX_MVC_ELEMENTS;i++)
		printf("Element %d: %s\n",i,buffer.element[i].data);

	printf("Dades de la cua MVC: start=%d, end=%d, elements=%d\n",buffer.start, buffer.end, buffer.elements);

	

	PopElement();
	
	PopElement();
	
	PopElement();
	
	PopElement();

	PushElement(testElements);

	PopElement();
	

	printf("\nElements dins la cua\n");
	for(int i=0;i<MAX_MVC_ELEMENTS;i++)
		printf("Element %d: %s\n",i,buffer.element[i].data);

	printf("Dades de la cua MVC: start=%d, end=%d, elements=%d\n",buffer.start, buffer.end, buffer.elements);


	system("pause");
	//exit(0);
		return Err::m_nOK;
}