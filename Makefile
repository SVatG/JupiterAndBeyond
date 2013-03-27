NAME = Jupiter

CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

DEFINES = -DRGB8Pixels
#DEFINES = -DRGB8Pixels -DEnableOverclocking

C_OPTS =	-std=c99 \
		-mthumb \
		-mcpu=cortex-m4 \
		-ILibraries/CMSIS/Include \
		-g \
		-Werror \
		-O3

LIBS =	-lm

SOURCE_DIR = .
BUILD_DIR = Build

C_FILES = Audio.c \
		BitBin.c \
		Button.c \
		CityText.c \
		Cityscape.c \
		Fields.c \
		HeightMap.c \
		Jupiter1.c \
		Jupiter3.c \
		JupiterCyborg.c \
		JupiterCyborg2.c \
		JupiterCyborg2Texture.c \
		JupiterHands.c \
		LED.c \
		LogoShow.c \
		Main.c \
		Mystery.c \
		PixelParticles.c \
		Pukezoomer.c \
		Random.c \
		Rasterize.c \
		RayArray.c \
		Starfield.c \
		Startup.c \
		System.c \
		TestSong.c \
		TorusTunnel.c \
		Utils.c \
		VGA.c \
		Voxelscape.c \
		Profiling.c \
		bezier.c \
		Credits.c \
		warpfunc.c \
		greetstars.c \
		font.c \
		Metablobs.c \
		Blobs.c \
		Graphics/Bitmap.c \
		Graphics/DrawingBitmaps.c \
		Graphics/DrawingLines.c \
		Graphics/DrawingPixels.c \
		Graphics/DrawingRectangles.c \
		Graphics/DrawingRLEBitmaps.c \
		Graphics/DrawingStraightLines.c \
		VectorLibrary/VectorFixed.c \
		VectorLibrary/MatrixFixed.c \

FONT_FILES = \
		bezier/font_enri.c \
		bezier/font_geo.c \
		bezier/font_led.c \
		bezier/revision_logo.c \
		bezier/rocket.c \

C_FILES += $(FONT_FILES)
			
S_FILES = 

OBJS = $(C_FILES:%.c=$(BUILD_DIR)/%.o) $(S_FILES:%.S=$(BUILD_DIR)/%.o)

ALL_CFLAGS = $(C_OPTS) $(DEFINES) $(CFLAGS)
ALL_LDFLAGS = $(LD_FLAGS) -mthumb -mcpu=cortex-m4 -nostartfiles -Wl,-T,Linker.ld,--gc-sections -specs Terrible.specs

AUTODEPENDENCY_CFLAGS=-MMD -MF$(@:.o=.d) -MT$@




all: $(NAME).bin

upload: $(NAME).bin
	openocd -f interface/stlink-v2.cfg -f target/stm32f4x_stlink.cfg \
	-c init -c "reset halt" -c "stm32f2x mass_erase 0" \
	-c "flash write_bank 0 $(NAME).bin 0" \
	-c "reset run" -c shutdown

$(FONT_FILES): bezier/fontToBezier.py
	cd bezier; python fontToBezier.py

debug:
	arm-eabi-gdb $(NAME).elf \
	--eval-command="target remote | openocd -f interface/stlink-v2.cfg -f target/stm32f4x_stlink.cfg -c 'gdb_port pipe'"

stlink:
	arm-eabi-gdb $(NAME).elf --eval-command="target ext :4242"

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

