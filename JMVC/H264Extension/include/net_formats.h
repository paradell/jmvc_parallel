/*
 * net_formats.cpp
 *
 *  Created on: 08/01/2011
 *      Author: lechuck
 */
#include <arpa/inet.h>
/**
 * estructura rtp con el añadido para video sin comprimir del rfc 3550
 */
/*
 * RTP Payload Format 
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |V=2|P|X|  CC   |M|     PT      |       sequence number         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                           timestamp                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |           synchronization source (SSRC) identifier            |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 */
typedef struct rtp_4175base_header{
	u_int8_t csrc_count:4,
		extension:1,
		padding:1,
		version:2;
	u_int8_t payload_type:7,
		marker:1;
	u_int16_t seqno_low;
	u_int32_t timestamp;
	u_int32_t ssrc;
	
} rtp_4175base_header_t;

static int size_rtp_4175base_header_t = 14;



/*
 *  esta estructura debe aparecer una vez por linea
 *
 *	lenght -> bytes que se envian de la linea
 * 	line_no -> numero de la linea en la imagen (empezando por la 0?)
 *  field_id -> 1 si es entrelazado
 * 	continuation -> las cabeceras de linea se ponen seguidas,
 *  			este campo a 0 indica que no tiene otra cabecera de linea mas detras, a 1 indica que si
 * 	offset -> indica que pixel de la linea es el primero que se encuentra en el paquete,
 *  			esta pensado para lineas fragmentadas (empezando por el 0)
 *  				ej: linea de 1920px / 3 paquetes = 640 pixels por paquete
 *  					offset primer paquete = 0
 *  					offset segundo paquete = 640
 *  					offset tercer paquete = 1280
 */
/*typedef struct rtp_4175line_header{
	u_int16_t lenght;
	u_int16_t line_no:15,
		field_id:1;
	u_int16_t offset:15,
		continuation:1;
}rtp_4175line_header_t;*/
typedef struct rtp_4175line_header{
	u_int16_t lenght;
	u_int16_t field_id__line_no;
	u_int16_t continuation__offset;
}rtp_4175line_header_t;

static int size_rtp_4175line_header_t = 6;
