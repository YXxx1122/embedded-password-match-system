/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "zlg7290.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#define ZLG_READ_ADDRESS1         0x01
#define ZLG_READ_ADDRESS2         0x10
#define ZLG_WRITE_ADDRESS1        0x10
#define ZLG_WRITE_ADDRESS2        0x11
#define BUFFER_SIZE1              (countof(Tx1_Buffer))
#define BUFFER_SIZE2              (countof(Rx2_Buffer))
#define countof(a) (sizeof(a) / sizeof(*(a)))

uint8_t flag;											//��ͬ�İ����в�ͬ�ı�־λֵ
uint8_t flag1 = 0;								//�жϱ�־λ��ÿ�ΰ�������һ���жϣ�����ʼ��ȡ8������ܵ�ֵ
uint8_t Rx2_Buffer[8]={0};
uint8_t Tx1_Buffer[8]={0};
uint8_t Rx1_Buffer[1]={0};
/* USER CODE END PV */
uint8_t pwd_hash[5] = {4,1,3,2,9};//��������Ĺ�ϣֵ�����洢�������룬��ֹ����й¶
uint8_t pwd_tmp[20];							//�������뻺��������ǰ5�ֽ���ʵ�����ã������ֽڷ�ֹ���
int indx = 0;											//�������뻺����ָ��
/* verify pwd*/
void verify_pwd_choose(uint8_t choice);
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void swtich_key(void);
void switch_flag(void);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */
	printf("\n\r");
	printf("\n\r-------------------------------------------------\r\n");
	printf("\n\r ����ƥ����� \r\n");	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	uint8_t verifyChoice = 0;				//����ѡ��ʹ����һ��verify_pwd����
	
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
		if(flag1 == 1)
		{
			flag1 = 0;
			I2C_ZLG7290_Read(&hi2c1,0x71,0x01,Rx1_Buffer,1);//����ֵ
			printf("\n\r������ֵ = %#x\r\n",Rx1_Buffer[0]);	//�򴮿ڷ��ͼ�ֵ
			swtich_key();																		//ɨ���ֵ��д��־λ
			verify_pwd_choose(verifyChoice);								//����ƥ��
			I2C_ZLG7290_Read(&hi2c1,0x71,0x10,Rx2_Buffer,8);//��8λ�����
			switch_flag();																	//ɨ�赽��Ӧ�İ��������������д����ֵ	
		}			
		verifyChoice = ~verifyChoice;
  }
  /* USER CODE END 3 */

}
/*
����ƥ�䣺
�û�ÿ������ֻ��ǰ5������ֵ�ᱻת��Ϊ��Ӧ���֣��������������뻺����pwd_tmp��
���û�����#��ʱ�Ż���п���ƥ�䡣
Ϊ�˷�ֹ����й¶���ڴ��д洢��������Ĺ�ϣֵ��
�����û�������й�ϣ��������������Ĺ�ϣֵ���бȽϡ�
*/
void verify_pwd1(void);
void verify_pwd2(void);

void verify_pwd_choose(uint8_t choice)
{
	if (!choice) {
		verify_pwd1();
	}
	else {
		verify_pwd2();
	}
	return;
}

/* ���������������������ͬ */
void verify_pwd1(void) {
	int i;
	if (Rx1_Buffer[0] == 0x2) {								//�û�����#������������ƥ��
		 indx = 0; 															//����������뻺������Ϊ��һ���û�����������׼��
		 printf("\n\r begin match \r\n");
		 printf("\n\r %#x %#x %#x %#x %#x \r\n", pwd_tmp[0], pwd_tmp[1], pwd_tmp[2], pwd_tmp[3], pwd_tmp[4]);
		 for (i = 0; i < 5; i++) {
				if (pwd_tmp[i]+1 != pwd_hash[i]) {	//���û���������ϣ������������Ĺ�ϣֵ���бȽ�
						printf ("\n\r pwd match error \r\n");
					return;
				} 
			}
		 printf("\n\r pwd match success \r\n");
		 return;
	}
	if (indx <= 5) {				//�û�ÿ�����룬ֻ��ǰ5������ֵ��Ч���������벻�������뻺����
	  pwd_tmp[indx] = flag;	//��switch_flag�����У�����ֵ��ת��Ϊ��Ӧ���֣���������ȫ�ֱ���flag��
	}
	indx = indx + 1;
}

void verify_pwd2(void)
{
	uint8_t i = 0;
	if (flag == 14) {
		printf("\n\r begin match \r\n");
		if (indx != 5) {
			printf("\n\r pwd match failed \r\n");
			return;
		}
		indx = 0;
		while (i < 5) {
			printf("%x ", pwd_tmp[i]);
			pwd_tmp[i]++;
			if (pwd_tmp[i] != pwd_hash[i]) {
				printf("\n\r pwd match failed \r\n");
				return;
			}
			i++;
		}
		printf("\n\r pwd match success \r\n");
		return;
	}
}


