/***************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include <stdint.h>
#include "platform.h"
#include "xil_printf.h"
#include "xil_io.h"
#include <xstatus.h>
#include "xil_cache.h"
#include <xil_types.h>

#include "dfx_controller.h"


// ----------------------------- File names ---------------------------------
const char* ip_const33_rp_1 = "rp1_33.bin";
const char* ip_const44_rp_1 = "rp1_44.bin";
const char* ip_const55_rp_1 = "rp1_55.bin";
const char* ip_const33_rp_2 = "rp2_33.bin";
const char* ip_const44_rp_2 = "rp2_44.bin";
const char* ip_const55_rp_2 = "rp2_55.bin";
const char* ip_const33_rp_3 = "rp3_33.bin";
const char* ip_const44_rp_3 = "rp3_44.bin";
const char* ip_const55_rp_3 = "rp3_55.bin";



int main()
{

	u32 	  * pcapCtrlPtr 	= (u32*)0x00FFCA3008;
	u_int32_t * RP_1_addr 	= (u_int32_t*)0xA0020000;
	u_int32_t * RP_2_addr 	= (u_int32_t*)0xA0030000;
	u_int32_t * RP_3_addr 	= (u_int32_t*)0xA0040000;
	int command = 0;
	u32 status;

	bitstream_t bit_ip_const33_rp_1;
	bitstream_t bit_ip_const44_rp_1;
	bitstream_t bit_ip_const55_rp_1;
	bitstream_t bit_ip_const33_rp_2;
	bitstream_t bit_ip_const44_rp_2;
	bitstream_t bit_ip_const55_rp_2;
	bitstream_t bit_ip_const33_rp_3;
	bitstream_t bit_ip_const44_rp_3;
	bitstream_t bit_ip_const55_rp_3;

	ip_t ip_const33;
	ip_t ip_const44;
	ip_t ip_const55;

	dfx_t dfx;


    init_platform();
    disable_caches();

    // ------------------------ PCAP disable ----------------------------------
    printf("\nPCAP_CNTR = %X", (u32)*pcapCtrlPtr);
    printf("\nChanging CAP control to ICAP");
    status = (*pcapCtrlPtr) & (~(u32)1);
    *pcapCtrlPtr = status;
    printf("\nNew PCAP_CNTR = %X", (u32)*pcapCtrlPtr);


	//------------------------- IPs init -------------------------------------------------
    ip_init_ip(&ip_const33, 1);
    ip_init_ip(&ip_const44, 2);
    ip_init_ip(&ip_const55, 3);

    //-------------------------loading Bitstreams  to memory and associate to the IP -----
    bitstream_init(&bit_ip_const33_rp_1, ip_const33_rp_1);
    bitstream_init(&bit_ip_const33_rp_2, ip_const33_rp_2);
    bitstream_init(&bit_ip_const33_rp_3, ip_const33_rp_3);
    ip_add_bitstream(&ip_const33, &bit_ip_const33_rp_1, TILE_1_ID);
    ip_add_bitstream(&ip_const33, &bit_ip_const33_rp_2, TILE_2_ID);
    ip_add_bitstream(&ip_const33, &bit_ip_const33_rp_3, TILE_3_ID);

    bitstream_init(&bit_ip_const44_rp_1, ip_const44_rp_1);
    bitstream_init(&bit_ip_const44_rp_2, ip_const44_rp_2);
    bitstream_init(&bit_ip_const44_rp_3, ip_const44_rp_3);
    ip_add_bitstream(&ip_const44, &bit_ip_const44_rp_1, TILE_1_ID);
	ip_add_bitstream(&ip_const44, &bit_ip_const44_rp_2, TILE_2_ID);
	ip_add_bitstream(&ip_const44, &bit_ip_const44_rp_3, TILE_3_ID);

    bitstream_init(&bit_ip_const55_rp_1, ip_const55_rp_1);
    bitstream_init(&bit_ip_const55_rp_2, ip_const55_rp_2);
    bitstream_init(&bit_ip_const55_rp_3, ip_const55_rp_3);
	ip_add_bitstream(&ip_const55, &bit_ip_const55_rp_1, TILE_1_ID);
	ip_add_bitstream(&ip_const55, &bit_ip_const55_rp_2, TILE_2_ID);
	ip_add_bitstream(&ip_const55, &bit_ip_const55_rp_3, TILE_3_ID);
    printf("\nBitstreams loaded into memory ");



    //-------------------------- Set IPs --------------------------------------------------


    printf("\nIPs created");

    //-------------------------- DFX init -------------------------------------------------
	if(dfx_init(&dfx)){
		printf("Error initializaing DFX Controller");
		return -1;
	}

    print("\n\r\n\r*** Dynamic Function eXchange SW Trigger ***\n\r");

    while(1)
	{
		printf("\n\n\n\r1 - read from RP_1");
		printf("\n2 - read from RP_2");
		printf("\n3 - read from RP_3");
		printf("\n4 - Load IP const_33");
		printf("\n5 - Load IP const_44");
		printf("\n6 - Load IP const_55");
		printf("\nOption: ");
		scanf("%d", &command);

		switch(command)
		{
			case 1:
				printf("\nRP_1 value = %X", *RP_1_addr);
				break;
			case 2:
				printf("\nRP_2 value = %X", *RP_2_addr);
				break;
			case 3:
				printf("\nRP_3 value = %X", *RP_3_addr);
				break;
				/*
				 * Proof of concept
				 */
			case 4:
				printf("\n\rLoad IP const_33");
				dfx_load_ip(&dfx, &ip_const33);
				break;
			case 5:
				printf("\n\rLoad IP const_44");
				dfx_load_ip(&dfx, &ip_const44);
				break;
			case 6:
				printf("\n\rLoad IP const_55");
				dfx_load_ip(&dfx, &ip_const55);
				break;
		default: break;
		}	// end of "switch(command)"

	}//end of while(1)
    cleanup_platform();
    return 0;
}
