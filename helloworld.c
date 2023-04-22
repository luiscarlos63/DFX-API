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
const char* rmConst45_name = "cnt45bs.bin";
const char* rmConst37_name = "cnt35bs.bin";



int main()
{

	u32 	  * pcapCtrlPtr 	= (u32*)0x00FFCA3008;
	u_int32_t * RP_const_addr 	= (u_int32_t*)0xA0010000;
	int command = 0;
	u32 status;

	bitstream_t bit_const37;
	bitstream_t bit_const45;


    init_platform();
    disable_caches();

    // ------------------------ PCAP disable ----------------------------------
    printf("\nPCAP_CNTR = %X", (u32)*pcapCtrlPtr);
    printf("\nChanging CAP control to ICAP");
    status = (*pcapCtrlPtr) & (~(u32)1);
    *pcapCtrlPtr = status;
    printf("\nNew PCAP_CNTR = %X", (u32)*pcapCtrlPtr);

    //-------------------------loading Bitstreams  to memory -----------------------------
    bitstream_init(&bit_const37, rmConst37_name);
    bitstream_init(&bit_const45, rmConst45_name);
    printf("\nBitstreams loaded into memory ");

    //++++++++++++++++++++++++++++++++ DFX Controller Driver initialize +++++++++++++++++++++++++++++++++++++++++++
    if(dfx_init()){
    	printf("Error initializaing DFX Controller");
    	return -1;
    }


    print("\n\r\n\r*** Dynamic Function eXchange SW Trigger ***\n\r");

    while(1)
	{
		printf("\n\n1 - read from RP_const");
		printf("\n2 - bit_const_37");
		printf("\n3 - bit_const_45");
		printf("\nOption: ");
		scanf("%d", &command);

		switch(command)
		{
			case 1:
				printf("\nRP_const value = %X", *RP_const_addr);
				break;

				/*
				 * Proof of concept
				 */
			case 2:
				printf("\nReconfiguring RP_const with bit_const_37...\n\r");
				dfx_load(&bit_const37);
				break;
			case 3:
				printf("\nReconfiguring RP_const with bit_const_45...\n\r");
				dfx_load(&bit_const45);
				break;
		default: break;
		}	// end of "switch(command)"

	}//end of while(1)
    cleanup_platform();
    return 0;
}