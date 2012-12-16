#include <cstdio>
#include <stdlib.h>
#include "H264AVCVideoIoLib.h"
#include "RtpPacker.h"
//#include "net_formats.h"

RtpPacker::RtpPacker(){};


RtpPacker::~RtpPacker()
{
}

ErrVal
RtpPacker::create( RtpPacker*& rpcRtpPacker ){
	RtpPacker* pcRtpPacker;

   pcRtpPacker = new RtpPacker;
   rpcRtpPacker = pcRtpPacker;

   

	
	return Err::m_nOK;
}
 

ErrVal
RtpPacker::destroy(){
	//printf("RtpPacker::destroy()\n");

	m_apcUDPController->destroy();
	return Err::m_nOK;
}


ErrVal
RtpPacker::init(char* confAdress, int confPort, bool Encoder){

	dataRTP[0]=NULL;

	UDPController::create(m_apcUDPController);
	

	if(!Encoder){ //Si és el Decoder, cal setejar les cues de recepció i fer el bind() del UDP

		m_apcUDPController->initReciever(confPort);

		//for(UInt i=0;i<2/*numViews*/;i++){
			

			MVCBuffer::create(m_apcMVCBuffer);
			m_apcMVCBuffer->init();


			RtpBuffer::create(m_apcRtpBuffer);
			m_apcRtpBuffer->init();
		//system("pause");
		//}
		
	}
	else
		m_apcUDPController->initSender(confAdress, confPort);
		
	

  

  fragmented=false;
  pcRtpHeader.init();	
  rtpData[0] =NULL;
  nal_unit_fragmented[0]=0;
  rtpPacketSize = 0;
  fragment_index=0;

  pcStandardNALUnit.forbidden_bit=false;
  pcStandardNALUnit.nri_bits=0;
  pcStandardNALUnit.type=0;



  pcFragmentNALUnit.start_bit=true;
  pcFragmentNALUnit.reserved_bit=true;
  pcFragmentNALUnit.end_bit=false;
  pcFragmentNALUnit.type=FRAGMENTED_TYPE;


  xgetNalFragUnit();
  
  //capçalera completa RTP per H264
  xsetRTPHeader(false,false);

  
	return Err::m_nOK;
 }

ErrVal
RtpPacker::uninit(){
	
	//("RtpPacker::uninit()\n");  
	return Err::m_nOK;
 }
ErrVal
RtpPacker::packInit(UChar * data, int size){
	fragment_index=0;
	fragmented = false;

	/*printf("tamany de les dades: %d\n",size);
	printf("Dades que enviem al packInit: ");

	  for(int i=0;i<size;i++)
		  printf("%X ",data[i]);

	  printf("\n");*/

	xRTPnotMVC(data,size);

	


	m_apcUDPController->send(rtpData,rtpPacketSize);
	increaseSeqNum();
	 return Err::m_nOK;
}

ErrVal
RtpPacker::endTransmission(){
	
	xRTPnotMVC(NULL,0);
	m_apcUDPController->send(rtpData,rtpPacketSize);

	return Err::m_nOK;
}


ErrVal
RtpPacker::pack(UChar * nal_unit,UChar * data, int size){
	fragmented=false;
	fragment_index=0;

	xSetStandardNAL(nal_unit[0]); //Llegim la NAL per saber quin tipus es,

	if(pcStandardNALUnit.type!=14){ //Si es de tipus 14, la NAL ve en un paquet separat de les dades, i s'ha de processar diferent
		first_view=false;

		//Com ve en un paquet la NAL i les dades, eliminem els 4 bytes de la NAL i rebaixem el tamany del size

		for(int i=0;i+4<size;i++)
			data[i]=data[i+4];

		size-=4;
	}
	else
		first_view=true;;

	
	

	/*if(first_view)
		printf("Paquet que enviarem de tamany %d bits, sense comptar NAL: ",size*8);
	else
		printf("Paquet que enviarem de tamany %d bits, amb la NAL ja treta: ",size*8);


	for(int i=0;i<min(size,30);i++)
			  printf("%X ",data[i]);

		  printf("\n");*/


	pcRtpHeader.setRTPHeader(false,false);

		//Cal fragmentar el paquet?
		if(size>DATA_SIZE){
			fragmented=true;
			xFragment(nal_unit,data,size);
			return Err::m_nOK;
		}

		else{
			xRTPSingle(nal_unit,data,size);
			m_apcUDPController->send(rtpData,rtpPacketSize);
			increaseSeqNum();
		}

	
	  return Err::m_nOK;
  }

