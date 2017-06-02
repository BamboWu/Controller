PROJECT_NAME := Controller

###
### Makefile and template
###
export OUTPUT_FILENAME
#默认$(MAKEFILE_LIST)变量即为本Makefile文件名
MAKEFILE_NAME := $(MAKEFILE_LIST)
#本Makefile的路径
MAKEFILE_DIR := $(dir $(MAKEFILE_NAME) )

#固件库驱动文件路径
DRIVER_PATH = ../../STM32Cube_FW_F1_V1.4.0/Drivers
#芯片Startup文件路径
DEVICE_PATH = $(DRIVER_PATH)/CMSIS/Device/ST/STM32F1xx/
#硬件抽象层驱动文件路径
HAL_PATH = $(DRIVER_PATH)/STM32F1xx_HAL_Driver

#gcc安装路径
GNU_INSTALL_ROOT := /home/babypanda/nRF_IDE/gcc-arm-none-eabi-5_4-2016q3
#gcc版本
GNU_VERSION := 5.4.1
#gcc工具前缀
GNU_PREFIX := arm-none-eabi

###
### tools
###
#建文件夹
MK := mkdir  
#移除文件
RM := rm -rf 

#echo suspend
ifeq ("$(VERBOSE)","1") #如果编译的make命令后的VERBOSE变量赋值为1
NO_ECHO := 
else                    #则会详细输出每一步编译动作
NO_ECHO := @  #'@' following the Tab makes the command excuted without print out
endif                   #否则NO_ECHO打头的命令不会有输出

# Toolchain commands    #编译工具的命令
CC              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-gcc'
AS              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-as'
AR              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ar' -r
LD              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ld'
NM              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-nm'
OBJDUMP         := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objdump'
OBJCOPY         := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objcopy'
SIZE            := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-size'

#function for removing duplicates in a list  #这是一个会递归展开的脚本，用于删除字符串中重复出现的word（以空格或引号分隔）
remduplicates = $(strip $(if $1,$(firstword $1) $(call remduplicates,$(filter-out $(firstword $1),$1)))) 
#$(strip string) is used to adjust format of string so that each two words in the string are sperated by only on blackspace. 
#First one plus the rest where the first one is filtered out, expended recursively until empty string is passed as argument $1

###
### source files and including path #源文件与头文件的路径
###
#assembly files (startup code) common to all targets
ASM_SOURCE_FILES  = $(abspath $(DEVICE_PATH)/Source/Templates/gcc/startup_stm32f103xe.s)
#CMSIS提供的汇编文件用于启动芯片

#source common to all targets
C_SOURCE_FILES  = $(abspath ../Src/main.c)          #程序的主文件 
C_SOURCE_FILES += $(abspath ../Src/stm32f1xx_it.c)  #中断处理函数
C_SOURCE_FILES += $(abspath ../Src/stm32f1xx_hal_msp.c)    #HAL底层函数实现
C_SOURCE_FILES += $(abspath $(DEVICE_PATH)/Source/Templates/system_stm32f1xx.c)
#CMSIS提供的源文件用于配置芯片
#sources of HAL (Hardware Abstract Layer) # HAL驱动均为ST公司提供
C_SOURCE_FILES += $(abspath $(HAL_PATH)/Src/stm32f1xx_hal.c)        #HAL
C_SOURCE_FILES += $(abspath $(HAL_PATH)/Src/stm32f1xx_hal_rcc.c)    #时钟管理
C_SOURCE_FILES += $(abspath $(HAL_PATH)/Src/stm32f1xx_hal_cortex.c) #内核管理
C_SOURCE_FILES += $(abspath $(HAL_PATH)/Src/stm32f1xx_hal_gpio.c)   #GPIO管理
C_SOURCE_FILES += $(abspath $(HAL_PATH)/Src/stm32f1xx_hal_tim.c)    #Timer管理
C_SOURCE_FILES += $(abspath $(HAL_PATH)/Src/stm32f1xx_hal_tim_ex.c) #Timer管理额外功能函数
C_SOURCE_FILES += $(abspath $(HAL_PATH)/Src/stm32f1xx_hal_dma.c)    #直接存数器访问
C_SOURCE_FILES += $(abspath $(HAL_PATH)/Src/stm32f1xx_hal_rcc_ex.c) #时钟管理额外功能函数
C_SOURCE_FILES += $(abspath $(HAL_PATH)/Src/stm32f1xx_hal_flash.c)  #flash管理功能函数
C_SOURCE_FILES += $(abspath $(HAL_PATH)/Src/stm32f1xx_hal_flash_ex.c) #flash管理额外功能函数
C_SOURCE_FILES += $(abspath $(HAL_PATH)/Src/stm32f1xx_hal_uart.c)   #串口功能函数
#sources for JLink tool -- RTT (Real-Time Terminal)
C_SOURCE_FILES += $(abspath ../../RTT/RTT/SEGGER_RTT.c)             #RTT调试
C_SOURCE_FILES += $(abspath ../../RTT/RTT/SEGGER_RTT_printf.c)      #RTT调试
#sources for valve control #控制电磁阀的函数源文件
C_SOURCE_FILES += $(abspath ../Src/Valve/valve.c)  #电磁阀控制函数 
#sources for coder control #与编码器交互的函数源文件
C_SOURCE_FILES += $(abspath ../Src/Coder/coder.c)  #电磁阀控制函数 
#sources for command interface #提供命令行交互界面的函数源文件
C_SOURCE_FILES += $(abspath ../Src/HMI/cmd.c)  #命令行交互界面函数 
#sources for HMI #提供人机交互界面的函数源文件
C_SOURCE_FILES += $(abspath ../Src/HMI/hmi.c)  #命令行交互界面函数 
#sources for CRC #提供CRC校验的函数源文件
C_SOURCE_FILES += $(abspath ../Src/HMI/crc.c)  #CRC校验计算函数 

