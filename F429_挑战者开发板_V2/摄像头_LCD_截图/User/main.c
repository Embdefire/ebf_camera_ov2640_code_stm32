/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   OV2640����ͷ��ʾ����
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� STM32  F429������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "stm32f4xx.h"
#include "./usart/bsp_debug_usart.h"
#include "./sdram/bsp_sdram.h"
#include "./lcd/bsp_lcd.h"
#include "./camera/bsp_ov2640.h"
#include "./systick/bsp_SysTick.h"
#include "./key/bsp_key.h"
#include "./bmp/bmp_ScreenShot.h"
#include "./font/fonts.h"
#include "./ff.h"
/*���������*/
uint32_t Task_Delay[NumOfTask];

uint8_t dispBuf[100];
OV2640_IDTypeDef OV2640_Camera_ID;

uint8_t fps=0;

//��ʾ֡�����ݣ�Ĭ�ϲ���ʾ����Ҫ��ʾʱ�����������Ϊ1���ɣ���������΢�������󣡣�
//������Һ����ʾ����ͷ����800*480���أ�֡��Ϊ14.2֡/�롣
//#define FRAME_RATE_DISPLAY 	1

// WIFI��SD������SDIO���ߣ���ʹ��SD��ʱ������WIFI�ĳ�ͻ
// ��Ҫ��WIFI��ʹ������PDN���͡�
void WIFI_PDNPIN_SET(void)
{
  /*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOB, ENABLE); 							   
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
  GPIO_Init(GPIOB, &GPIO_InitStructure);	
  
  GPIO_ResetBits(GPIOB,GPIO_Pin_13);  //����WiFiģ��
} 
extern __IO uint8_t dma_flag;
char OV2640_dma_iic_flag=0;
/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
int main(void)
{
  static FATFS fs;
	static FRESULT res_sd; 
	/*����ͷ��RGB LED�ƹ������ţ���Ҫͬʱʹ��LED������ͷ*/
	
  Debug_USART_Config();
	
		// ʧ��WIFI
	WIFI_PDNPIN_SET();
	
	/* ����SysTick Ϊ10us�ж�һ��,ʱ�䵽�󴥷���ʱ�жϣ�
	*����stm32fxx_it.c�ļ���SysTick_Handler����ͨ�����жϴ�����ʱ
	*/
	SysTick_Init();
	 
	/*��ʼ��Һ����*/
  LCD_Init();
  LCD_LayerInit();
  LTDC_Cmd(ENABLE);
	
	/*�ѱ�����ˢ��ɫ*/
  LCD_SetLayer(LCD_BACKGROUND_LAYER);
	LCD_SetTransparency(0xFF);
	
	LCD_Clear(LCD_COLOR_BLACK);
	
  /*��ʼ����Ĭ��ʹ��ǰ����*/
	LCD_SetLayer(LCD_FOREGROUND_LAYER);
	/*Ĭ�����ò�͸��	���ú�������Ϊ��͸���ȣ���Χ 0-0xff ��0Ϊȫ͸����0xffΪ��͸��*/
  LCD_SetTransparency(0xFF);
	LCD_Clear(TRANSPARENCY);
	
	LCD_SetColors(LCD_COLOR_RED,TRANSPARENCY);

	LCD_ClearLine(LINE(18));
  LCD_DisplayStringLine_EN_CH(LINE(18),(uint8_t* )" ģʽ:UXGA 800x480");

  CAMERA_DEBUG("STM32F429 DCMI ����OV2640����");

  /* ��ʼ������ͷGPIO��IIC */
  OV2640_HW_Init();

  /* ��ȡ����ͷоƬID��ȷ������ͷ�������� */
  OV2640_ReadID(&OV2640_Camera_ID);

   if(OV2640_Camera_ID.PIDH  == 0x26)
  {
//    sprintf((char*)dispBuf, "              OV2640 ����ͷ,ID:0x%x", OV2640_Camera_ID.PIDH);
//		LCD_DisplayStringLine_EN_CH(LINE(0),(uint8_t*)dispBuf);
    CAMERA_DEBUG("%x %x",OV2640_Camera_ID.Manufacturer_ID1 ,OV2640_Camera_ID.Manufacturer_ID2);

  }
  else
  {
    LCD_SetTextColor(LCD_COLOR_RED);
    LCD_DisplayStringLine_EN_CH(LINE(0),(uint8_t*) "         û�м�⵽OV2640�������¼�����ӡ�");
    CAMERA_DEBUG("û�м�⵽OV2640����ͷ�������¼�����ӡ�");

    while(1);
  }

	while(1)
 {
		res_sd = f_mount(&fs,"0:",1);
		if(res_sd == FR_OK)
		{
			printf("�ļ�ϵͳ���سɹ���");
			break;
		}
		else
			printf("�ļ�ϵͳ����ʧ�ܣ�");
		Delay(10);
 }
	 
 LCD_SetLayer(LCD_BACKGROUND_LAYER);
 
  OV2640_Init();
  OV2640_UXGAConfig();
  
	//ʹ��DCMI�ɼ�����
  DCMI_Cmd(ENABLE);
  DCMI_CaptureCmd(ENABLE);



	/*DMAֱ�Ӵ�������ͷ���ݵ�LCD��Ļ��ʾ*/
  while(1)
	{

//��ʾ֡�ʣ�Ĭ�ϲ���ʾ
#if FRAME_RATE_DISPLAY
		if(Task_Delay[0]==0)
		{
						
			LCD_SetColors(LCD_COLOR_RED,TRANSPARENCY);

			LCD_ClearLine(LINE(17));
			sprintf((char*)dispBuf, " ֡��:%.1f/s", (float)fps/5.0);
			
			/*���֡��*/
			LCD_DisplayStringLine_EN_CH(LINE(17),dispBuf);
			//����
			fps =0;
			
			
			Task_Delay[0]=5000; //��ֵÿ1ms���1������0�ſ������½�������


		}
			
#endif
		if(Key_Scan(KEY1_GPIO_PORT, KEY1_PIN) == KEY_ON)
    {
      uint8_t res = 0;
      
      printf("��ʼ��ͼ\r\n");
			
			OV2640_dma_iic_flag = 1;	//��ʼ��ͼ����Ҫ���ж������㣬�ر�DMA
			
			while(OV2640_dma_iic_flag){Delay(1);};
			
      res = screen_shot(0);  //LCD_FOREGROUND_LAYER
      
      if(res == 0)
      {
        printf("��ͼ�ɹ�\r\n");
      }
      else
      {
        printf("��ͼʧ��\r\n");
      }
			OV2640_dma_iic_flag = 0;
      dma_flag = 0;
			OV2640_start(); //����DMA
    }
		if(Key_Scan(KEY2_GPIO_PORT, KEY2_PIN) == KEY_ON)
    {
      OV2640_Stop();
      dma_flag = 1;
    }
	}
}



/*********************************************END OF FILE**********************/