ErrVal
RtpPacker::increaseSeqNum(){
	pcRtpHeader.increaseSequenceNumber();
	return Err::m_nOK; 

}

ErrVal
RtpPacker::increaseTimeStamp(){
	pcRtpHeader.increaseTimestamp(period);
	return Err::m_nOK; 

}

ErrVal
RtpPacker::setPeriod(int freq){
	period=(100/freq);
	//printf("Period = %d, freq = %d\n",period,freq);
	return Err::m_nOK; 

}


UChar
RtpPacker::xgetNalUnit(){
	
	//printf("NAL Genèrica\n");
	//printf("Valor dels diferents camps:\nForbidden bit:%d\nNRI bit:%d\nType:%d\n",pcStandardNALUnit.forbidden_bit,pcStandardNALUnit.nri_bits,pcStandardNALUnit.type);

	pcNALUnit = ((pcStandardNALUnit.forbidden_bit&true)<<7)|((pcStandardNALUnit.nri_bits&0x03)<<5)|(pcStandardNALUnit.type&0x1F);
	//printf("Valor de la NAL:%X\n",pcNALUnit);

 return pcNALUnit;
}

bool
RtpPacker::isMvcBufferEmpty(){
	return m_apcMVCBuffer->isEmpty();
}

bool
RtpPacker::isRtpBufferEmpty(){

	return m_apcRtpBuffer->isEmpty();
}

bool
RtpPacker::isRtpBufferFull(){

	return m_apcRtpBuffer->isFull();
}

