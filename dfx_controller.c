#include "dfx_controller.h"
#include "ff.h"








//---------------------- For DFX Controller -----------------------------------
static XPrc Prc;
static XPrc_Config *XPrcCfgPtr;
static u32 prc_init;
static u32 prc_status;
static u32 prc_status_state;
static u32 prc_status_last_state;
static u32 prc_status_err;

u32 rm_loading   	= 0;
u32 const_loading  	= 0;
u32 mult_loading	= 0;
u32 add_loading		= 0;


static FATFS  fatfs;


int SD_Init()
{
	FRESULT rc;
	TCHAR *Path = "0:/";
	rc = f_mount(&fatfs,Path,0);
	if (rc) {
		//xil_printf(" ERROR : f_mount returned %d\r\n", rc);
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

int SD_Eject()
{
	FRESULT rc;
	TCHAR *Path = "0:/";
	rc = f_mount(0,Path,1);
	if (rc) {
		//xil_printf(" ERROR : f_mount returned %d\r\n", rc);
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}



int ReadFile(char *FileName, u32** DestinationAddress, u32* size)
{
	FIL fil;
	FRESULT rc;
	UINT br;
	u32 file_size;
	rc = f_open(&fil, FileName, FA_READ);
	if (rc) {
		xil_printf(" ERROR : f_open returned %d\r\n", rc);
		return XST_FAILURE;
	}
	file_size = f_size(&fil);	//file_size = fil.fsize;
	*size = file_size;

	//space allocation
	*DestinationAddress = (u32 *)malloc(file_size * sizeof(char)); // Enough memory for the file

	rc = f_lseek(&fil, 0);
	if (rc) {
		xil_printf(" ERROR : f_lseek returned %d\r\n", rc);
		return XST_FAILURE;
	}
	rc = f_read(&fil, (void**) *DestinationAddress, file_size, &br);
	if (rc) {
		xil_printf(" ERROR : f_read returned %d\r\n", rc);
		return XST_FAILURE;
	}
	rc = f_close(&fil);
	if (rc) {
		xil_printf(" ERROR : f_close returned %d\r\n", rc);
		return XST_FAILURE;
	}
	Xil_DCacheFlush();
	return XST_SUCCESS;
}






/////////////////////////////////////////////	Bitstream	///////////////////////////////
uint32_t bitstream_get_rp_id(bitstream_t* bit)
{
	return bit->tile_id;
}
/*
*    Everything necessary to laod the bitstreams from SD card to DRAM.
*                        SD_card -> DRAM
*/
uint8_t bitstream_init(bitstream_t* bit, const char* file_path)
{
	int status;
	uint32_t *file_ptr;
	uint32_t file_size;

	//mount SD card
	status = SD_Init();
	if (status != XST_SUCCESS)
		return XST_FAILURE;

	//read bitstream from file in the SD card
	status = ReadFile(file_path, &file_ptr, &file_size);
	if (status != XST_SUCCESS)
		return XST_FAILURE;

	//set the bitstream struct
	bit->data = file_ptr;
	bit->size = file_size;

	//unmount SD card
	status = SD_Eject();
	if (status != XST_SUCCESS)
		return XST_FAILURE;

	return status;
}


/////////////////////////////////////////////	IP		///////////////////////////////

uint8_t ip_init_ip(ip_t* ip, uint32_t ip_id)
{
	if(!ip_id)	//0 is reserved
		return XST_FAILURE;

	ip->id = ip_id;
	return XST_SUCCESS;
}

/*
 * add a bitstream to an IP
 * @ ip			- IP instance
 * @ bit 		- Bitstream to associate
 * @ tile_id	- Tile to which the bit is associated
 */
uint8_t ip_add_bitstream(ip_t* ip, bitstream_t* bit, uint32_t tile_id)
{
	if(tile_id >= TILE_NUM_OF_TILES)
		return XST_FAILURE;

	ip->bits[tile_id] = bit;
	return XST_SUCCESS;
}


////////////////////////////////////////////	TILE	//////////////////////////////////

/*
 * initilaizes a tile
 */
uint8_t tile_init(tile_t* til)
{
	til->ip_running = 0;
	til->status = AVAIL;
}
//////////////////////////////////////////////	DFX //////////////////////////////////
/*
 * Sets the ADDRESS and SIZE of the
 */

uint8_t dfx_set_bitstream_source(dfx_t* dfx, uint32_t tile_id, bitstream_t* bit)
{
	//DFX controller shutdown
    XPrc_SendShutdownCommand(&dfx->dfx_ctrl, tile_id);
	while(XPrc_IsVsmInShutdown(&dfx->dfx_ctrl, tile_id)==XPRC_SR_SHUTDOWN_OFF);

    XPrc_SetBsSize   (&dfx->dfx_ctrl, tile_id, 0,  bit->size);
    XPrc_SetBsAddress(&dfx->dfx_ctrl, tile_id, 0,  (uint32_t)bit->data);

    XPrc_SendRestartWithNoStatusCommand(&dfx->dfx_ctrl, tile_id);
	while(XPrc_IsVsmInShutdown(&dfx->dfx_ctrl, tile_id)==XPRC_SR_SHUTDOWN_ON);
}

/*
*    initialization of dfx standalone
*    Everything necessary to get the DFX controller up and running.
*/
uint8_t dfx_init(dfx_t* dfx)
{
	uint32_t prc_init;

	//DFX controller IP init
    dfx->dfx_conf = XPrc_LookupConfig(XDFXC_DEVICE_ID);
	if (NULL == dfx->dfx_conf) {
	return XST_FAILURE;
	}

	prc_init = XPrc_CfgInitialize(&dfx->dfx_ctrl, dfx->dfx_conf,dfx->dfx_conf->BaseAddress);
	if (prc_init != XST_SUCCESS) {
	return XST_FAILURE;
	}


	//Tile init
	tile_init(&dfx->tiles[0]);
	tile_init(&dfx->tiles[1]);
	tile_init(&dfx->tiles[2]);

	dfx->tiles[0].base_ptr = (uint32_t*)TILE_1_ADDR;
	dfx->tiles[1].base_ptr = (uint32_t*)TILE_2_ADDR;
	dfx->tiles[2].base_ptr = (uint32_t*)TILE_3_ADDR;

	return XST_SUCCESS;
}


/*
*    Every operation necessary to load the bitstream to the RP
*                        DRAM -> RP
*                           DFX_trigger
*/
uint8_t dfx_load_ip(dfx_t* dfx, ip_t* ip)
{
	uint32_t tile_id = 0;
	int i = 0;

	//see what tile is available
	while(i < TILE_NUM_OF_TILES){
		if(dfx->tiles[i].status == AVAIL)
		{
			tile_id = i;
			break;
		}
		i++;
	}

	if(tile_id >= TILE_NUM_OF_TILES)
		return XST_FAILURE;

	//set the "to be run" bitstream info
	dfx_set_bitstream_source( dfx, 								//dfx instance
							  tile_id,				//choosen tile instance
							  ip->bits[tile_id]);	//ip to be run	&   in the corresponding tile

	dfx->tiles[tile_id].ip_running = ip->id;
	//sent the ATTEST module the "to be run" bitstream info
	/*
	 *	send_ip_id
	 *	send_tile_ip
	 */

	//Send trigger to dfx controller
	if (XPrc_IsSwTriggerPending(&dfx->dfx_ctrl, tile_id, NULL)==XPRC_NO_SW_TRIGGER_PENDING)
	{
	    XPrc_SendSwTrigger(&dfx->dfx_ctrl, tile_id, 0);
	}
	return XST_SUCCESS;
}




