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

uint8_t flag;											//不同的按键有不同的标志位值
uint8_t flag1 = 0;								//中断标志位，每次按键产生一次中断，并开始读取8个数码管的值
uint8_t Rx2_Buffer[8]={0};
uint8_t Tx1_Buffer[8]={0};
uint8_t Rx1_Buffer[1]={0};
/* USER CODE END PV */
uint8_t pwd_hash[5] = {4,1,3,2,9};//内置密码的哈希值，不存储明文密码，防止密码泄露
uint8_t pwd_tmp[20];							//输入密码缓冲区，仅前5字节有实际作用，多余字节防止溢出
int indx = 0;											//输入密码缓冲区指针
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
	printf("\n\r 口令匹配程序 \r\n");	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	uint8_t verifyChoice = 0;				//用于选择使用哪一个verify_pwd函数
	
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
		if(flag1 == 1)
		{
			flag1 = 0;
			I2C_ZLG7290_Read(&hi2c1,0x71,0x01,Rx1_Buffer,1);//读键值
			printf("\n\r按键键值 = %#x\r\n",Rx1_Buffer[0]);	//向串口发送键值
			swtich_key();																		//扫描键值，写标志位
			verify_pwd_choose(verifyChoice);								//口令匹配
			I2C_ZLG7290_Read(&hi2c1,0x71,0x10,Rx2_Buffer,8);//读8位数码管
			switch_flag();																	//扫描到相应的按键并且向数码管写进数值	
		}			
		verifyChoice = ~verifyChoice;
  }
  /* USER CODE END 3 */

}
/*
口令匹配：
用户每次输入只有前5个按键值会被转换为相应数字，并放入输入密码缓冲区pwd_tmp。
当用户按下#键时才会进行口令匹配。
为了防止密码泄露，内存中存储的是密码的哈希值。
程序将用户密码进行哈希后，再与内置密码的哈希值进行比较。
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

/* 两个程序冗余项，程序功能相同 */
void verify_pwd1(void) {
	int i;
	if (Rx1_Buffer[0] == 0x2) {								//用户按下#键，进行密码匹配
		 indx = 0; 															//清空输入密码缓冲区，为下一次用户输入密码做准备
		 printf("\n\r begin match \r\n");
		 printf("\n\r %#x %#x %#x %#x %#x \r\n", pwd_tmp[0], pwd_tmp[1], pwd_tmp[2], pwd_tmp[3], pwd_tmp[4]);
		 for (i = 0; i < 5; i++) {
				if (pwd_tmp[i]+1 != pwd_hash[i]) {	//将用户输入做哈希后与内置密码的哈希值进行比较
						printf ("\n\r pwd match error \r\n");
					return;
				} 
			}
		 printf("\n\r pwd match success \r\n");
		 return;
	}
	if (indx <= 5) {				//用户每次输入，只有前5个按键值有效，多余输入不放入密码缓冲区
	  pwd_tmp[indx] = flag;	//在switch_flag函数中，按键值被转换为相应数字，并放入了全局变量flag中
	}
	indx = indx + 1;
}

void verify_pwd2(void)
{
	uint8_t i = 0;
	if (flag == 14) {		//按键#
		printf("\n\r begin match \r\n");
		if (indx != 5) {	//长度匹配
			printf("\n\r pwd match failed \r\n");
			return;
		}
		indx = 0;					//清空密码缓冲区
		while (i < 5) {		//逐位密码进行匹配
			printf("%x ", pwd_tmp[i]);
			pwd_tmp[i]++;		//密码哈希
			if (pwd_tmp[i] != pwd_hash[i]) { //密码匹配
				printf("\n\r pwd match failed \r\n");
				return;
			}
			i++;
		}
		printf("\n\r pwd match success \r\n");
		return;
	}
	else if (indx <= 5) {//当输入不为#键且小于5位时，将其放入密码输入缓冲区
		pwd_tmp[indx] = flag;
		indx++;
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