ErrVal
RtpPacker::unpack(){

	//printf("\n----------UNPACK------\n");

	first_view=false;

	UInt nal_size=0;

	Element element;
	element.Sequence_num=0;
	element.timestamp=0;
	DecoderElement decElement;

	char data[15000];

	decElement.data[0]=NULL;
	decElement.size = 0;

	Bool packetsRemaining = true;
	//fragment_NAL_unit nal_frag;//pcFragmentNALUnit
	//NAL_unit nal; pcStandardNALUnit

	//m_apcMVCBuffer->printMvcList();

	//m_apcRtpBuffer->printSeqNumList();

	if(!m_apcRtpBuffer->CheckBufferFragments()){
		printf("unpack -> CheckBufferFragments()\n");
		 return Err::m_nOK;
	}

	//Fer un check de RTPBuffer que hi siguin tots els fragments.


	while(packetsRemaining){
	//Pop from RTPBuffer
		element=m_apcRtpBuffer->PopElement();

		if(!element.Sequence_num){ //SO el Seq Num és 0, vol dir que el paquet que havia d'arribar no estava ordenat i que cal esperar
			 printf("UNPACK() - Hi ha un paquet perdut\n"); 
			 return Err::m_nOK;
		}
		
		
		xSetStandardNAL(element.data[0]);
		xSetFragmentNAL(element.data[1]);
		/*printf("Valor de la NAL: forbidden=%d, nri=%d, type=%d\n",pcStandardNALUnit.forbidden_bit,pcStandardNALUnit.nri_bits,pcStandardNALUnit.type);
		printf("Valor de la NAL de frag: start=%d, end=%d, reserverd=%d, type=%d\n",pcFragmentNALUnit.start_bit,pcFragmentNALUnit.end_bit,pcFragmentNALUnit.reserved_bit,pcFragmentNALUnit.type);*/


		if(element.MarkerBit){
		//Join packets (if necessary)
			//printf("NAL Fragmentada\n");
			////Llegit el segoon byte i mirar els valors end i start
			//
			//printf("Valor de la NAL: forbidden=%d, nri=%d, type=%d\n",pcStandardNALUnit.forbidden_bit,pcStandardNALUnit.nri_bits,pcStandardNALUnit.type);

			//printf("Valor de la NAL de frag: start=%d, end=%d, reserverd=%d, type=%d\n",pcFragmentNALUnit.start_bit,pcFragmentNALUnit.end_bit,pcFragmentNALUnit.reserved_bit,pcFragmentNALUnit.type);
			//
			
			

			//Eliminar la NAL de fragmentació
			
			if(pcFragmentNALUnit.start_bit){
				//printf("Primer paquet del fragment\n");
				//Editar el type de la primera NAL 
				pcStandardNALUnit.type = pcFragmentNALUnit.type;
		
				/*printf("Valor de la NAL: forbidden=%d, nri=%d, type=%d\n",pcStandardNALUnit.forbidden_bit,pcStandardNALUnit.nri_bits,pcStandardNALUnit.type);

				printf("Valor de la NAL de frag: start=%d, end=%d, reserverd=%d, type=%d\n",pcFragmentNALUnit.start_bit,pcFragmentNALUnit.end_bit,pcFragmentNALUnit.reserved_bit,pcFragmentNALUnit.type);
			*/

				element.data[1] = xgetNalUnit(); //Posem la NAL correcta al segon byte, ja que mourem tot un caràcter cap a la esquerra

				if(pcStandardNALUnit.type==14){ //implica que és de la view 0, que la NAL ja està enviada i s'ha de treure també del primer paquet, si no eliminem només el primer byte de la NAL
					nal_size=5;
					//printf("Primer paquet de la view 0\n");
				}
				else{
					nal_size=1;
					//printf("Primer paquet de la view 1\n");
				}

			
				
				
				//eliminar la NAL de fragmentació
				//xRemoveFragNalUnit(element.data);
				for(UInt i=0;i+nal_size<element.size;i++)
					element.data[i]=element.data[i+nal_size];

				element.size-=nal_size;

				//strncpy(data,(char*)element.data,element.size);

				for(UInt i=0;i<element.size;i++)
					data[i]=element.data[i];

				

			}
			else{
				//Eliminar la part de NAL+NALfrag+NALext i copiar u element MVC
				nal_size=5;

				for(UInt i=0;i+nal_size<element.size;i++)
					element.data[i]=element.data[i+nal_size];

				element.size-=nal_size;

				for(UInt i=0;i<element.size;i++)
					data[i+decElement.size]=element.data[i];
			}
				
			if(pcFragmentNALUnit.end_bit)
				packetsRemaining=false;


		}
		else{
			packetsRemaining=false; //Si no està fragmentat, no s'agafen més


			switch(pcStandardNALUnit.type){
				case 28: //PAquet fragmentat -> Mirem que realment sigui l'ultim i elimem les NAL+Nalfrag+NALext (5Bytes)
					/*printf("No queden REMAINING PACKETS, i ens surt END bit\n");

					printf("Valor de la NAL: forbidden=%d, nri=%d, type=%d\n",pcStandardNALUnit.forbidden_bit,pcStandardNALUnit.nri_bits,pcStandardNALUnit.type);
					printf("Valor de la NAL de frag: start=%d, end=%d, reserverd=%d, type=%d\n",pcFragmentNALUnit.start_bit,pcFragmentNALUnit.end_bit,pcFragmentNALUnit.reserved_bit,pcFragmentNALUnit.type);
			*/
					if(pcFragmentNALUnit.end_bit) //Comprovem que realment é sl'ultim paquet
						nal_size=5;
					else
						nal_size=0;

					break;

				case 14: //Paquet de la view0 -> Paquet de la view0, la NAL ja ha estat processada, cal eliminar la NAL+NAL ext (4Bytes)
					if(element.size>4) //No és un paquet NAL sol if(element.size>4)
						nal_size=4;
					else
						nal_size=0;
					break;

				default: //Paquet no fragmentat de la resta de vistes -> NAL no processada, per tant, copiem TOT el paquet
					nal_size=0;
					break;
			};
			//if(pcStandardNALUnit.type==28){ //Es paquet fragmentat
			//	xSetFragmentNAL(element.data[1]); //Llegim la NAL de fragmentació
			//	if(pcFragmentNALUnit.end_bit) //Comprovem que realment é sl'ultim paquet
			//		nal_size=5;
			//}


			//if(pcStandardNALUnit.type==14&&element.size>4) //implica que és de la view 0, que la NAL ja està enviada i s'ha de treure també del primer paquet, si no eliminem només el primer byte de la NAL
			//	nal_size=4;
			//else
			//	nal_size=0;
			
			for(UInt i=0;i+nal_size<element.size;i++)
					data[i+decElement.size]=element.data[i+nal_size];

			element.size-=nal_size;
		}
		//Push into MVCBuffer
		//if(decElement.size>0)
		//for(UInt i=0;i<element.size;i++){ //Attach data to decElement
		//	decElement.data[decElement.size+i]=element.data[i];
		//	printf("i= %d\n",i);
		//}
		//else
			//decElement.data=element.data;
		//decElement.data=element.data;
		
		//printf("cadena concatenada\n");
		decElement.size += element.size; //Update decElement size
		
		
	}
	//m_apcMVCBuffer->printMvcList();

	//strcpy((char*)decElement.data,data);


	//Aquest punter fa que es copiin totes les dades !!!!!!
	for(UInt n=0;n<decElement.size;n++ )
		decElement.data[n]=data[n];

	
	//Un cop tot el paquet està preparat, es fa el Push
	m_apcMVCBuffer->PushElement(decElement); 

	//m_apcMVCBuffer->printMvcList();

	//printf("\n----------FI_UNPACK------\n");

	/*printf("Dades rebudes del paquet MVC: ");

	for(UInt i=0;i<min(decElement.size,30);i++)
			  printf("%X ",decElement.data[i]);

		  printf("\n");*/

	//printf("RtpPacker::unpack()\n");  
	  return Err::m_nOK;
  }

