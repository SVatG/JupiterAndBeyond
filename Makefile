NAME = Test

CC = arm-eabi-gcc
LD = arm-eabi-gcc
OBJCOPY = arm-eabi-objcopy

DEFINES =	-DUSE_STDPERIPH_DRIVER \
			-DSTM32F4XX

C_OPTS =	-std=c99 \
			-mthumb \
			-mcpu=cortex-m4 \
			-I. \
			-ILibraries/CMSIS/Include \
			-ILibraries/STM32F4xx_StdPeriph_Driver/inc \
			-g \
			-Os

LIBS =	

SOURCE_DIR = .
BUILD_DIR = Build

#			Utilities/STM32F4-Discovery/stm32f4_discovery.c \
#			Utilities/STM32F4-Discovery/stm32f4_discovery_lis302dl.c \
#			Utilities/STM32F4-Discovery/stm32f4_discovery_audio_codec.c

C_FILES =	Accelerometer.c \
			Button.c \
			LED.c \
			Main.c \
			Interrupts.c \
			System.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_des.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dac.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_sha1.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dcmi.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_iwdg.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_crc.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rng.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_adc.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/misc.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_pwr.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_tdes.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rtc.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_wwdg.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fsmc.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dbgmcu.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_aes.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sdio.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_md5.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_can.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c \
			Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.c \
			
S_FILES = TerribleStartup.S

OBJS = $(C_FILES:%.c=$(BUILD_DIR)/%.o) $(S_FILES:%.S=$(BUILD_DIR)/%.o)

ALL_CFLAGS = $(C_OPTS) $(DEFINES) $(CFLAGS)
ALL_LDFLAGS = $(LD_FLAGS) -mthumb -nostartfiles -Wl,-T,Linker.ld,--gc-sections

AUTODEPENDENCY_CFLAGS=-MMD -MF$(@:.o=.d) -MT$@




all: $(NAME).bin

upload: $(NAME).bin
	openocd -f interface/stlink-v2.cfg -f target/stm32f4x_stlink.cfg \
	-c init -c "reset halt" -c "stm32f2x mass_erase 0" \
	-c "flash write_bank 0 $(NAME).bin 0" \
	-c "reset run" -c shutdown

debug:
	arm-eabi-gdb $(NAME).elf \
	--eval-command="target remote | openocd -f interface/stlink-v2.cfg -f target/stm32f4x_stlink.cfg -c 'gdb_port pipe'"

clean:
	rm -rf $(BUILD_DIR) $(NAME).elf $(NAME).bin

$(NAME).bin: $(NAME).elf
	$(OBJCOPY) -O binary $(NAME).elf $(NAME).bin

$(NAME).elf: $(OBJS)
	$(LD) $(ALL_LDFLAGS) -o $@ $^ $(LIBS)

.SUFFIXES: .o .c .S

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

