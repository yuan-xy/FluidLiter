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
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASMM_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASMM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@
$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@


$(BUILD_DIR)/lib$(TARGET).a: $(OBJECTS) Makefile
	$(AR) rcs $@ $(OBJECTS)
	$(SZ) $@
	
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