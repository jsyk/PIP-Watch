###############################################################################
TRGT = arm-none-eabi-
CC      = $(TRGT)gcc
LD      = $(TRGT)gcc
AR      = $(TRGT)ar
AS      = $(TRGT)gcc
CP      = $(TRGT)objcopy
OD		= $(TRGT)objdump
###############################################################################
# List all default C defines here
DDEFS = -DSTM32F10X_MD_VL -DUSE_STDPERIPH_DRIVER

# List all default ASM defines here
DADEFS = 

# List all default directories to look for include files here
DINCDIR = 

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS = 
###############################################################################
NAME   		= stm32test
CPU		= cortex-m3
PRJDIR		= prj
###############################################################################
SRC = ./src/main.c \
	./src/epd_imgs.c \
	./src/gpio.c \
	./src/misc.c \
	./src/system_stm32f10x.c \
	./src/stm32f10x_gpio.c \
	./src/stm32f10x_rcc.c \
	./src/stm32f10x_spi.c \
	./src/stm32f10x_usart.c

	# ./src/epd.c \
	# ./src/epd_ac.c \
	# ./src/stm32f10x_adc.c \
	# ./src/stm32f10x_bkp.c \
	# ./src/stm32f10x_can.c \
	# ./src/stm32f10x_cec.c \
	# ./src/stm32f10x_crc.c \
	# ./src/stm32f10x_dac.c \
	# ./src/stm32f10x_dbgmcu.c \
	# ./src/stm32f10x_dma.c \
	# ./src/stm32f10x_exti.c \
	# ./src/stm32f10x_flash.c \
	# ./src/stm32f10x_fsmc.c \
	# ./src/stm32f10x_i2c.c \
	# ./src/stm32f10x_iwdg.c \
	# ./src/stm32f10x_pwr.c \
	# ./src/stm32f10x_rtc.c \
	# ./src/stm32f10x_sdio.c \
	# ./src/stm32f10x_tim.c \
	# ./src/stm32f10x_wwdg.c

###############################################################################
ASRC = ./src/startup_stm32f10x_md_vl.s
###############################################################################
# List all user directories here
UINCDIR = ./inc/
# List the user directory to look for the libraries here
ULIBDIR =
# List all user libraries here
ULIBS = 
###############################################################################
INCDIR  = $(patsubst %,-I%,$(DINCDIR) $(UINCDIR))
LIBDIR  = $(patsubst %,-L%,$(DLIBDIR) $(ULIBDIR))

ADEFS   = $(DADEFS) $(UADEFS)
DEFS    = $(DDEFS) $(UDEFS)
LIBS    = $(DLIBS) $(ULIBS)

IMM0 = $(ASRC:.s=.o)
IMM1 = $(IMM0:.S=.O)
IMM2 = $(SRC:.c=.o)
IMM3 = $(IMM2:.C=.O)
OBJ  = $(IMM1) $(IMM3)
#OBJ  = $(IMM1) $(IMM3)
IMM2SPI = $(SRCSPI:.c=.o)
IMM3SPI = $(IMM2SPI:.C=.O)
OBJSPI  = $(IMM1) $(IMM3SPI)

IMM2UC4 = $(SRCUC4:.c=.o)
IMM3UC4 = $(IMM2UC4:.C=.O)
OBJUC4  = $(IMM1) $(IMM3UC4)

CFLAGS  = -c -O0 -g3 -std=c99 -Wall -mcpu=$(CPU) -mthumb -mtune=$(CPU) -ffunction-sections $(INCDIR) $(LIBDIR)  $(DEFS)
#-Wc++-compat
ASFLAGS = -c -mcpu=$(CPU) -mthumb -g3 -x assembler-with-cpp $(ADEFS)
LFLAGS  = -nostartfiles -Wl,--gc-sections
CPFLAGS = -Obinary
ODFLAGS	= -S
# -D

CFLAGS += -mno-long-calls

# all:$(NAME)_ram.lst $(NAME)_ram.bin $(NAME)_flash.lst $(NAME)_flash.bin

all:$(NAME)_flash.lst $(NAME)_flash.bin

%o: %c
	$(CC) -c $(CFLAGS) -I . $(INCDIR) $< -o $@

%O: %C
	$(CC) -c $(CFLAGS) -I . $(INCDIR) $< -o $@

%o: %s
	$(AS) -c $(ASFLAGS) -I . $(INCDIR) $< -o $@

%O: %S
	$(AS) -c $(ASFLAGS) -I . $(INCDIR) $< -o $@

$(NAME)_flash.lst: $(NAME)_flash.elf
	$(OD) $(ODFLAGS) $< > $@

$(NAME)_ram.lst: $(NAME)_ram.elf
	$(OD) $(ODFLAGS) $< > $@

$(NAME)_flash.bin: $(NAME)_flash.elf
	$(CP) $(CPFLAGS) $< $@

$(NAME)_ram.bin: $(NAME)_ram.elf
	$(CP) $(CPFLAGS) $< $@

$(NAME)_flash.elf: $(OBJ) 
	$(LD) $(DEBUG_GDB) $(LFLAGS) -T $(PRJDIR)/stm32_flash.ld -o $(NAME)_flash.elf $(OBJ) $(LIBS)
	$(TRGT)size $@

# $(NAME)_ram.elf:	$(OBJ) 
# 	$(LD) $(DEBUG_GDB) $(LFLAGS) -T $(PRJDIR)\linker_ram.ld -o $(NAME)_ram.elf $(OBJ) $(LIBS)
# 	$(TRGT)size $@

# spitest_ram.elf:	$(OBJSPI) 
# 	$(LD) $(DEBUG_GDB) $(LFLAGS) -T $(PRJDIR)\linker_ram.ld -o $@ $(OBJSPI) $(LIBS)
# 	$(TRGT)size $@


clean:
	rm -f $(OBJ) $(OBJUC4)
	rm -f $(NAME)_flash.elf $(NAME)_flash.lst  $(NAME)_flash.bin
