CC = gcc
CFLAGS = -std=c99 -g -O3  -Winline
APP_NAME = test
DEFINES = -DRGB8Pixels -DTESTING -DTEST

AUTODEPENDENCY_CFLAGS=-MMD -MF$(@:.o=.d) -MT$@
COMPILE = $(CC) $(CFLAGS) $(DEFINES) $(AUTODEPENDENCY_CFLAGS)

LIBS = -lm -lglut -lGL -lrt

APP_OBJ = main.o bezier.o font.o Random.o bezier/font_geo.o\
		  warpfunc.o  Starfield.o\
		  JupiterCyborg.o  Jupiter3.o bezier/font_led.o\
		  	Utils.o \
			Graphics/Bitmap.o \
			Graphics/DrawingBitmaps.o \
			Graphics/DrawingLines.o \
			Graphics/DrawingPixels.o \
			Graphics/DrawingRectangles.o \
			Graphics/DrawingRLEBitmaps.o \
			Graphics/DrawingStraightLines.o \
			VectorLibrary/VectorFixed.o \
			VectorLibrary/MatrixFixed.o 
FONT_OBJ = bezier/font_enri.c bezier/revision_logo.c bezier/rocket.c

APP_OBJ += $(FONT_OBJ:%.c=%.o)


all: app 
	
app: $(APP_OBJ) $(FONT_HDR) 
	$(COMPILE) $(APP_OBJ) $(LIBS) -o $(APP_NAME)

.c.o:
	$(COMPILE) -c $< -o $@

$(FONT_OBJ): bezier/fontToBezier.py
	cd bezier; python fontToBezier.py

test: all
	-./$(APP_NAME)

gdb: all
	-gdb ./$(APP_NAME)

setup:
	-./setup.sh 2>/dev/null

profile: all
	valgrind --tool=callgrind --dump-instr=yes ./$(APP_NAME) benchmark

benchmark: profile

clean:
	rm -f $(APP_OBJ) $(APP_OBJ:.o=.d) $(APP_NAME)

distclean: clean
	find -type l | xargs rm  # removing links

-include $(APP_OBJ:.o=.d)
