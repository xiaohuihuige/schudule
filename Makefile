CURRENT_DIR = $(shell pwd)
OUT_DIR     = $(CURRENT_DIR)/out
OBJ_DIR     = $(OUT_DIR)/obj
IPCS_DIR    = $(CURRENT_DIR)/src/ipcs
NET_DIR     = $(CURRENT_DIR)/src/net
UTIL_DIR    = $(CURRENT_DIR)/src/util
BIN_DIR     = $(OUT_DIR)/bin
INCLUDE_DIR = $(OUT_DIR)/include
LIB_DIR     = $(OUT_DIR)/lib
SRC_DIR     = $(CURRENT_DIR)/src

STATIC_NAME  = $(LIB_DIR)/libschedule.a
DYNAMIC_NAME = $(LIB_DIR)/libschedule.so

OBJ_FILE = $(wildcard $(OBJ_DIR)/*.o)

EXECUTABLES = $(patsubst example/%.c, $(BIN_DIR)/%, $(wildcard example/*.c))

CFLAGS = -I$(IPCS_DIR) -I$(NET_DIR) -I$(UTIL_DIR)  -lpthread -lrt -Wall -fPIC -g

.PHONY: all clean subdir TEMP_PATH

all: TEMP_PATH subdir $(EXECUTABLES)

TEMP_PATH:
ifeq ("$(wildcard $(OUT_DIR))","")
	mkdir $(OUT_DIR)
endif
ifeq ("$(wildcard $(OBJ_DIR))","")
	mkdir $(OBJ_DIR)
endif
ifeq ("$(wildcard $(BIN_DIR))","")
	mkdir $(BIN_DIR)
endif
ifeq ("$(wildcard $(INCLUDE_DIR))","")
	mkdir $(INCLUDE_DIR)
endif
ifeq ("$(wildcard $(LIB_DIR))","")
	mkdir $(LIB_DIR)
endif

$(BIN_DIR)/%: example/%.c | TEMP_PATH
	$(CC) -o $@ $^ $(OBJ_FILE) $(CFLAGS)

subdir:
	$(MAKE) -C $(SRC_DIR) CFLAGS="$(CFLAGS)" OBJ_DIR=$(OBJ_DIR) IPCS_DIR=$(IPCS_DIR) NET_DIR=$(NET_DIR) STATIC_NAME=$(STATIC_NAME) UTIL_DIR=$(UTIL_DIR) DYNAMIC_NAME=$(DYNAMIC_NAME) 
	find $(SRC_DIR) -name "*.h" -exec cp {} $(INCLUDE_DIR) \;

clean:
	-$(RM) -rf $(OUT_DIR) 
	
install:
ifeq ("$(wildcard /usr/local/include/schedule)","")
	sudo mkdir /usr/local/include/schedule
endif
	-sudo cp $(DYNAMIC_NAME)  /usr/local/lib/libschedule.so
	-sudo cp $(BIN_DIR)/*      /usr/local/bin/
	-sudo cp $(INCLUDE_DIR)/*  /usr/local/include/schedule/
	-sudo ldconfig



