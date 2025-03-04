TARGET = fluidliter
ARCH = i386
BUILD = Debug

ifeq ($(WITH_FLOAT), 0)
	C_DEFS = 
else
	C_DEFS = -DWITH_FLOAT
endif

ifeq ($(BUILD), Debug)
	BUILD_DIR ?= Debug
	CFLAGS = -g3 -gdwarf-2   # -g3 生成包含宏定义的调试信息
	OPT = -Og
	C_DEFS += -DDEBUG=1
else
	BUILD_DIR ?= Release
	CFLAGS = 
	OPT = -O2
endif

ifeq ($(EMPTY_REVERB), 1)
	CFLAGS += -DEMPTY_REVERB
endif

ifeq ($(EMPTY_CHORUS), 1)
	CFLAGS += -DEMPTY_CHORUS
endif

ifeq ($(GEN_TABLE_RUNTIME), 1)
	CFLAGS += -DGEN_TABLE_RUNTIME
endif

ifeq ($(ENABLE_7th_DSP), 1)
	CFLAGS += -DENABLE_7th_DSP
endif

ifneq ($(DEFAULT_LOG_LEVEL),)
	CFLAGS += -DDEFAULT_LOG_LEVEL=$(DEFAULT_LOG_LEVEL)
endif

CC = gcc
AS = gcc -x assembler-with-cpp
CP = objcopy
SZ = size

ifeq ($(MAKECMDGOALS), js)
	ARCH = wasm
endif

ifeq ($(ARCH), i386)
	CFLAGS += -m32
else ifeq ($(ARCH), arm)
	PREFIX = arm-none-eabi-
	CC = $(PREFIX)gcc
	AS = $(PREFIX)gcc -x assembler-with-cpp
	CP = $(PREFIX)objcopy
	SZ = $(PREFIX)size
else ifeq ($(ARCH), wasm)
	CC = emcc
	SZ = emsize
else
	# Default to native arch
endif

ifeq ($(OS), Windows_NT)
	LIBS =
	TARGET_LIB = $(TARGET).lib
else
	LIBS = -lc -lm
	TARGET_LIB = lib$(TARGET).a
endif

ifeq ($(ARCH), arm)
	CPU = -mcpu=cortex-m4
	FPU = -mfpu=fpv4-sp-d16
	FLOAT-ABI = -mfloat-abi=hard
	MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)
	LIBS += -lnosys 
	LIBDIR = 
	LDFLAGS = -specs=nosys.specs $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections
else
	MCU = 
	LIBDIR = 
	LDFLAGS = $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map -Wl,--gc-sections
endif