UChar*
RtpPacker::popMVCdata(){

	while(m_apcMVCBuffer->isEmpty()){
		//Wait until there is any element waiting
		//m_apcRtpBuffer->printRtpBuffer();
		printf("Estem dins el bucle m_apcMVCBuffer->isEmpty()");
	}
	
	//m_apcMVCBuffer->printMvcList();

	return m_apcMVCBuffer->popElementData();

}

DecoderElement
RtpPacker::popMVCelement(){
	//Check if there is elements and then pop the oldest element out of the queue.
	
	while(m_apcMVCBuffer->isEmpty()){
		//Wait until there is any element waiting
	}
	

	return m_apcMVCBuffer->PopElement();
}

UInt
RtpPacker::packetSize(){

	//printf("Tamany del buffer MVC: %d\n",m_apcMVCBuffer->popBufferSize());
	return m_apcMVCBuffer->popSize();
}


ErrVal
RtpPacker::xsetRTPHeader(bool padding, bool markerbit){
	
	pcRtpHeader.setRTPHeader(padding,markerbit);


	  return Err::m_nOK;
  }

UChar *
RtpPacker::xgetRTPHeader(){

	return pcRtpHeader.getRTPHeader();
	//return "TEst";
  }




UChar
RtpPacker::xgetNalFragUnit(){

	//printf("NAL de fragmentació\n");
	//printf("Valor dels diferents camps:\nStart bit:%d\nEnd bit:%d\nReserved bit:%d\nType:%d\n",pcFragmentNALUnit.start_bit,pcFragmentNALUnit.end_bit,pcFragmentNALUnit.reserved_bit,pcFragmentNALUnit.type);

	pcNALFragUnit = ((pcFragmentNALUnit.start_bit&true)<<7)|((pcFragmentNALUnit.end_bit&true)<<6)|((pcFragmentNALUnit.reserved_bit&true)<<5)|(pcFragmentNALUnit.type&0x1F);
	//printf("Valor de la NAL:%X\n",pcNALFragUnit);

	return pcNALFragUnit;
}
/*
ErrVal
RtpPacker::xsetNalUnit(bool start, bool end){
	return Err::m_nOK;
}*/


ErrVal
RtpPacker::xInsertHeader(UChar * data, int size){

	int i=0;
	
	
	for(i=0;i<size;i++)
		rtpData[i+rtpPacketSize]=data[i];
		
	
	return i;
}

