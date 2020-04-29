##############################################################################
BUILD = build
BIN = test
SRC_DIR = .

##############################################################################
.PHONY: all directory clean size

CC = arm-none-eabi-gcc
CCPP = arm-none-eabi-g++
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE = arm-none-eabi-size

CPPFLAGS = -fno-rtti
CCFLAGS = -std=gnu99
CFLAGS += -W -Wall -Wno-unused-parameter -Os -g2
CFLAGS += -fno-diagnostics-show-caret
CFLAGS += -fdata-sections -ffunction-sections -fno-exceptions
CFLAGS += -funsigned-char -funsigned-bitfields
CFLAGS += -mcpu=cortex-m0plus -mthumb #-mno-unaligned-access
CFLAGS += -MD -MP -MT $(BUILD)/$(*F).o -MF $(BUILD)/$(@F).d

LDFLAGS += -mcpu=cortex-m0plus -mthumb
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,--script=../linker/samd10d14.ld
LDFLAGS += --specs=nano.specs

INCLUDES += \
  -I../include \
  -I..

SRCS += $(wildcard $(SRC_DIR)/*.c)
SRCS += $(wildcard $(SRC_DIR)/*.cpp)
#SRCS += ../startup_samd10.c

DEFINES += \
  -D__SAMD10D14AM__ \
  -DDONT_USE_CMSIS_INIT \
  -DF_CPU=48000000 

CFLAGS += $(INCLUDES) $(DEFINES)

OBJS = $(addprefix $(BUILD)/, $(addsuffix .o, $(basename $(notdir $(SRCS)))))

all: directory $(SRCS) $(BUILD)/$(BIN).elf $(BUILD)/$(BIN).hex $(BUILD)/$(BIN).bin $(BUILD)/$(BIN).lss size

$(BUILD)/$(BIN).elf: $(OBJS)
	@echo LD $@
	@$(CCPP) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

$(BUILD)/$(BIN).hex: $(BUILD)/$(BIN).elf
	@echo OBJCOPY $@
	@$(OBJCOPY) -O ihex $^ $@

$(BUILD)/$(BIN).bin: $(BUILD)/$(BIN).elf
	@echo OBJCOPY $@
	@$(OBJCOPY) -O binary $^ $@

$(BUILD)/$(BIN).lss: $(BUILD)/$(BIN).elf
	@echo OBJDUMP $@
	@$(OBJDUMP) -x -S $^ > $@

$(BUILD)/%.o: $(SRC_DIR)/%.c
	@echo Building $<
	@$(CC) $(CFLAGS) $(CCFLAGS) -c $< -o $@

$(BUILD)/%.o: $(SRC_DIR)/%.cpp
	@echo Building $<
	@$(CCPP) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD)/%.o: $(SRC_DIR)/%.s
	@echo Building $<
	@$(CC) $(CFLAGS) $(CCFLAGS) -c $< -o $@

directory:
	@gmkdir -p $(BUILD)

size: $(BUILD)/$(BIN).elf
	@echo size:
	@$(SIZE) -t $^

clean:
	@echo clean
	@-rm -rf $(BUILD)

-include $(wildcard $(BUILD)/*.d)
