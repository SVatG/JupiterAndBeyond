#include "Accelerometer.h"

__IO uint32_t  LIS302DLTimeout = LIS302DL_FLAG_TIMEOUT;   

#define READWRITE_CMD 0x80 
#define MULTIPLEBYTE_CMD 0x40
#define DUMMY_BYTE 0x00

#define LIS302DL_SPI                       SPI1
#define LIS302DL_SPI_CLK                   RCC_APB2Periph_SPI1

#define LIS302DL_SPI_SCK_PIN               GPIO_Pin_5                  /* PA.05 */
#define LIS302DL_SPI_SCK_GPIO_PORT         GPIOA                       /* GPIOA */
#define LIS302DL_SPI_SCK_GPIO_CLK          RCC_AHB1Periph_GPIOA
#define LIS302DL_SPI_SCK_SOURCE            GPIO_PinSource5
#define LIS302DL_SPI_SCK_AF                GPIO_AF_SPI1

#define LIS302DL_SPI_MISO_PIN              GPIO_Pin_6                  /* PA.6 */
#define LIS302DL_SPI_MISO_GPIO_PORT        GPIOA                       /* GPIOA */
#define LIS302DL_SPI_MISO_GPIO_CLK         RCC_AHB1Periph_GPIOA
#define LIS302DL_SPI_MISO_SOURCE           GPIO_PinSource6
#define LIS302DL_SPI_MISO_AF               GPIO_AF_SPI1

#define LIS302DL_SPI_MOSI_PIN              GPIO_Pin_7                  /* PA.7 */
#define LIS302DL_SPI_MOSI_GPIO_PORT        GPIOA                       /* GPIOA */
#define LIS302DL_SPI_MOSI_GPIO_CLK         RCC_AHB1Periph_GPIOA
#define LIS302DL_SPI_MOSI_SOURCE           GPIO_PinSource7
#define LIS302DL_SPI_MOSI_AF               GPIO_AF_SPI1

#define LIS302DL_SPI_CS_PIN                GPIO_Pin_3                  /* PE.03 */
#define LIS302DL_SPI_CS_GPIO_PORT          GPIOE                       /* GPIOE */
#define LIS302DL_SPI_CS_GPIO_CLK           RCC_AHB1Periph_GPIOE

#define LIS302DL_SPI_INT1_PIN              GPIO_Pin_0                  /* PE.00 */
#define LIS302DL_SPI_INT1_GPIO_PORT        GPIOE                       /* GPIOE */
#define LIS302DL_SPI_INT1_GPIO_CLK         RCC_AHB1Periph_GPIOE
#define LIS302DL_SPI_INT1_EXTI_LINE        EXTI_Line0
#define LIS302DL_SPI_INT1_EXTI_PORT_SOURCE EXTI_PortSourceGPIOE
#define LIS302DL_SPI_INT1_EXTI_PIN_SOURCE  EXTI_PinSource0
#define LIS302DL_SPI_INT1_EXTI_IRQn        EXTI0_IRQn 

#define LIS302DL_SPI_INT2_PIN              GPIO_Pin_1                  /* PE.01 */
#define LIS302DL_SPI_INT2_GPIO_PORT        GPIOE                       /* GPIOE */
#define LIS302DL_SPI_INT2_GPIO_CLK         RCC_AHB1Periph_GPIOE
#define LIS302DL_SPI_INT2_EXTI_LINE        EXTI_Line1
#define LIS302DL_SPI_INT2_EXTI_PORT_SOURCE EXTI_PortSourceGPIOE
#define LIS302DL_SPI_INT2_EXTI_PIN_SOURCE  EXTI_PinSource1
#define LIS302DL_SPI_INT2_EXTI_IRQn        EXTI1_IRQn 

static uint8_t LIS302DL_SendByte(uint8_t byte);
static void LIS302DL_LowLevel_Init(void);

void LIS302DL_Init(LIS302DL_InitTypeDef *LIS302DL_InitStruct)
{
  uint8_t ctrl = 0x00;
  
  /* Configure the low level interface ---------------------------------------*/
  LIS302DL_LowLevel_Init();
  
  /* Configure MEMS: data rate, power mode, full scale, self test and axes */
  ctrl = (uint8_t) (LIS302DL_InitStruct->Output_DataRate | LIS302DL_InitStruct->Power_Mode | \
                    LIS302DL_InitStruct->Full_Scale | LIS302DL_InitStruct->Self_Test | \
                    LIS302DL_InitStruct->Axes_Enable);
  
  /* Write value to MEMS CTRL_REG1 regsister */
  LIS302DL_Write(&ctrl, LIS302DL_CTRL_REG1_ADDR, 1);
}

