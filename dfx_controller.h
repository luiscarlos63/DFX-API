#ifndef DFX_CONTROLER_H
#define DFX_CONTROLER_H


#include <stdint.h>
#include <stdlib.h>



// ---------------------------------- defines -----------------------
#define XDFXC_DEVICE_ID         XPAR_DFX_CONTROLLER_0_DEVICE_ID

#define XDFXC_VS_CONST_ID		XPAR_DFX_CONTROLLER_0_VS_CONST_ID
#define XDFXC_VS_CONST_45_ID	XPAR_DFX_CONTROLLER_0_VS_CONST_RM_CONST_45_ID

#define  XDFXC_VS_MULT_ID		XPAR_DFX_CONTROLLER_0_VS_MULT_ID
#define  XDFXC_VS_MULT_5_ID		XPAR_DFX_CONTROLLER_0_VS_MULT_RM_MUL5_ID

#define  XDFXC_VS_ADD_ID		XPAR_DFX_CONTROLLER_0_VS_ADD_ID
#define  XDFXC_VS_ADD_35_ID		XPAR_DFX_CONTROLLER_0_VS_ADD_RM_ADD_35_ID


typedef struct
{
    uint32_t* data;
    uint32_t size;
    
}bitstream_t;

/*
*    tudo o que é necessario com o carreganto do bitrsream do SD_Card para a DRAM
*                        SD_card -> DRAM
*/
uint8_t bitstream_init(bitstream_t*, const char* file_path);





/*
*    init do dfx standalone
*    o que é necessario configurar no DFX_C para que este esteja pronto a operar
*/
uint8_t dfx_init();


/*
 * Diz ao DFX Controller o ADDRESS e o SIZE do bitstream
 * Neste momento so funciona para uma RP (CONST)
 */
uint8_t dfx_bitstrm_set(const bitstream_t*);


/*
*    todas as operaçoes relacionadas com carregamento de um bitstream
*    (com origem da DRAM para uma RP)
*                        DRAM -> RM -> RP
*                           DFX_trigger
*/
uint8_t dfx_load(const bitstream_t*);


/*
 * Devolve o estado em que se encontra
 */
//dfx_status();


















/*
    assumptions:
        - o modulo ID 1


    TEREFAS
        testar, metendo o 37 a funcionar    (caso 1)
        novo branch do GIT

        

*/

/*

    caso 1:
        1 RP com 1 RM para 1 RB
        EX: Const

        RP ----- RM 45 ------ Bit Const 45

        - Sem a API: sim
        - Com a API:
        
    caso 1.2:
        1 RP com RMn para RBn
        EX: Const

        		 +----- RM 45 ------ Bit Const 45
        RP ----- |
        		 +----- RM 37 ------ Bit Const 37

		- Sem a API: sim
        - Com a API:

    caso 2: usar uma RP com apenas um RM associado, podendo carregar varios bitstreams (RB) para essa RP (atraves do mesmo RM).
        1 RP com 1 RM para RBn

        EX: Const
        						+----- Bit Const 45
        RP ----- RM-------------|
        	(RM generico)		+----- Bit Const 37
        	(isto obriga a trocar o endereço do bitstream sempre que é preciso trocar o bitstream)

        - Sem a API: sim
        - Com a API:

    caso 2.2: usar varias RPs com cada uma com 1 RM associado. podendo, apenas com esse RM carregar varios bitstreams (RB) para essa RP.
        RPn com RMn para carregar RBnm

        EX:
         							+----- Bit Const 45
        RP_const-----RM_const-------|
        							+----- Bit Const 37

        							+----- Bit Add 35
        RP_add-------RM_add---------|
        							+----- Bit Add 65

         							+----- Bit Mult 5
        RP_mult------RM_mult--------|
        							+----- Bit Mult 50


    caso 3: usar varias RPs com cada uma com 1 RM associado. Este caso sugere que qualquer IP pode ser
    carregado para qualquer RP.
    A dificuldade surge em como lidar com as limitaçoes de implementação. Um bitstream parcial que foi
    implementado para uma certa RP (por exemplo RP_1) nao poder ser usado noutra RP (por exemplo RP_2)
    mesmo que o IP que implmentam seja extamente igual.

    No entanto, devido à natureza estatica das RP (uma vez que as partiçoes tem de ser definidas à priori
    e sao NAO dinamicas),
        RPn com RMn para carregar IPm
								Em vez de bitstreams, referimo-nos a acelaradores (ou IPs)

							  +----- IP_1	(Bit IP_1_RP_1)
							  |
		RP_1 ------RM_1-------+----- IP_2	(Bit IP_2_RP_1)
							  |
							  +----- IP_n	(Bit IP_n_RP_1)


							  +----- IP_1	(Bit IP_1_RP_2)
							  |
		RP_2 ------RM_2-------+----- IP_2	(Bit IP_2_RP_2)
							  |
							  +----- IP_n	(Bit IP_n_RP_2)


							  +----- IP_1	(Bit IP_1_RP_n)
							  |
		RP_3 ------RM_3-------+----- IP_2	(Bit IP_2_RP_n)
							  |
							  +----- IP_n	(Bit IP_n_RP_n)

*/
#endif
