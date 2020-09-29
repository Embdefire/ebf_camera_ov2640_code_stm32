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
  * ʵ��ƽ̨:Ұ�� STM32  F407������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "stm32f4xx.h"
#include "./usart/bsp_debug_usart.h"
#include "./lcd/bsp_nt35510_lcd.h"
#include "./camera/bsp_ov2640.h"
#include "./systick/bsp_SysTick.h"



/*���������*/
uint32_t Task_Delay[NumOfTask];

char dispBuf[100];
OV2640_IDTypeDef OV2640_Camera_ID;

uint8_t fps=0;


extern uint16_t img_width, img_height;

/**
  * @brief  Һ����������ʹ������ͷ���ݲɼ�
  * @param  ��
  * @retval ��
  */
void ImagDisp(void)
{
		//ɨ��ģʽ������
    NT35510_GramScan(5);
    LCD_SetFont(&Font16x32);
		LCD_SetColors(RED,BLACK);
	
    NT35510_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	/* ��������ʾȫ�� */
    NT35510_DispStringLine_EN(LINE(0),"BH 4.8 inch LCD + OV2640");
	
		/*DMA������ݴ��䵽Һ���������������ݰ��������� */
    NT35510_OpenWindow(0,0,img_width,img_height);	
		
		OV2640_Capture_Control(ENABLE);
}

/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
int main(void)
{	
	/*ͼ���С���޸�������ֵ���ɸı�ͼ��Ĵ�С*/
	//Ҫ��Ϊ4�ı���
	img_width=800;
	img_height =480;

	
	NT35510_Init ();         //LCD ��ʼ��
	
	LCD_SetFont(&Font16x32);
	LCD_SetColors(RED,BLACK);

  NT35510_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	/* ��������ʾȫ�� */

  Debug_USART_Config();   
	
	/* ����SysTick Ϊ1ms�ж�һ��,ʱ�䵽�󴥷���ʱ�жϣ�
	*����stm32fxx_it.c�ļ���SysTick_Handler����ͨ�����жϴ�����ʱ
	*/
	SysTick_Init();

	//Һ��ɨ�跽��
	NT35510_GramScan(5);
	
  CAMERA_DEBUG("STM32F407 DCMI ����OV2640����");

  /* ��ʼ������ͷGPIO��IIC */
  OV2640_HW_Init();   

  /* ��ȡ����ͷоƬID��ȷ������ͷ�������� */
  OV2640_ReadID(&OV2640_Camera_ID);

  if(OV2640_Camera_ID.PIDH  == 0x26)
  {
    sprintf(dispBuf, "OV2640 camera,ID:0x%x", OV2640_Camera_ID.PIDH);
		NT35510_DispStringLine_EN(LINE(0),dispBuf);
    CAMERA_DEBUG("��⵽����ͷ %x %x",OV2640_Camera_ID.Manufacturer_ID1 ,OV2640_Camera_ID.Manufacturer_ID2);

  }
  else
  {
    LCD_SetTextColor(RED);
    NT35510_DispString_EN(10,10,"Can not detect OV2640 module,please check the connection!");
    CAMERA_DEBUG("û�м�⵽OV2640����ͷ�������¼�����ӡ�");

    while(1);  
  }

  /*ʹ�üĴ������ʼ������ͷ*/
  OV2640_Init();
  OV2640_UXGAConfig(); 
  
	/*DMAֱ�Ӵ�������ͷ���ݵ�LCD��Ļ��ʾ*/
	ImagDisp();

  while(1)
	{
		//ʹ�ô������֡��
		if(Task_Delay[0]==0)
		{						
			/*���֡��*/
			CAMERA_DEBUG("\r\n֡��:%.1f/s \r\n", (double)fps/5.0);
			//����
			fps =0;			
			
			Task_Delay[0]=5000; //��ֵÿ1ms���1������0�ſ������½�������

		}		
	}
}



/*********************************************END OF FILE**********************/

