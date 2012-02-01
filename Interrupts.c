#include "Interrupts.h"

#include <stdint.h>
#include <stm32f4xx_exti.h>

void NMI_Handler()
{
}

void HardFault_Handler()
{
	for(;;);
}

void MemManage_Handler()
{
	for(;;);
}

void BusFault_Handler()
{
	for(;;);
}

void UsageFault_Handler()
{
	for(;;);
}

void SVC_Handler()
{
}

void DebugMon_Handler()
{
}

void PendSV_Handler()
{
}

volatile uint32_t SysTickCounter=0;

void SysTick_Handler(void)
{  
	SysTickCounter++;
#if 0
  uint8_t *buf;
  uint8_t temp1, temp2 = 0x00;

    Counter ++;
    if (Counter == 10)
    {
      Buffer[0] = 0;
      Buffer[2] = 0;
      /* Disable All TIM4 Capture Compare Channels */
      TIM_CCxCmd(TIM4, TIM_Channel_1, DISABLE);
      TIM_CCxCmd(TIM4, TIM_Channel_2, DISABLE);
      TIM_CCxCmd(TIM4, TIM_Channel_3, DISABLE);
      TIM_CCxCmd(TIM4, TIM_Channel_4, DISABLE);
      
      LIS302DL_Read(Buffer, LIS302DL_OUT_X_ADDR, 6);
      /* Remove the offsets values from data */
      Buffer[0] -= X_Offset;
      Buffer[2] -= Y_Offset;
      /* Update autoreload and capture compare registers value*/
      temp1 = ABS((int8_t)(Buffer[0]));
      temp2 = ABS((int8_t)(Buffer[2]));       
      TempAcceleration = MAX(temp1, temp2);

      if(TempAcceleration != 0)
      { 
        if ((int8_t)Buffer[0] < -2)
        {
          /* Enable TIM4 Capture Compare Channel 4 */
          TIM_CCxCmd(TIM4, TIM_Channel_4, ENABLE);
          /* Sets the TIM4 Capture Compare4 Register value */
          TIM_SetCompare4(TIM4, TIM_CCR/TempAcceleration);
        }
        if ((int8_t)Buffer[0] > 2)
        {
          /* Enable TIM4 Capture Compare Channel 2 */
          TIM_CCxCmd(TIM4, TIM_Channel_2, ENABLE);
          /* Sets the TIM4 Capture Compare2 Register value */
          TIM_SetCompare2(TIM4, TIM_CCR/TempAcceleration);
        }
        if ((int8_t)Buffer[2] > 2)
        { 
          /* Enable TIM4 Capture Compare Channel 1 */
          TIM_CCxCmd(TIM4, TIM_Channel_1, ENABLE);
          /* Sets the TIM4 Capture Compare1 Register value */
          TIM_SetCompare1(TIM4, TIM_CCR/TempAcceleration);
        }      
        if ((int8_t)Buffer[2] < -2)
        { 
          /* Enable TIM4 Capture Compare Channel 3 */
          TIM_CCxCmd(TIM4, TIM_Channel_3, ENABLE);
          /* Sets the TIM4 Capture Compare3 Register value */
          TIM_SetCompare3(TIM4, TIM_CCR/TempAcceleration);
        }
        /* Time base configuration */
        TIM_SetAutoreload(TIM4,  TIM_ARR/TempAcceleration);
      }
      Counter = 0x00;
    }  
  }
#endif  
}

/*void PPP_IRQHandler()
{
}*/

void EXTI0_IRQHandler()
{
	EXTI_ClearITPendingBit(EXTI_Line0);
}

/*void OTG_FS_WKUP_IRQHandler()
{
}

void OTG_FS_IRQHandler()
{
}*/