#includes common to all targets
INC_PATHS  = -I$(abspath $(DRIVER_PATH)/CMSIS/Include) #包含CMSIS提供的头文件
INC_PATHS += -I$(abspath $(DEVICE_PATH)/Include)       #包含芯片相关的头文件
INC_PATHS += -I$(abspath ../Inc)
#工程Inc文件夹下的main.h stm32f103xx_it.h stm32f103_hal_conf.h
#includes of HAL (Hardware Abstract Layer)
INC_PATHS += -I$(abspath $(HAL_PATH)/Inc)              #包含HAL驱动文件的头文件
#headers for JLink tool -- RTT (Real-Time Terminal)
INC_PATHS += -I$(abspath ../../RTT/RTT)                #包含RTT工具的头文件

###
### output settings #输出设定
###
OBJECT_DIRECTORY = ../_build
#输出.o文件存储的文件夹
LISTING_DIRECTORY = $(OBJECT_DIRECTORY)
#输出Listing文件存储的文件夹
OUTPUT_BINARY_DIRECTORY = $(OBJECT_DIRECTORY)
#输出二进制文件存储的文件夹

# Sorting removes duplicates                   #汇总输出文件文件夹
BUILD_DIRECTORIES := $(sort $(OBJECT_DIRECTORY) $(OUTPUT_BINARY_DIRECTORY) $(LISTING_DIRECTORY) ) #BUILD_DIRECTORIES are/is the summary of directories used for ouput ordered alphabetically


###
### compiler and linker parameters
###
#flags common to all targets    # CFLAGS为编译C文件时给gcc设定的参数
CFLAGS  = -DSTM32F103xE         # define target processor (used in system_stm32f1xx.c)                          # STM32F103xE是高容量芯片(STM32F103xC也是)配置中要用到的define
CFLAGS += -mcpu=cortex-m3       # specify the target processor core
CFLAGS += -mthumb               # generate core that executes in Thumb states
CFLAGS += -mlittle-endian       # compile code for little endian target
#CFLAGS += -mabi=aapcs --std=gnu99
CFLAGS += -Wall -Werror         # enable all warnings; show error?; 
CFLAGS += -Os                   # Optimize for size
#CFLAGS += -mfloat-abi=soft
# keep every function in separate section. This will allow linker to dump unused functions
#CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
#CFLAGS += -fno-builtin --short-enums
#CFLAGS += -c                    # compile only

