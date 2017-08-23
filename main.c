#include "chip.h"
#include <stdio.h>
#include <string.h>
#include "SPI_MSD_Driver.h"
#include "diskio.h"
#include "ff.h"

FATFS fs;         /* Work area (file system object) for logical drive */
FIL fsrc;         /* file objects */   
FRESULT res;
UINT br;


char path[512]="0:";
char textFileBuffer[50];   
uint16_t data[2];

void __delay_ms(uint32_t time){

	for(uint32_t t=time*500000;t>0;t--);

}
int SD_TotalSize(void){
    FATFS *fs;
    DWORD fre_clust;        

    res = f_getfree("0:", &fre_clust, &fs);  /* 必须是根目录，选择磁盘0 */
    if ( res==FR_OK ) 
    {
	  /* Print free space in unit of MB (assuming 512 bytes/sector) */
      printf("\r\n%d MB total drive space.\r\n"
             "%d MB available.\r\n",
           ( (fs->n_fatent - 2) * fs->csize ) / 2 /1024 , (fre_clust * fs->csize) / 2 /1024 );
		
	  return 1;
	}
	else 
	  return 0;   
}

void Open_Periphericals(void){
	
	Chip_GPIO_Init(LPC_GPIO);
	
	/*Funciones para configurar PINs*/

	/*Configuracion de Pines del USART*/
	
	Chip_IOCON_PinMuxSet(LPC_IOCON,IOCON_PIO1_6, FUNC1);
	Chip_IOCON_PinMuxSet(LPC_IOCON,IOCON_PIO1_7,FUNC1);
	
	/*Configuracion de Pines del ADC*/
	
	Chip_IOCON_PinMuxSet(LPC_IOCON,IOCON_PIO0_11,FUNC2);
	Chip_IOCON_PinMuxSet(LPC_IOCON,IOCON_PIO1_0,FUNC2);

	/*Configuracion de Pines del SSP0*/
	
	Chip_IOCON_PinMuxSet(LPC_IOCON,IOCON_PIO0_9,FUNC1);
	Chip_IOCON_PinMuxSet(LPC_IOCON,IOCON_PIO0_8,FUNC1);
	Chip_IOCON_PinLocSel(LPC_IOCON,IOCON_SCKLOC_PIO2_11);
	Chip_IOCON_PinMuxSet(LPC_IOCON,IOCON_PIO2_11,FUNC1);

	/*Configuracion de Pines de GPIO*/
	
	Chip_IOCON_PinMuxSet(LPC_IOCON,IOCON_PIO1_10,FUNC0);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO,1,10);
	
	Chip_IOCON_PinMuxSet(LPC_IOCON,IOCON_PIO1_11,FUNC0);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO,1,11);
	
	/*
	 * Inicializa UART0  a 115200 bps
	 *
	 *
	 * 
	 */


	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART,115200);
	
	/*
	 * Inicializa ADC  a 400KSPS
	 *
	 *
	 * 
	 */
	
	ADC_CLOCK_SETUP_T AdcSetup;
	
	Chip_ADC_Init(LPC_ADC,&AdcSetup);
	

	Chip_SSP_Disable(LPC_SSP0);
	Chip_SSP_Init(LPC_SSP0);
	Chip_SSP_Enable(LPC_SSP0);
	
	
}

int main(void){
	
	SystemCoreClockUpdate();
	
	Open_Periphericals();
	/*Main Code Here*/
	if( _card_insert() == 0 ){
		
		printf("-- SD card detected OK \r\n");
		
	}
	else
	{
		printf("-- Please connect a SD card \r\n"     );
		while( _card_insert() != 0 );
		printf("-- SD card connection detected \r\n");
		__delay_ms(10);
	}
    
	f_mount(&fs,"",0);	
  
	res = f_open( &fsrc , "Simple20.csv" , FA_CREATE_NEW | FA_WRITE | FA_OPEN_APPEND);		

	if ( res == FR_OK )
	{ 
		/* Write buffer to file */
		
		for(uint32_t cont = 0; cont<50; cont++){
			
			for(uint8_t ADC_Cont = 0; ADC_Cont<2;ADC_Cont++){
				
				Chip_ADC_EnableChannel(LPC_ADC,ADC_Cont,ENABLE);
				Chip_ADC_SetStartMode(LPC_ADC,ADC_START_NOW,ADC_TRIGGERMODE_RISING);
				
				while(Chip_ADC_ReadStatus(LPC_ADC,ADC_Cont,ADC_DR_DONE_STAT)==RESET);
				
				if(Chip_ADC_ReadValue(LPC_ADC,ADC_Cont,&data[ADC_Cont]) == ERROR){
					
					printf("Error\r\n");
				
				}

				
				Chip_ADC_EnableChannel(LPC_ADC,ADC_Cont,DISABLE);
				
			}
			
			sprintf(textFileBuffer,"%u,%u\r\n",data[0],data[1]);
			
			
			res = f_write(&fsrc, textFileBuffer,(unsigned) strlen(textFileBuffer), &br);     

			if(res == FR_OK){
			
				printf("Simple20.csv successfully created        %u,%u\r\n",br,cont);
				
			}else{
			
				printf("Simple20.csv successfully created with some Error        %u\r\n",res);
			
			
			}
			
			__delay_ms(10);
			
		}
			 
	}
	else if ( res == FR_EXIST )
	{
		
		printf("Demo.TXT created in the disk      \r\n");
		
	}
	
	f_close(&fsrc);
	SD_TotalSize();
	
	f_mount(0,"",0);
	

	for(;;){
	
		
	
	}

	return 1;
	
}