/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  __PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */
void swtich_key(void)
{
	switch(Rx1_Buffer[0])
	{
        case 0x1C:
					flag = 1;					
          break;
        case 0x1B:	
					flag = 2;
          break;
        case 0x1A:	
					flag = 3;
          break;
        case 0x14:
					flag = 4;
          break;   
				case 0x13:
					flag = 5;
					break;
        case 0x12:
					flag = 6;
          break;
        case 0x0C:
					flag = 7;
          break;
        case 0x0B:
          flag = 8;
          break;
				case 0x0A:
					flag = 9;
					break;
				case 0x03:
					flag = 0;
					break;
				case 0x19:
					flag = 10;
					break;
				case 0x11:
					flag = 11;
					break;
				case 0x09:
					flag = 12;
					break;
				case 0x01:
					flag = 13;
					break;
				case 0x02:
					flag = 14;
					break;
        default:
          break;
			}
}

void switch_flag(void){
	switch(flag){
			case 1:
				Tx1_Buffer[0] = 0x0c;
				if(Rx2_Buffer[0] == 0)
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);
					}
					else
					{									
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS2,Rx2_Buffer,BUFFER_SIZE2);
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);					
					}
				break;
			case 2:
				Tx1_Buffer[0] = 0xDA;
				if(Rx2_Buffer[0] == 0)
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);
					}
					else
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS2,Rx2_Buffer,BUFFER_SIZE2);
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);		
					}
				break;
			case 3:
				Tx1_Buffer[0] = 0xF2;
				if(Rx2_Buffer[0] == 0)
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);
					}
					else
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS2,Rx2_Buffer,BUFFER_SIZE2);						
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);		
					}
				break;
			case 4:
				Tx1_Buffer[0] = 0x66;
				if(Rx2_Buffer[0] == 0)
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);
					}
					else
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS2,Rx2_Buffer,BUFFER_SIZE2);						
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);					
					}
				break;
			case 5:
				Tx1_Buffer[0] = 0xB6;
				if(Rx2_Buffer[0] == 0)
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);
					}
					else
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS2,Rx2_Buffer,BUFFER_SIZE2);
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);						
					}
				break;
			case 6:
				Tx1_Buffer[0] = 0xBE;
				if(Rx2_Buffer[0] == 0)
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);
					}
					else
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS2,Rx2_Buffer,BUFFER_SIZE2);						
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);						
					}
				break;
			case 7:
				Tx1_Buffer[0] = 0xE0;
				if(Rx2_Buffer[0] == 0)
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);
					}
					else
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS2,Rx2_Buffer,BUFFER_SIZE2);
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);						
					}
				break;
			case 8:
				Tx1_Buffer[0] = 0xFE;
				if(Rx2_Buffer[0] == 0)
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);
					}
					else
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS2,Rx2_Buffer,BUFFER_SIZE2);					
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);							
					}
				break;
			case 9:
				Tx1_Buffer[0] = 0xE6;
				if(Rx2_Buffer[0] == 0)
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);
					}
					else
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS2,Rx2_Buffer,BUFFER_SIZE2);					
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);					
					}
				break;
			case 10:
				Tx1_Buffer[0] = 0xEE;
				if(Rx2_Buffer[0] == 0)
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);
					}
					else
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS2,Rx2_Buffer,BUFFER_SIZE2);					
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);						
					}
				break;
			case 11:
				Tx1_Buffer[0] = 0x3E;
				if(Rx2_Buffer[0] == 0)
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);
					}
					else
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS2,Rx2_Buffer,BUFFER_SIZE2);							
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);						
					}
				break;
					case 12:
				Tx1_Buffer[0] = 0x9C;
				if(Rx2_Buffer[0] == 0)
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);
					}
					else
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS2,Rx2_Buffer,BUFFER_SIZE2);						
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);								
					}
				break;
					case 13:
				Tx1_Buffer[0] = 0x7A;
				if(Rx2_Buffer[0] == 0)
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);
					}
					else
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS2,Rx2_Buffer,BUFFER_SIZE2);						
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);									
					}
				break;
					case 14:
							Tx1_Buffer[0] = 0x00;
							I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,8);
						break;
					case 0:
				Tx1_Buffer[0] = 0xFC;
				if(Rx2_Buffer[0] == 0)
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);
					}
					else
					{
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS2,Rx2_Buffer,BUFFER_SIZE2);						
						I2C_ZLG7290_Write(&hi2c1,0x70,ZLG_WRITE_ADDRESS1,Tx1_Buffer,1);						
					}
				break;
			default:
				break;
		}
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	flag1 = 1;
}
int fputc(int ch, FILE *f)
{ 
  uint8_t tmp[1]={0};
	tmp[0] = (uint8_t)ch;
	HAL_UART_Transmit(&huart1,tmp,1,10);	
	return ch;
}
/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
