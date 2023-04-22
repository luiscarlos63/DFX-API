#include "dfx_controller.h"
#include "ff.h"
#include "xparameters.h"
#include "xprc.h"



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
		xil_printf(" ERROR : f_mount returned %d\r\n", rc);
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
		xil_printf(" ERROR : f_mount returned %d\r\n", rc);
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




uint8_t dfx_init()
{
    XPrcCfgPtr = XPrc_LookupConfig(XDFXC_DEVICE_ID);
	if (NULL == XPrcCfgPtr) {
	return XST_FAILURE;
	}

	prc_init = XPrc_CfgInitialize(&Prc, XPrcCfgPtr, XPrcCfgPtr->BaseAddress);
	if (prc_init != XST_SUCCESS) {
	return XST_FAILURE;
	}
}


uint8_t dfx_bitstrm_set(const bitstream_t* bit)
{
	//DFX controller shutdown
    XPrc_SendShutdownCommand(&Prc, XDFXC_VS_CONST_ID);
	while(XPrc_IsVsmInShutdown(&Prc, XDFXC_VS_CONST_ID)==XPRC_SR_SHUTDOWN_OFF);

    XPrc_SetBsSize   (&Prc, XDFXC_VS_CONST_ID, XDFXC_VS_CONST_45_ID,  bit->size);
    XPrc_SetBsAddress(&Prc, XDFXC_VS_CONST_ID, XDFXC_VS_CONST_45_ID,  bit->data);

    XPrc_SendRestartWithNoStatusCommand(&Prc, XDFXC_VS_CONST_ID);
	while(XPrc_IsVsmInShutdown(&Prc, XDFXC_VS_CONST_ID)==XPRC_SR_SHUTDOWN_ON);
}



uint8_t dfx_load(const bitstream_t* source)
{
    //Diz ao DFX Controller o ADDRESS e o SIZE do bitstream que sera carregado agora.
	dfx_bitstrm_set(source);


    if (XPrc_IsSwTriggerPending(&Prc, XDFXC_VS_CONST_ID, NULL)==XPRC_NO_SW_TRIGGER_PENDING) {
        
        XPrc_SendSwTrigger(&Prc, XDFXC_VS_CONST_ID, XDFXC_VS_CONST_45_ID);
    }


}

