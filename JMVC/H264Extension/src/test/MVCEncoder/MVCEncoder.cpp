#include "MVCEncoder.h"
#include "H264AVCEncoderTest.h"
#include "H264AVCCommonLib/CommonBuffers.h"
#include "AssemblerParameter.h"
#include "Assembler.h"
#include "MVCBStreamAssembler.h"
#include <iostream>
#include <fstream>
#include <string>




//creates the assembler file and returns the number of views to encode
int create_assembler_file(char* config_file){

	int num_views = -1;

	/*ifstream infile;
	
	char* buffer;
	//open Config_file and look for NumViewsMinusOne

	infile.open(config_file);

	while(!(infile.eof)){
		//Read the file since find the field NumViewsMinusOne
		getline(infile,buffer);
		printf("Data in Buffer:\n---------\n%s",buffer);
	}

	infile.close();
	
	//*/

	return num_views;
}

//Codifies all the views one by one
int run_encoder(int argc,char** argv,int n_views){


	printf("JMVC %s Encoder\n\n",_JMVC_VERSION_);


	  H264AVCEncoderTest*               pcH264AVCEncoderTest = NULL;

	  
		  
	  for(int i =0;i< n_views;i++){


		  itoa(i,argv[3],10);
		  
			printf("Encoding started for view %d = %s\n",i,argv[3]);

		  	RNOK( H264AVCEncoderTest::create( pcH264AVCEncoderTest ) );

			RNOKS( pcH264AVCEncoderTest->init   ( argc, argv ) );
			RNOK ( pcH264AVCEncoderTest->go     () );
			RNOK ( pcH264AVCEncoderTest->destroy() );

		}	 
		  

	  printf("Encoding finished \n\n------------------------------------------------\n\n");


	return 1;
}
//Codifies in parallel every view




//Assembles all the view files into a single H264 File
int run_assembler(int argc,char** argv){

	printf("Assemblng started\n");


	  //Passing the assembler file as parameter
	  argv[2] = argv[4];
	  


	  Assembler*          pcAssembler = NULL;
	  AssemblerParameter  cParameter;

	  printf( "JMVM %s MVC BitStream Assembler \n\n", "1.0");
	  

	  RNOKRS( cParameter.init       ( argc, argv ),   -2 );

	  
	  for( Int n = 0; n < 1; n++ )
	  {
		RNOKR( Assembler::create    ( pcAssembler ),  -3 );

		RNOKR( pcAssembler->init    ( &cParameter ),  -4 );

		RNOKR( pcAssembler->go      (),               -5 );

		RNOKR( pcAssembler->destroy (),               -6 );
	  }

	  printf("Assemblng finished\n");

	return 1;
}

int main(int argc, char** argv)
{

	//Storing the parameters to pass it to H264Encoder and MVCAssembler
	//The command line should be c:\>MVCEncoder.exe -vf <config_file> <num_views> <assembler_file>

	char * encoder_file = argv [2];
	char * num_views = argv[3];
	char * assembler_file = argv [4];

	//Convert the parameter
	int n_views = atoi(num_views);

	//create assembler.cfg

		//create_assembler_file(encoder_file);
	
		printf ("Encoder : %s\nNum Views: %s\nAssembler: %s\n-----------\n",encoder_file,num_views,assembler_file);

			

		printf("\n\nMVC Encoder and Assembler for 3D real-time transmissions\n\n");
			  
		//Codification of all of the n_views to encode
		run_encoder(4,argv,n_views);
		//run_RT_encoder(4,argv);
		//Assembling of the views to one single H264 file
		run_assembler(3,argv);
			  
		printf("\n\nMVC Encoder finished.\n---------------------\nAleix Paradell Navarro. EPSC-UPC. Barcelona 2012\n");
			
	  
		return 0;
}

