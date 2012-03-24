#include "Audio.h"
#include "RCC.h"
#include "GPIO.h"
#include "stm32f4xx.h"

static void WriteRegister(uint8_t address,uint8_t value);

void InitializeAudio()
{
	// Turn on peripherals.
	EnableAHB1PeripheralClock(RCC_AHB1ENR_GPIOAEN|RCC_AHB1ENR_GPIOBEN|RCC_AHB1ENR_GPIOCEN|RCC_AHB1ENR_GPIODEN);
	EnableAPB1PeripheralClock(RCC_APB1ENR_I2C1EN);
	//EnableAPB2PeripheralClock(RCC_APB2ENR_TIM8EN|RCC_APB2ENR_TIM9EN|RCC_APB2ENR_SYSCFGEN);

	// Configure reset pin.
	SetGPIOOutputMode(GPIOD,1<<4);
	SetGPIOPushPullOutput(GPIOD,1<<4);
	SetGPIOSpeed50MHz(GPIOD,1<<4);
	SetGPIONoPullResistor(GPIOD,1<<4);

	// Configure I2C SCL and SDA pins.
	SetGPIOAlternateFunctionMode(GPIOB,(1<<6)|(1<<9));
	SetGPIOOpenDrainOutput(GPIOB,(1<<6)|(1<<9));
	SetGPIOSpeed50MHz(GPIOB,(1<<6)|(1<<9));
	SetGPIONoPullResistor(GPIOB,(1<<6)|(1<<9));
	SelectAlternateFunctionForGPIOPin(GPIOB,6,4);
	SelectAlternateFunctionForGPIOPin(GPIOB,9,4);

	// Configure I2S SCK and SD pins.
	SetGPIOAlternateFunctionMode(GPIOC,(1<<10)|(1<<12));
	SetGPIOPushPullOutput(GPIOC,(1<<10)|(1<<12));
	SetGPIOSpeed50MHz(GPIOC,(1<<10)|(1<<12));
	SetGPIONoPullResistor(GPIOC,(1<<10)|(1<<12));
	SelectAlternateFunctionForGPIOPin(GPIOC,10,6);
	SelectAlternateFunctionForGPIOPin(GPIOC,12,6);

	// Configure I2S WS pin.
	SetGPIOAlternateFunctionMode(GPIOA,1<<4);
	SetGPIOPushPullOutput(GPIOA,1<<4);
	SetGPIOSpeed50MHz(GPIOA,1<<4);
	SetGPIONoPullResistor(GPIOA,1<<4);
	SelectAlternateFunctionForGPIOPin(GPIOA,4,6);

	// Configure I2S MCK pin. TODO: Optional?
	SetGPIOAlternateFunctionMode(GPIOC,1<<7);
	SetGPIOPushPullOutput(GPIOC,1<<7);
	SetGPIOSpeed50MHz(GPIOC,1<<7);
	SetGPIONoPullResistor(GPIOC,1<<7);
	SelectAlternateFunctionForGPIOPin(GPIOC,7,6);
  
	// Reset the codec.
	GPIOD->BSRRH=1<<4;
	//Delay(0x4FFF); 
	for(volatile int i=0;i<0x4fff;i++) __asm__ volatile("nop");
	GPIOD->BSRRL=1<<4;

	// Reset I2C.
	SetAPB1PeripheralReset(RCC_APB1RSTR_I2C1RST);
	ClearAPB1PeripheralReset(RCC_APB1RSTR_I2C1RST);

	// Enable the I2C peripheral.
	I2C1->CR1|=I2C_CR1_PE;

	const uint32_t i2c_speed=1000000;
	uint32_t pclk1=PCLK1Frequency();

	// TODO: Remove useless register read.
	uint16_t cr2=I2C1->CR2;
	cr2&=~I2C_CR2_FREQ; // Clear frequency bits.
	cr2|=pclk1/i2c_speed; // Set frequency bits depending on pclk1 value.
	I2C1->CR2=cr2;

	I2C1->CR1&=~I2C_CR1_PE; // Disable I2C to configure TRISE.

	// Configure speed in standard mode.
	int ccrspeed=pclk1/(i2c_speed*2);
	if(ccrspeed<4) ccrspeed=4;
    I2C1->TRISE=pclk1/i2c_speed+1; // Set Maximum Rise Time for standard mode.
	I2C1->CCR=ccrspeed;

	I2C1->CR1|=I2C_CR1_PE; // Enable I2C again.

	// TODO: Remove useless register read.
	uint16_t cr1=I2C1->CR1;
	cr1&=~(I2C_CR1_SMBUS|I2C_CR1_SMBTYPE|I2C_CR1_STOP);
	cr1|=I2C_CR1_ACK;
	I2C1->CR1=cr1;

	I2C1->OAR1=0x4000|0x33;

	WriteRegister(0x02,0x01); // Keep codec powered off.
	WriteRegister(0x04,0xaf); // SPK always off and HP always on.
	//OutputDev = 0xaf;

	WriteRegister(0x05,0x81); // Clock configuration: Auto detection.
	WriteRegister(0x06,0x04); // Set slave mode and Philips audio standard.
	// #define I2S_STANDARD                   I2S_Standard_Phillips         

	SetAudioVolume(128);

	// Power on the codec.
	WriteRegister(0x02,0x9e);  

	// Configure codec for fast shutdown.
	WriteRegister(0x0a,0x00); // Disable the analog soft ramp.
	WriteRegister(0x0e,0x04); // Disable the digital soft ramp.

	WriteRegister(0x27,0x00); // Disable the limiter attack level.
	WriteRegister(0x1f,0x0f); // Adjust bass and treble levels.

	WriteRegister(0x1a,0x0a); // Adjust PCM volume level.
	WriteRegister(0x1b,0x0a);

	/* Configure the I2S peripheral */
	//Codec_AudioInterface_Init(AudioFreq);  
}

