# Makefile for the STM32F103C8T6 lqfp48  imu altimeter vario 

PROJECT = vario

# Project Structure
SRCDIR = .
LIBDIR = ../lib
DRVDIR = ../drv
BINDIR = bin
OBJDIR = obj


# OCDDIR = d:/ARMTC/openocd-0.9.0
OCDDIR = /usr/local/share/openocd

# Project target
CPU = cortex-m3

DRIVERS = $(DRVDIR)/stm32f1xx_hal.c $(DRVDIR)/stm32f1xx_hal_rcc.c $(DRVDIR)/stm32f1xx_hal_rcc_ex.c \
 $(DRVDIR)/stm32f1xx_hal_cortex.c $(DRVDIR)/stm32f1xx_hal_uart.c $(DRVDIR)/stm32f1xx_hal_gpio.c \
 $(DRVDIR)/uart.c $(DRVDIR)/stm32f1xx_hal_adc.c $(DRVDIR)/stm32f1xx_hal_adc_ex.c \
 $(DRVDIR)/stm32f1xx_hal_dma.c  $(DRVDIR)/stm32f1xx_hal_tim.c  $(DRVDIR)/stm32f1xx_hal_tim_ex.c  

LIBRARIES = $(LIBDIR)/dwt.c $(LIBDIR)/kalmanfilter3.c \
 $(LIBDIR)/ms5611.c  $(LIBDIR)/MahonyAHRS.c $(LIBDIR)/piezo.c 

# Sources
SRC = $(wildcard $(SRCDIR)/*.c)  ../system_stm32f1xx.c  $(DRIVERS) $(LIBRARIES) 
ASM =  ../startup_stm32f103xb.s

# Include directories
INCLUDE  = -I. -I.. -I../lib -I../drv -I../CMSIS/Include

# Linker 
LSCRIPT = ../STM32F103X8_FLASH.ld

# C Flags
GCFLAGS  = -Wall -std=gnu99 -fno-common -mthumb -mcpu=$(CPU) -O2 -DSTM32F103xB --specs=nosys.specs --specs=nano.specs
GCFLAGS += $(INCLUDE)
LDFLAGS += -T$(LSCRIPT) -mthumb -mcpu=$(CPU) -lm --specs=nosys.specs --specs=nano.specs
ASFLAGS += -mcpu=$(CPU)

# Flashing
OCDFLAGS = -f $(OCDDIR)/scripts/interface/stlink-v2.cfg \
		   -f $(OCDDIR)/scripts/target/stm32f1x.cfg \
		   -f openocd.cfg

# Tools
CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
AR = arm-none-eabi-ar
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size
OBJDUMP = arm-none-eabi-objdump
OCD = openocd

RM = rm -rf

## Build process

OBJ := $(addprefix $(OBJDIR)/,$(notdir $(SRC:.c=.o)))
OBJ += $(addprefix $(OBJDIR)/,$(notdir $(ASM:.s=.o)))


all:: $(BINDIR)/$(PROJECT).bin

flash: $(BINDIR)/$(PROJECT).bin
	$(OCD) $(OCDFLAGS)

hex:: $(BINDIR)/$(PROJECT).hex
	
$(BINDIR)/$(PROJECT).hex: $(BINDIR)/$(PROJECT).elf
	$(OBJCOPY) -R .stack -O ihex $(BINDIR)/$(PROJECT).elf $(BINDIR)/$(PROJECT).hex

$(BINDIR)/$(PROJECT).bin: $(BINDIR)/$(PROJECT).elf
	$(OBJCOPY) -R .stack -O binary $(BINDIR)/$(PROJECT).elf $(BINDIR)/$(PROJECT).bin

$(BINDIR)/$(PROJECT).elf: $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(OBJ) $(LDFLAGS) -o $(BINDIR)/$(PROJECT).elf


clean:
	$(RM) $(BINDIR)
	$(RM) $(OBJDIR)

# Compilation
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(GCFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(DRVDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(GCFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(LIBDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(GCFLAGS) -c $< -o $@


$(OBJDIR)/%.o: $(SRCDIR)/%.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -o $@ $<

$(OBJDIR)/%.o: ../%.c
	@mkdir -p $(dir $@)
	$(CC) $(GCFLAGS) -c $< -o $@

$(OBJDIR)/%.o: ../%.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -o $@ $<