ErrVal
RtpPacker::xInsertData(UChar * data, int size){

	int i=0;

	for(i=0;i<size;i++)
				rtpData[i+rtpPacketSize]=data[fragment_index+i];

	return i;
}

ErrVal
RtpPacker::xInsertData(UChar * nal_unit,UChar * data, int size){

	int i=0;
	bool isNal=true;

	/*if(first_view)
		printf("Insertem dades en un paquet de només dades\n");
	else
		printf("Insertem dades en un paquet de NAL i dades\n");*/

	//Si els primers 4 caràcters de data[] són els mateixos que nal_data, llavors ens els saltem, si no, incluim tota la data al paquet rtpData. CAl tenir en compte que fragment_index sempre
	//l'hem de forçar a 0 al principi de cada Frame, no tan sols a les fragmentades
	

	for(int j=1;j<4 && isNal==true;j++) //El primer byte no el comptem per si està fragmentat
		if(data[j]!=nal_unit[j])
			isNal=false;

	/*if(isNal)
		printf("isNAL\n");
	else
		printf("NO isNAL\n");*/

	if(isNal) //cas de la NAL sola i de NAL+data
		for(i=0;i+4<size;i++)//Cas de que sigui la primera view no cal enviar res perquè ja ho hem enviat amb les NAL, si es la segona
				rtpData[i+rtpPacketSize]=data[fragment_index+i+4];
	else
		for(i=0;i<size;i++)//Si el paquet es fragmenta, però NO és el primer fragment de tota la frame, copiem totes les dades, ja que no hi ha NAL al principi
				rtpData[i+rtpPacketSize]=data[fragment_index+i];

	return i;

}
ErrVal
RtpPacker::xRTPnotMVC(UChar * data, int size){
	
	rtpPacketSize=0;
	//printf("Tamany del paquet inici: %d\n",rtpPacketSize);
	//insertem la Header
	xsetRTPHeader(false,false);
	rtpPacketSize+=xInsertHeader(pcRtpHeader.getRTPHeader(),HEADER_SIZE);
	//printf("Tamany del paquet de RTP: %d\n",rtpPacketSize);
	
	
	//printf("Tamany del paquet sumant la NAL: %d\n",rtpPacketSize);
	//Insertem la data (VIGILAR NAL + DATA)
	rtpPacketSize+=xInsertData(data,size);

	//printf("Dades que posem dins en el paquet RTP: ");

	//  for(int i=0;i<min(size,30);i++)
	//	  printf("%X ",data[i]);

	//  printf("\n");

	//printf("non-MVC data sent: %d\n",size);
	////printf("Tamany del paquet amb dades: %d\n",rtpPacketSize);

	//
	//printf("\n");


	
	  return Err::m_nOK;
}

ErrVal
RtpPacker::xRTPSingle(UChar * nal_unit,UChar * data, int size){

	rtpPacketSize=0;

	/*if(first_view)
		printf("Fragmentarem un paquet de només dades\n");
	else
		printf("Fragmentarem un paquet de NAL i dades\n");*/

	//printf("Tamany del paquet inici: %d\n",rtpPacketSize);
	//insertem la Header
	//printf("Seq Num: %d\n",pcRtpHeader.getSequenceNumber());
	rtpPacketSize+=xInsertHeader(pcRtpHeader.getRTPHeader(),HEADER_SIZE);
	//printf("Tamany del paquet de RTP: %d\n",rtpPacketSize);
	
	//Insertem la NAL
	if(!fragmented)
		rtpPacketSize+=xInsertHeader(nal_unit,NAL_HEADER_SIZE);
	else
		rtpPacketSize+=xInsertHeader(nal_unit_fragmented,NAL_FRAG_HEADER_SIZE);

	//printf("Tamany del paquet sumant la NAL: %d\n",rtpPacketSize);
	//Insertem la data (VIGILAR NAL + DATA)
	rtpPacketSize+=xInsertData(nal_unit,data,size);

	//printf("Dades RTP: SeqNum = %d, TimeStamp = %d\n",pcRtpHeader.getSequenceNumber(),pcRtpHeader.getTimestamp() );
	//printf("Tamany del paquet amb dades: %d\n",rtpPacketSize);

	/*printf("Valor del paquet:\n");

	int i=0;
	printf("Valor de la header RTP:\n");
	for(i=0;i<12;i++)
		printf(" %X ",(UChar)rtpData[i]);
	printf("\n");

	printf("Valor de la header NAL:\n");
	if(!fragmented)
		for(i=12;i<16;i++)
			printf(" %X ",(UChar)rtpData[i]);
	else
		for(i=12;i<17;i++)
			printf(" %X ",(UChar)rtpData[i]);
	
	printf("\n");

	/*printf("Valor del paquet en caràcters:\n");
	if(!fragmented)
		for(i=16;i<rtpPacketSize;i++)
			printf("%c",rtpData[i]);
	else
		for(i=17;i<rtpPacketSize;i++)
			printf("%c",rtpData[i]);
*/
	//printf("\n");


	
	  return Err::m_nOK;
}