static void LIS302DL_LowLevel_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

  /* Enable the SPI periph */
  RCC_APB2PeriphClockCmd(LIS302DL_SPI_CLK, ENABLE);

  /* Enable SCK, MOSI and MISO GPIO clocks */
  RCC_AHB1PeriphClockCmd(LIS302DL_SPI_SCK_GPIO_CLK | LIS302DL_SPI_MISO_GPIO_CLK | LIS302DL_SPI_MOSI_GPIO_CLK, ENABLE);

  /* Enable CS  GPIO clock */
  RCC_AHB1PeriphClockCmd(LIS302DL_SPI_CS_GPIO_CLK, ENABLE);
  
  /* Enable INT1 GPIO clock */
  RCC_AHB1PeriphClockCmd(LIS302DL_SPI_INT1_GPIO_CLK, ENABLE);
  
  /* Enable INT2 GPIO clock */
  RCC_AHB1PeriphClockCmd(LIS302DL_SPI_INT2_GPIO_CLK, ENABLE);

  GPIO_PinAFConfig(LIS302DL_SPI_SCK_GPIO_PORT, LIS302DL_SPI_SCK_SOURCE, LIS302DL_SPI_SCK_AF);
  GPIO_PinAFConfig(LIS302DL_SPI_MISO_GPIO_PORT, LIS302DL_SPI_MISO_SOURCE, LIS302DL_SPI_MISO_AF);
  GPIO_PinAFConfig(LIS302DL_SPI_MOSI_GPIO_PORT, LIS302DL_SPI_MOSI_SOURCE, LIS302DL_SPI_MOSI_AF);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  /* SPI SCK pin configuration */
  GPIO_InitStructure.GPIO_Pin = LIS302DL_SPI_SCK_PIN;
  GPIO_Init(LIS302DL_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /* SPI  MOSI pin configuration */
  GPIO_InitStructure.GPIO_Pin =  LIS302DL_SPI_MOSI_PIN;
  GPIO_Init(LIS302DL_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /* SPI MISO pin configuration */
  GPIO_InitStructure.GPIO_Pin = LIS302DL_SPI_MISO_PIN;
  GPIO_Init(LIS302DL_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  /* SPI configuration -------------------------------------------------------*/
  SPI_I2S_DeInit(LIS302DL_SPI);
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_Init(LIS302DL_SPI, &SPI_InitStructure);

  /* Enable SPI1  */
  SPI_Cmd(LIS302DL_SPI, ENABLE);

  /* Configure GPIO PIN for Lis Chip select */
  GPIO_InitStructure.GPIO_Pin = LIS302DL_SPI_CS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(LIS302DL_SPI_CS_GPIO_PORT, &GPIO_InitStructure);

  /* Deselect : Chip Select high */
  GPIO_SetBits(LIS302DL_SPI_CS_GPIO_PORT, LIS302DL_SPI_CS_PIN);
  
  /* Configure GPIO PINs to detect Interrupts */
  GPIO_InitStructure.GPIO_Pin = LIS302DL_SPI_INT1_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(LIS302DL_SPI_INT1_GPIO_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = LIS302DL_SPI_INT2_PIN;
  GPIO_Init(LIS302DL_SPI_INT2_GPIO_PORT, &GPIO_InitStructure);
}

/**
  * @brief  Set LIS302DL Internal High Pass Filter configuration.
  * @param  LIS302DL_Filter_ConfigTypeDef: pointer to a LIS302DL_FilterConfig_TypeDef 
  *         structure that contains the configuration setting for the LIS302DL Filter.
  * @retval None
  */
void LIS302DL_FilterConfig(LIS302DL_FilterConfigTypeDef *LIS302DL_FilterConfigStruct)
{
  uint8_t ctrl = 0x00;
  
  /* Read CTRL_REG2 register */
  LIS302DL_Read(&ctrl, LIS302DL_CTRL_REG2_ADDR, 1);
  
  /* Clear high pass filter cut-off level, interrupt and data selection bits*/
  ctrl &= (uint8_t)~(LIS302DL_FILTEREDDATASELECTION_OUTPUTREGISTER | \
                     LIS302DL_HIGHPASSFILTER_LEVEL_3 | \
                     LIS302DL_HIGHPASSFILTERINTERRUPT_1_2);
  /* Configure MEMS high pass filter cut-off level, interrupt and data selection bits */                     
  ctrl |= (uint8_t)(LIS302DL_FilterConfigStruct->HighPassFilter_Data_Selection | \
                    LIS302DL_FilterConfigStruct->HighPassFilter_CutOff_Frequency | \
                    LIS302DL_FilterConfigStruct->HighPassFilter_Interrupt);
  
  /* Write value to MEMS CTRL_REG2 register */
  LIS302DL_Write(&ctrl, LIS302DL_CTRL_REG2_ADDR, 1);
}

/**
  * @brief Set LIS302DL Interrupt configuration
  * @param  LIS302DL_InterruptConfig_TypeDef: pointer to a LIS302DL_InterruptConfig_TypeDef 
  *         structure that contains the configuration setting for the LIS302DL Interrupt.
  * @retval None
  */
void LIS302DL_InterruptConfig(LIS302DL_InterruptConfigTypeDef *LIS302DL_IntConfigStruct)
{
  uint8_t ctrl = 0x00;
  
  /* Read CLICK_CFG register */
  LIS302DL_Read(&ctrl, LIS302DL_CLICK_CFG_REG_ADDR, 1);
  
  /* Configure latch Interrupt request, click interrupts and double click interrupts */                   
  ctrl = (uint8_t)(LIS302DL_IntConfigStruct->Latch_Request| \
                   LIS302DL_IntConfigStruct->SingleClick_Axes | \
                   LIS302DL_IntConfigStruct->DoubleClick_Axes);
  
  /* Write value to MEMS CLICK_CFG register */
  LIS302DL_Write(&ctrl, LIS302DL_CLICK_CFG_REG_ADDR, 1);
}

/**
  * @brief  Change the lowpower mode for LIS302DL
  * @param  LowPowerMode: new state for the lowpower mode.
  *   This parameter can be one of the following values:
  *     @arg LIS302DL_LOWPOWERMODE_POWERDOWN: Power down mode
  *     @arg LIS302DL_LOWPOWERMODE_ACTIVE: Active mode  
  * @retval None
  */
void LIS302DL_LowpowerCmd(uint8_t LowPowerMode)
{
  uint8_t tmpreg;
  
  /* Read CTRL_REG1 register */
  LIS302DL_Read(&tmpreg, LIS302DL_CTRL_REG1_ADDR, 1);
  
  /* Set new low power mode configuration */
  tmpreg &= (uint8_t)~LIS302DL_LOWPOWERMODE_ACTIVE;
  tmpreg |= LowPowerMode;
  
  /* Write value to MEMS CTRL_REG1 regsister */
  LIS302DL_Write(&tmpreg, LIS302DL_CTRL_REG1_ADDR, 1);
}

/**
  * @brief  Data Rate command 
  * @param  DataRateValue: Data rate value
  *   This parameter can be one of the following values:
  *     @arg LIS302DL_DATARATE_100: 100 Hz output data rate 
  *     @arg LIS302DL_DATARATE_400: 400 Hz output data rate    
  * @retval None
  */
void LIS302DL_DataRateCmd(uint8_t DataRateValue)
{
  uint8_t tmpreg;
  
  /* Read CTRL_REG1 register */
  LIS302DL_Read(&tmpreg, LIS302DL_CTRL_REG1_ADDR, 1);
  
  /* Set new Data rate configuration */
  tmpreg &= (uint8_t)~LIS302DL_DATARATE_400;
  tmpreg |= DataRateValue;
  
  /* Write value to MEMS CTRL_REG1 regsister */
  LIS302DL_Write(&tmpreg, LIS302DL_CTRL_REG1_ADDR, 1);
}

/**
  * @brief  Change the Full Scale of LIS302DL
  * @param  FS_value: new full scale value. 
  *   This parameter can be one of the following values:
  *     @arg LIS302DL_FULLSCALE_2_3: +-2.3g
  *     @arg LIS302DL_FULLSCALE_9_2: +-9.2g   
  * @retval None
  */
void LIS302DL_FullScaleCmd(uint8_t FS_value)
{
  uint8_t tmpreg;
  
  /* Read CTRL_REG1 register */
  LIS302DL_Read(&tmpreg, LIS302DL_CTRL_REG1_ADDR, 1);
  
  /* Set new full scale configuration */
  tmpreg &= (uint8_t)~LIS302DL_FULLSCALE_9_2;
  tmpreg |= FS_value;
  
  /* Write value to MEMS CTRL_REG1 regsister */
  LIS302DL_Write(&tmpreg, LIS302DL_CTRL_REG1_ADDR, 1);
}

/**
  * @brief  Reboot memory content of LIS302DL
  * @param  None
  * @retval None
  */
void LIS302DL_RebootCmd(void)
{
  uint8_t tmpreg;
  /* Read CTRL_REG2 register */
  LIS302DL_Read(&tmpreg, LIS302DL_CTRL_REG2_ADDR, 1);
  
  /* Enable or Disable the reboot memory */
  tmpreg |= LIS302DL_BOOT_REBOOTMEMORY;
  
  /* Write value to MEMS CTRL_REG2 regsister */
  LIS302DL_Write(&tmpreg, LIS302DL_CTRL_REG2_ADDR, 1);
}

void LIS302DL_ReadACC(int32_t* out)
{
  uint8_t buffer[6];
  uint8_t crtl, i = 0x00;
   
  LIS302DL_Read(&crtl, LIS302DL_CTRL_REG1_ADDR, 1);  
  LIS302DL_Read(buffer, LIS302DL_OUT_X_ADDR, 6);
  
  switch(crtl & 0x20) 
    {
    /* FS bit = 0 ==> Sensitivity typical value = 18milligals/digit*/ 
    case 0x00:
      for(i=0; i<0x03; i++)
      {

        *out =(int32_t)(LIS302DL_SENSITIVITY_2_3G *  (int8_t)buffer[2*i]);
        out++;
      }
      break;
    /* FS bit = 1 ==> Sensitivity typical value = 72milligals/digit*/ 
    case 0x20:
      for(i=0; i<0x03; i++)
      {
        *out =(int32_t)(LIS302DL_SENSITIVITY_9_2G * (int8_t)buffer[2*i]);
        out++;
      }         
      break;
    default:
      break;
    }
 }




static uint8_t ReadByte(uint8_t address)
{  
	LowerCS();

	TransferByte(address|READWRITE_CMD);
	uint8_t byte=TransferByte(0);

	RaiseCS();

	return byte;
}

static void ReadBytes(uint8_t *buffer,uint8_t address,uint16_t numbytes)
{  
	if(numbytes>1) address|=READWRITE_CMD|MULTIPLEBYTE_CMD;
	else address|=READWRITE_CMD;

	LowerCS();

	TransferByte(address);
	for(int i=0;i<numbytes;i++) buffer[i]=TransferByte(0);

	RaiseCS();
}

static void WriteByte(uint8_t byte,uint8_t address)
{
	LowerCS();

	TransferByte(address);
	TransferByte(byte);

	RaiseCS();
}

static void WriteBytes(uint8_t *bytes,uint8_t address,int numbytes)
{
	if(numbytes>1) address|=MULTIPLEBYTE_CMD;

	LowerCS();

	TransferByte(address);
	for(int i=0;i<numbytes;i++) TransferByte(bytes[i]);

	RaiseCS();
}

static inline uint8_t LowerCS()
{
	GPIOE->BSRRL=1<<3;
}

static inline uint8_t RaiseCS()
{
	GPIOE->BSRRH=1<<3;
}

static uint8_t TransferByte(uint8_t byte)
{
  /* Loop while DR register in not emplty */
  LIS302DLTimeout = LIS302DL_FLAG_TIMEOUT;
  while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)==RESET)
  {
    if((LIS302DLTimeout--) == 0) return LIS302DL_TIMEOUT_UserCallback();
  }

	SPI_I2S_SendData(SPI1,byte);
  
  /* Wait to receive a Byte */
  LIS302DLTimeout = LIS302DL_FLAG_TIMEOUT;
  while (SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)==RESET)
  {
    if((LIS302DLTimeout--) == 0) return LIS302DL_TIMEOUT_UserCallback();
  }

	return SPI_I2S_ReceiveData(SPI1);
}

#ifdef USE_DEFAULT_TIMEOUT_CALLBACK
uint32_t LIS302DL_TIMEOUT_UserCallback(void)
{
  /* Block communication and all processes */
  while (1)
  {   
  }
}
#endif /* USE_DEFAULT_TIMEOUT_CALLBACK */