void AudioOn()
{
}

void AudioOff()
{
}

void SetAudioVolume(int volume)
{
	WriteRegister(0x20,(volume+0x19)&0xff);
	WriteRegister(0x21,(volume+0x19)&0xff);
}

int SetAudioFormat(int samplerate,int bitspersample,int channels)
{
}

void PlayAudioWithCallback(AudioCallbackFunction *callback,void *context)
{
}

void StopAudio()
{
}

void ProvideAudioBuffer(void *samples,int numsamples)
{
}

bool ProvideAudioBufferWithoutBlocking(void *samples,int numsamples)
{
}




void SetAudioOutputThroughDAC()
{
	// DAC channel 1 & 2 (DAC_OUT1 = PA.4)
	SetGPIOAnalogMode(GPIOA,1<<4);
	SetGPIOPushPullOutput(GPIOA,1<<4);
	SetGPIOSpeed50MHz(GPIOA,1<<4);
	SetGPIONoPullResistor(GPIOA,1<<4);

	// Enable passthrough on AIN1A and AIN1B.
	WriteRegister(0x08,0x01);
	WriteRegister(0x09,0x01);

	// Route the analog input to the HP line.
	WriteRegister(0x0e,0xc0);
    
	// Set the passthough volume
	WriteRegister(0x14,0x00);
	WriteRegister(0x15,0x00);

	EnableAPB1PeripheralClock(RCC_APB1ENR_DACEN);

	DAC->CR=DAC_CR_EN1;
}

void MakeDACNoise()
{
	DAC->CR=DAC_CR_WAVE1_0|DAC_CR_EN1; // Noise.
//	DAC->CR=DAC_CR_WAVE1_1|DAC_CR_EN1; // Triangle.
}

void SetAudioDACValue(int16_t sample)
{
	DAC->DHR12R1=sample>>4; // TODO: Check if the right-aligned register is useful.
}



static void WriteRegister(uint8_t address,uint8_t value)
{
	while(I2C1->SR2&I2C_SR2_BUSY);

	I2C1->CR1|=I2C_CR1_START; // Start the config sequence.

	while((I2C1->SR1&(I2C_SR1_SB))!=(I2C_SR1_SB) ||
	(I2C1->SR2&(I2C_SR2_MSL|I2C_SR2_BUSY))!=(I2C_SR2_MSL|I2C_SR2_BUSY)); // Wait for master mode

	I2C1->DR=0x94|I2C_OAR1_ADD0;

	while((I2C1->SR1&(I2C_SR1_ADDR|I2C_SR1_TXE))!=(I2C_SR1_ADDR|I2C_SR1_TXE) ||
	(I2C1->SR2&(I2C_SR2_MSL|I2C_SR2_BUSY|I2C_SR2_TRA))!=(I2C_SR2_MSL|I2C_SR2_BUSY|I2C_SR2_TRA)); // Wait for master transmitter mode.

	I2C1->DR=address; // Transmit the first address for write operation.

	while((I2C1->SR1&(I2C_SR1_TXE))!=(I2C_SR1_TXE) ||
	(I2C1->SR2&(I2C_SR2_MSL|I2C_SR2_BUSY|I2C_SR2_TRA))!=(I2C_SR2_MSL|I2C_SR2_BUSY|I2C_SR2_TRA)); // Wait for master byte transmit.

	I2C1->DR=value; // Prepare the register value to be sent.

	while(I2C1->SR1&I2C_SR1_BTF); // Wait for all bytes to send.

	I2C1->CR1|=I2C_CR1_STOP; // End the configuration sequence.
}
