/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "pure_pursuit.h"
#include "cubic_spline.h"
#include <vector>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint32_t id;
uint32_t dlc;
uint8_t data[3];

void run(int timnum,int ch1,int ch2,int D_ratio ){
	if(timnum==1){
		if(ch1==1&&ch2==2){
			if(D_ratio>0){
				__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,D_ratio);
				__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,0);
			}
			if(D_ratio<0){
				__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,0);
				__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,D_ratio);
			}
			else{
				__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,0);
				__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,0);
			}
		}
		if(ch1==3&&ch2==4){
			if(D_ratio>0){
				__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,D_ratio);
				__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,0);
			}
			if(D_ratio<0){
				__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,0);
				__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,D_ratio);
			}
			else{
				__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,0);
				__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,0);
			}
		}
	}
	if(timnum==2){
			if(ch1==1&&ch2==2){
				if(D_ratio>0){
					__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,D_ratio);
					__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,0);
				}
				if(D_ratio<0){
					__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,0);
					__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,D_ratio);
				}
				else{
					__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,0);
					__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,0);
				}
			}
			if(ch1==3&&ch2==4){
				if(D_ratio>0){
					__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_3,D_ratio);
					__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_4,0);
				}
				if(D_ratio<0){
					__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_3,0);
					__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_4,D_ratio);
					}
				else{
					__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_3,0);
					__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_4,0);
				}
			}
	}
}
double speed(double xcnt,double ycnt){

}
void HAL_CANRxCallback(CAN_HandleTypeDef *hcan){
	CAN_RxHeaderTypeDef RxHeader;
	uint8_t RxData[3];
	if(HAL_CAN_GetRxMessage(hcan,CAN_RX_FIFO0,&RxHeader,RxData)==HAL_OK){
		id=(RxHeader.IDE==CAN_ID_STD)? RxHeader.StdId:RxHeader.ExtId;
		dlc=RxHeader.DLC;
		data[0]=RxData[0];
		data[1]=RxData[1];
		data[2]=RxData[2];
	}
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  double cnt[3]={0,0,0};
  double x,y,z;
  double speed_x,speed_y,v;
  double mt[3];
  double init_x,init_y,init_yaw,init_v;
  double speed;
  int target_ind;
  int last_ind;
  double lf;
  double delta;
  double circumference_length=0.06*M_PI;
  int allcnt=1000;

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN1_Init();
  MX_CAN2_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  std::vector<double>point_x{0,0,0,0.5,1,1,1};
  std::vector<double>point_y{1,2,1,1,2,2,2};
  std::vector<double>tx;
  std::vector<double>ty;
  std::vector<double>rx;
  std::vector<double>ry;
  std::vector<double>dist;


  CubicSpline course_x(point_x);
  CubicSpline course_y(point_y);

  State state(init_x,init_y,init_yaw,init_v);

  for(double i=0.0;i<point_x.size();i+=0.001){
	  rx.push_back(course_x.Calc(i));
	  ry.push_back(course_y.Calc(i));
  }
 //ターゲット地点を0.01mごとにする--------------------------
  for(double i=1;i<rx.size()-1;i++){
	  double d=hypot(rx[i+1]-rx[i],ry[i+1]-ry[i]);
	  dist.push_back(d);
  }
  for(int i=0;i<dist.size();i++){
	  if(dist[i]>0.01||dist[i]<0.009){
		  tx.push_back(rx[i]);
		  ty.push_back(ry[i]);
	  }
  }
//------------------------------------------------------------------------------
  last_ind=tx.size()-1;
  TargetCourse target_course(tx,ty);
  std::tie(target_ind,lf)=target_course.search_target_index(state);

//can 受信　IDフィルター設定---------------------------------------------------
  uint32_t fid=0x750<<21;
  uint32_t fmask=(0x7F0<<21)|0x4;

  CAN_FilterTypeDef filter;
  filter.FilterIdHigh=fid>>16;
  filter.FilterIdLow=fid;
  filter.FilterMaskIdHigh=fmask>>16;
  filter.FilterMaskIdLow=fmask;
  filter.FilterScale=CAN_FILTERSCALE_32BIT;
  filter.FilterFIFOAssignment=CAN_FILTER_FIFO0;
  filter.FilterBank=0;
  filter.FilterMode=CAN_FILTERMODE_IDMASK;
  filter.SlaveStartFilterBank=14;
  filter.FilterActivation=ENABLE;
  HAL_CAN_ConfigFilter(&hcan1,&filter);

  HAL_CAN_Start(&hcan1);
  HAL_CAN_ActivateNotification(&hcan1,CAN_IT_RX_FIFO0_MSG_PENDING);

  //pwm出力開始-----------------------------------------------------------------

  HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_4);
  HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_4);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  cnt[0]/=allcnt;
	  cnt[1]/=allcnt;
	  cnt[2]/=allcnt;

	  cnt[0]*=circumference_length;
	  cnt[1]*=circumference_length;
	  cnt[2]*=circumference_length;

	  x=-cnt[0]+cnt[1]*sqrt(3)/2+cnt[2]*sqrt(3)/2;
	  y=-cnt[1]/2 +cnt[2]/2;
	  z=cnt[0]+cnt[1]+cnt[2];


	  std::tie(target_ind,delta)=pursuit_control(state,target_course,target_ind);
	  state.update(x,y,speed,delta);

	  speed_x=v*cos(delta);
	  speed_y=v*sin(delta);

	  mt[0]= -speed_x;
	  mt[1]= (1/2)*speed_x-(sqrt(3)/2)*speed_y;
	  mt[2]= (1/2)*speed_x+(sqrt(3)/2)*speed_y;


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