C_SOURCES =  \
$(wildcard src/*.c)

AS_DEFS = 

# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-Iinclude \
-Isrc

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections



# Generate dependency information
C_DEPEND = -MMD -MP -MF"$(@:%.o=%.d)"
# -MMD: 这是GCC编译器的一个选项，用于生成依赖文件（.d 文件）。
# -MP: 这也是GCC编译器的一个选项，用于为每个依赖的头文件生成一个空的伪目标规则。
# -MF: 这是GCC编译器的一个选项，用于指定生成的依赖文件的名称。
# $(@:%.o=%.d): 这是一个Makefile中的模式替换语法。
# 	$@ 是Makefile中的自动变量，表示当前目标文件（例如 main.o）。
# 	$(@:%.o=%.d) 表示将目标文件的扩展名从 .o 替换为 .d。例如，如果目标文件是 main.o，那么 $(@:%.o=%.d) 会生成 main.d。
# 	因此，-MF"$(@:%.o=%.d)" 会将依赖文件的名称设置为与目标文件同名，但扩展名为 .d。例如，main.o 对应的依赖文件是 main.d。


# Verbose mode
ifeq ("$(V)","1")
$(info CFLAGS  $(CFLAGS) ) $(info )
$(info LDFLAGS $(LDFLAGS)) $(info )
$(info ASFLAGS $(ASFLAGS)) $(info )
endif


# default action: build all
all: $(BUILD_DIR)/$(TARGET_LIB)
	@echo "done!"
	

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

ifeq ($(ARCH), arm)
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR) 
	@echo CC $(notdir $@)
	@$(CC) -c $(CFLAGS) $(C_DEPEND) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@
else
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR) 
	@echo CC $(notdir $@)
	@$(CC) -c $(CFLAGS) $(C_DEPEND) $< -o $@
endif

# -Wa 是 GCC 的选项，用于将后续参数传递给汇编器。
# -a,-ad,-alms 是汇编器的选项，用于生成汇编列表文件（.lst 文件）。
# $(BUILD_DIR)/$(notdir $(<:.c=.lst)) 是生成的汇编列表文件的路径：
# $< 表示第一个依赖文件（即 .c 文件）。
# $(<:.c=.lst) 将 .c 替换为 .lst。
# $(notdir ...) 去掉路径，只保留文件名。
# 最终生成的 .lst 文件会放在 $(BUILD_DIR) 目录下。

js: $(BUILD_DIR)/fluidsynth.js

$(BUILD_DIR)/fluid_wasm.o: example/src/fluid_wasm.c
	$(CC) -c $(CFLAGS) $(C_DEPEND) $< -o $@

$(BUILD_DIR)/fluidsynth.js: $(OBJECTS) $(BUILD_DIR)/fluid_wasm.o
	$(CC) $(CFLAGS) -s EXPORTED_FUNCTIONS='["_get_log_level", "_set_log_level", "_fluid_log", "_fluid_init","_fluid_program_select","_fluid_noteon","_fluid_noteoff","_fluid_write_float","_get_buffer_ptr","_get_buffer_size"]' -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' -o $(BUILD_DIR)/fluidsynth.js $(OBJECTS)  $(BUILD_DIR)/fluid_wasm.o
	$(SZ) $@

# emnm Release/fluidsynth.wasm
# apt install wabt
# wasm-objdump Release/fluidsynth.wasm -s
# wasm-interp Release/fluidsynth.wasm --run-all-exports

$(BUILD_DIR)/$(TARGET_LIB): $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)
	$(SZ) -t $@
	
$(BUILD_DIR):
	mkdir $@		


clean:
	-rm -fR $(BUILD_DIR)



echo: #要测试makefile里的语句，必须放到目标中执行。而且还不能放在all前面
	@echo $(C_DEFS)
	@echo "Operating System: $(OS)"
ifeq ($(ARCH), i386)
	@echo "i386 on x86_64 (gcc-multilib)"
else ifeq ($(ARCH),)
	@echo "Default to native arch"
else
	@echo $(ARCH)
endif

-include $(wildcard $(BUILD_DIR)/*.d)   #dependencies


# *** EOF of fluidliter, below is for test cases***


TEST_DIR = example/src

TEST_SOURCES =  \
$(wildcard $(TEST_DIR)/test*.c)

# C includes
TEST_INCLUDES =  \
-Iinclude \
-Isrc \
-Iexample

TEST_EXECS = $(patsubst $(TEST_DIR)/%.c, %, $(TEST_SOURCES))


${BUILD_DIR}/%: $(TEST_DIR)/%.c  $(BUILD_DIR)/$(TARGET_LIB) #将每个 .c 文件编译为同名的可执行文件
	$(CC) $<  $(CFLAGS) -Wno-unused-result -L${BUILD_DIR} -l$(TARGET) ${LDFLAGS} -o $@ 

# 规则名如test2, 而生成的文件是${BUILD}/test2, 导致每次运行make test2都会重新编译。

# 动态生成每个可执行文件的运行规则。比如test_song, 生成规则test_song_run，通过make test_song_run可执行该测试。
define RUN_RULE
$(1)_run: $(1)
	@echo "Running $(1)..."
ifeq ($(tool), massif)
	@valgrind --tool=massif --massif-out-file=${BUILD_DIR}/massif_$(1) ${BUILD_DIR}/$(1)
	massif-visualizer ${BUILD_DIR}/massif_$(1)
else ifeq ($(tool), callgrind)
	@valgrind --dsymutil=yes --tool=callgrind --dump-instr=yes --collect-jumps=yes --callgrind-out-file=${BUILD_DIR}/callgrind_$(1) ${BUILD_DIR}/$(1)
	kcachegrind ${BUILD_DIR}/callgrind_$(1)
else
	@${BUILD_DIR}/$(1)
endif
endef

define RUN_RULE2
$(1): ${BUILD_DIR}/$(1)
endef

# 为每个可执行文件生成运行规则
$(foreach exec,$(TEST_EXECS),$(eval $(call RUN_RULE,$(exec))))
$(foreach exec,$(TEST_EXECS),$(eval $(call RUN_RULE2,$(exec))))


echo_test:
	@echo $(TEST_EXECS)

run_test: $(TEST_EXECS)
	@echo "Running all tests..."
	@for exec in $(TEST_EXECS); do \
		echo "Running $$exec..."; \
		${BUILD_DIR}/$$exec; \
		if [ $$? -ne 0 ]; then \
			echo "Test $$exec failed!"; \
			exit 1; \
		fi; \
	done
	@echo "All tests passed!"

test: run_test

# Print out the value of a make variable.
# https://stackoverflow.com/questions/16467718/how-to-print-out-a-variable-in-makefile
print-%:
	@echo $* = $($*)