ErrVal
RtpPacker::xFragment(UChar * nal_unit,UChar * data, int FullSize){

	pcFragmentNALUnit.start_bit=true;
	pcFragmentNALUnit.end_bit=false;
	

	
	while(FullSize){

		//Posem el marker bit a 0 i actualitzem la header de RTP
		pcRtpHeader.setRTPHeader(false,true);

		//printf("\n\nFragment de paquet\n");

		//Calculem si aquest serà l'ultim paquet, posem el bit end a 0 i el Marker bit del RTP a false
		if(FullSize<DATA_SIZE-1){
			pcFragmentNALUnit.end_bit=true;
			pcRtpHeader.setRTPHeader(false,false);
		}
			

		//Treure les primeres dades


		//Editar la  NAL
		xEditFragmentNAL(nal_unit);

		//Afegir RTP
		if(!pcFragmentNALUnit.end_bit){
			xRTPSingle(nal_unit,data,DATA_SIZE-1); //Si encara no és el ultim paquet, l'omplim del tot
			FullSize=FullSize-(DATA_SIZE-1);
		}
		else {
			xRTPSingle(nal_unit,data,FullSize); //Si ja és l'ultim, només amb el que queda
			FullSize=0;
		}

		m_apcUDPController->send(rtpData,rtpPacketSize);
		

		//Augmentem el Seq number
		increaseSeqNum();
		//increaseTimeStamp(160);

		// Posem l'index a on comença el segon fragment de dades
		fragment_index+=(DATA_SIZE-1);

		//printf("RtpPacker::xFragment()\n\n");
		
		pcFragmentNALUnit.start_bit=false;
	  
	}
	return Err::m_nOK;
  }



ErrVal
RtpPacker::xSetFragmentNAL(UChar nal_unit){
	//Li passem una NAL genèrica i omple els camps

	pcFragmentNALUnit.start_bit =nal_unit>>7;
	pcFragmentNALUnit.end_bit=(nal_unit&0x40)>>6;
	pcFragmentNALUnit.reserved_bit=(nal_unit&0x20)>>5;
	pcFragmentNALUnit.type=nal_unit&0x1F;

	return Err::m_nOK;
}


ErrVal
RtpPacker::xSetStandardNAL(UChar nal_unit){
	//Li passem una NAL genèrica i omple els camps

	pcStandardNALUnit.forbidden_bit=nal_unit>>7;
	pcStandardNALUnit.nri_bits=(nal_unit&0x60)>>5;
	pcStandardNALUnit.type=nal_unit&0x1F;

	return Err::m_nOK;
}

ErrVal
RtpPacker::xEditFragmentNAL(UChar * nal_unit){


	//Copiem les dades de la NAL genèrica 
	xSetStandardNAL(nal_unit[0]);

	//Editar el type de la  NAL de fragmentació
	pcFragmentNALUnit.type = pcStandardNALUnit.type; //posem el type de la NAL com a type de la NAL de fragmentació
	//printf("type de la fragmentada: %X\n",pcFragmentNALUnit.type);
	
	
	//Posem el valor 28 (x1C) al type de la NAL Genèrica
	pcStandardNALUnit.type=FRAGMENTED_TYPE;
	//printf("type de la Nal genèrica: %X\n",pcStandardNALUnit.type);


	//Afegim la NAL genèrica
	nal_unit_fragmented[0]=xgetNalUnit();

	//Afegir NAL de Fragmentació 
	nal_unit_fragmented[1]=xgetNalFragUnit();

	//Afegir la resta de dades de la NAL extesa per JMVC
	nal_unit_fragmented[2]=nal_unit[1];
	nal_unit_fragmented[3]=nal_unit[2];
	nal_unit_fragmented[4]=nal_unit[3];



	//printf("RtpPacker::xEditFragmentNAl()()\n");  
	  return Err::m_nOK;
  }


