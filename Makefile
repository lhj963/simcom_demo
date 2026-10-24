
CFLAGS = -c -O2 
	 
MAIN_DIR = .

TARGET_BIN_DIR = $(MAIN_DIR)/bin
TARGET_OBJ_DIR = $(MAIN_DIR)/obj

BIN_NAME  = demo_app

TESTC_DIR = $(MAIN_DIR)/src

INCLUDE_PREFIX = -I

LINK_PREFIX = -L

ALL_PATHS += ./inc
ALL_PATHS += $(LIB_PATH)/usr/include
ALL_PATHS += $(LIB_PATH)/usr/include/sdk_api
ALL_PATHS += $(LIB_PATH)/usr/include/sdk_api/common
ALL_PATHS += $(LIB_PATH)/usr/include/glib-2.0
ALL_PATHS += $(LIB_PATH)/usr/include/bluez-5.30
ALL_INCLUDES = $(addprefix $(INCLUDE_PREFIX), $(ALL_PATHS))

LIB_DIRS += $(LIB_PATH)/usr/lib
ALL_LINKS += $(addprefix $(LINK_PREFIX), $(LIB_DIRS))

OBJ_CMD = -o 

LD_CMD = -o

TEST_OBJS = $(TARGET_OBJ_DIR)/main.o

SIMCOM_CFLAGS := 

LFLAGS +=  -lm -lsdk -lcurl -lcrypto -lssl -lubus -lubox -lrilutil -ljson-c -luci -lpolarssl -llog -lprop2uci

ifeq ($(SIMCOM_BLUETOOTH_FUNC),TRUE)
	CFLAGS += -DFEATURE_DEMO_BT
	LIBS += -l:libbluetooth.so -lglib-2.0 -lreadline -lncurses -lgatt -lshared
endif

CFLAGS +=$(SIMCOM_CFLAGS)

BIN_OBJS = $(TEST_OBJS) 

$(TARGET_OBJ_DIR)/%.o:$(TESTC_DIR)/%.c
	echo ---------------------------------------------------------
	#echo $(ALL_PATHS)
	#echo $(ALL_INCLUDES)
	echo Build OBJECT $(@) from SOURCE $<
	$(CC) $(CFLAGS) $(ALL_INCLUDES) $(OBJ_CMD) $@ $<
	echo ---------------------------------------------------------

$(TARGET_OBJ_DIR)/%.o:$(TESTC_DIR)/%.cpp
	#echo $(ALL_PATHS)
	#echo $(ALL_INCLUDES)
	echo Build OBJECT $(@) from SOURCE $<
	$(CC) $(CFLAGS) $(ALL_INCLUDES) $(OBJ_CMD) $@ $<
	echo ---------------------------------------------------------

.PHONY: all clean

all:prep bin 

prep:
	@if test ! -d $(TARGET_BIN_DIR); then mkdir $(TARGET_BIN_DIR); fi
	@if test ! -d $(TARGET_OBJ_DIR); then mkdir $(TARGET_OBJ_DIR); fi

bin:$(BIN_OBJS)
	@echo Create bin file $(BIN_NAME)
	@$(CXX) $(ALL_LINKS) $(BUILD_LDFLAGS) -o $(TARGET_BIN_DIR)/$(BIN_NAME) $^ $(LFLAGS)
	@echo ---------------------------------------------------------
	
clean:
	@rm -fr $(TARGET_OBJ_DIR) $(TARGET_BIN_DIR) 
	
