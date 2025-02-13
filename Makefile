TARGET = fluidlite
BUILD = Debug

ifeq ($(BUILD), Debug)
BUILD_DIR = Debug
else
BUILD_DIR = Release
endif

CFLAGS = -m32
C_DEFS = -DWITH_FLOAT

C_SOURCES =  \
$(wildcard src/*.c)

CC = gcc
AS = gcc -x assembler-with-cpp
AR = ar
CP = objcopy
SZ = size

AS_DEFS = 

# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-Iinclude \
-Isrc


ifeq ($(BUILD), Debug)
C_DEFS += -DDEBUG=1
CFLAGS += -g3 -gdwarf-2   # -g3 生成包含宏定义的调试信息
OPT = -Og
else
OPT = -O2
endif

# compile gcc flags
ASFLAGS = $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS += $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections



# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"
# -MMD: 这是GCC编译器的一个选项，用于生成依赖文件（.d 文件）。
# -MP: 这也是GCC编译器的一个选项，用于为每个依赖的头文件生成一个空的伪目标规则。
# -MF: 这是GCC编译器的一个选项，用于指定生成的依赖文件的名称。
# $(@:%.o=%.d): 这是一个Makefile中的模式替换语法。
# 	$@ 是Makefile中的自动变量，表示当前目标文件（例如 main.o）。
# 	$(@:%.o=%.d) 表示将目标文件的扩展名从 .o 替换为 .d。例如，如果目标文件是 main.o，那么 $(@:%.o=%.d) 会生成 main.d。
# 	因此，-MF"$(@:%.o=%.d)" 会将依赖文件的名称设置为与目标文件同名，但扩展名为 .d。例如，main.o 对应的依赖文件是 main.d。


LIBS = -lc -lm
LIBDIR = 
LDFLAGS = -v $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/lib$(TARGET).a


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

# -Wa 是 GCC 的选项，用于将后续参数传递给汇编器。
# -a,-ad,-alms 是汇编器的选项，用于生成汇编列表文件（.lst 文件）。
# $(BUILD_DIR)/$(notdir $(<:.c=.lst)) 是生成的汇编列表文件的路径：
# $< 表示第一个依赖文件（即 .c 文件）。
# $(<:.c=.lst) 将 .c 替换为 .lst。
# $(notdir ...) 去掉路径，只保留文件名。
# 最终生成的 .lst 文件会放在 $(BUILD_DIR) 目录下。


$(BUILD_DIR)/lib$(TARGET).a: $(OBJECTS) Makefile
	$(AR) rcs $@ $(OBJECTS)
	$(SZ) -t $@
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***