# keep every function in separate section. This will allow linker to dump unused functions                       #LDFLAGS 为使用gcc链接器时设定的参数
LDFLAGS += -Xlinker -Map=$(LISTING_DIRECTORY)/$(OUTPUT_FILENAME).map
#LDFLAGS += -mabi=aapcs -L $(TEMPLATE_PATH)
LDFLAGS += -T$(LINKER_SCRIPT)   # use specific linker script
LDFLAGS += -mthumb 
LDFLAGS += -mlittle-endian
LDFLAGS += -mcpu=cortex-m3
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections    # enable garbage collection of unused input sections
# use newlib in nano version
#LDFLAGS += --specs=nano.specs -lc -lnosys

# Assembler flags               #ASMFLAGS为用gcc汇编器时设定的参数
ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -DSTM32F103xE       # STM32F103xE是高容量芯片配置中要用到的define

###
### targets #目标
###
#default target - first one defined #默认清除所有生成文件后，再编译STM32F103xx
default: clean STM32F103xx

#building all targets
all: clean
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e cleanobj
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e STM32F103xx
	# -f specify script file, -C change work directory, -e = --environment-overrides Give variables taken from the environment precedence over variables from makefiles. 

#target for printing all targets
help:
	@echo following targets are available:
	@echo 	STM32F103xx
	@echo 	flash

C_SOURCE_FILE_NAMES = $(notdir $(C_SOURCE_FILES))
C_PATHS = $(call remduplicates, $(dir $(C_SOURCE_FILES) ) )
C_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(C_SOURCE_FILE_NAMES:.c=.o) )

ASM_SOURCE_FILE_NAMES = $(notdir $(ASM_SOURCE_FILES))
ASM_PATHS = $(call remduplicates, $(dir $(ASM_SOURCE_FILES) ))
ASM_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(ASM_SOURCE_FILE_NAMES:.s=.o) )

vpath %.c $(C_PATHS)
vpath %.s $(ASM_PATHS)

OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

STM32F103xx: OUTPUT_FILENAME := STM32F103xx
STM32F103xx: LINKER_SCRIPT=STM32F103XC_FLASH.ld
STM32F103xx: $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo Linking target: $(OUTPUT_FILENAME).out
	$(NO_ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e finalize

## Create build directories                   #创建存放编译结果的文件夹
$(BUILD_DIRECTORIES):
	echo $(MAKEFILE_NAME)
	$(MK) $@

# Create objects from C SRC files             #对于C源文件，做编译得.o文件
$(OBJECT_DIRECTORY)/%.o: %.c
	@echo Compiling file: $(notdir $<)
	$(NO_ECHO)$(CC) $(CFLAGS) $(INC_PATHS) -c -o $@ $<

# Assemble files                              #对于汇编对象，做汇编得.o文件
$(OBJECT_DIRECTORY)/%.o: %.s
	@echo Compiling file: $(notdir $<)
	$(NO_ECHO)$(CC) $(ASMFLAGS) $(INC_PATHS) -c -o $@ $<


# Link                                        #链接.o文件生成可执行文件.out
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out: $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo Linking target: $(OUTPUT_FILENAME).out
	$(NO_ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out


## Create binary .bin file from the .out file #与下文genbin一样
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	@echo Preparing: $(OUTPUT_FILENAME).bin
	$(NO_ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin

## Create binary .hex file from the .out file #与下文genhex一样
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	@echo Preparing: $(OUTPUT_FILENAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex

                                              #生成二进制文件并输出大小
finalize: genbin genhex echosize

##                                            #由.out文件生成.bin文件
genbin:
	@echo Preparing: $(OUTPUT_FILENAME).bin
	$(NO_ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin

## Create binary .hex file from the .out file #由.out文件生成.hex文件
genhex: 
	@echo Preparing: $(OUTPUT_FILENAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex

##                                            #输出.out文件大小
echosize:
	-@echo ''
	$(NO_ECHO)$(SIZE) $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	-@echo ''

                                              #清除整个_build文件夹
clean:
	$(RM) $(BUILD_DIRECTORIES)      

                                              #清除_build文件夹中的所有.o文件
cleanobj:
	$(RM) $(BUILD_DIRECTORIES)/*.o  

JLINK_SCRIPT = ./JLinkCommand.jlink     #JLinkExe烧录动作的脚本文件，连接后执行
flash: 
	@echo Flashing: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex
	JLinkExe -device STM32F103XC -if SWD -speed 4000 -autoconnect 1 -CommanderScript $(JLINK_SCRIPT)                #连接指定芯片，SWD口通信，速率4000kHz