ErrVal
RtpPacker::recieveRTP(){

	UInt size = 0;

	Element element;

	element.MarkerBit = true;

	while(element.MarkerBit){ //Mirar una forma més elegant de veure quin és l'ultim paquet (Posar el marker bit a 0 si es l'ultim paquet ????

		//printf("\nEsperant un nou paquet\n\n");
		size = m_apcUDPController->recieve(dataRTP,1500); //guardem el paquet RTP+NAL+(NALfrag)+NALext+MVCdata

		/*printf("Size del paquet que hem rebut = %d\n", size);

		 printf("Dades rebudes del paquet UDP: ");

		  for(UInt i=0;i<min(size,30);i++)
			  printf("%X ",dataRTP[i]);

		  printf("\n");*/
		  element = m_apcRtpBuffer->processPacket(dataRTP,size);
		//Processar la capçalera RTP i guarda el paquet NAL+(NALfrag)+NALext+MVCdata a RTPBuffer
		m_apcRtpBuffer->PushElement(element);
	
	}
	return Err::m_nOK;
}

ErrVal
RtpPacker::xDefragment(){
	//printf("RtpPacker::xDefragment()\n");  
	  return Err::m_nOK;
  }

int
RtpPacker::test(){

	//printf("La Header que queda es: %X\n",xgetRTPHeader());  

  int size=1500;
  

  char nal_unit[4];
  nal_unit[0]=0x0E;
  nal_unit[1]=0x42;
  nal_unit[2]=0x00;
  nal_unit[3]=0x11;

  nal_unit[0]=0x6E;
  nal_unit[1]=0x0;
  nal_unit[2]=0x0;
  nal_unit[3]=0x0;

  //printf("Tamany de les dades: %d",sizeof("data de prova 0data de prova 1data de prova 2"));
  
  //pack(nal_unit,"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur enim mauris, vulputate vel scelerisque et, euismod ut sem. Etiam a orci nisl, eget feugiat eros. Etiam sodales odio at ligula tristique facilisis. Etiam sed odio quis lectus faucibus ultricies a tincidunt lorem. Aliquam eu malesuada nisl. Maecenas tristique volutpat congue. Etiam luctus faucibus justo, quis fringilla ligula viverra non. Etiam sapien eros, tincidunt eget venenatis vel, fringilla quis libero. Curabitur at quam quam, et blandit justo.  Aenean in nunc ligula, eu convallis turpis. Nam a mi ut sapien tincidunt varius sit amet eget nisl. Pellentesque interdum hendrerit sapien, id vestibulum metus euismod in. Proin faucibus fermentum augue eget aliquet. Etiam id dui id sem eleifend lobortis ut consectetur velit. Ut fringilla ornare urna, aliquet sollicitudin nisl sodales in. Nunc vitae risus et lacus pretium suscipit. Fusce a faucibus mauris. Morbi et felis a eros consectetur faucibus in sed urna. Mauris scelerisque aliquet tempus. Vivamus dapibus nulla a purus luctus congue. Pellentesque volutpat imperdiet tellus at ullamcorper. Suspendisse lobortis vestibulum turpis ac hendrerit.  Mauris non elit eget lectus gravida suscipit. Etiam sit amet odio non arcu luctus interdum. Vivamus sagittis, ligula vel semper congue, nulla dolor egestas nulla, nec congue felis lectus quis dui. Aliquam dolor nibh, pretium eu aliquam ac, semper vitae felis. Nunc ipsum nisl, tempus non pellentesque nec, suscipit id urna amet. ",1500);

	


	return 0;
}