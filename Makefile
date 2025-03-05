CURRENT_DIR = $(shell pwd)
OUT_DIR     = $(CURRENT_DIR)/out
OBJ_DIR     = $(OUT_DIR)/obj
IPCS_DIR    = $(CURRENT_DIR)/src/ipcs
NET_DIR     = $(CURRENT_DIR)/src/net
UTIL_DIR    = $(CURRENT_DIR)/src/util
BIN_DIR     = $(OUT_DIR)/bin
SRC_DIR     = $(CURRENT_DIR)/src

STATIC_NAME  = $(OUT_DIR)/libmylib.a
DYNAMIC_NAME = $(OUT_DIR)/libmylib.so

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


$(BIN_DIR)/%: example/%.c | TEMP_PATH
	$(CC) -o $@ $^ $(OBJ_FILE) $(CFLAGS)

subdir:
	$(MAKE) -C $(SRC_DIR) CFLAGS="$(CFLAGS)" OBJ_DIR=$(OBJ_DIR) IPCS_DIR=$(IPCS_DIR) NET_DIR=$(NET_DIR) STATIC_NAME=$(STATIC_NAME) UTIL_DIR=$(UTIL_DIR) DYNAMIC_NAME=$(DYNAMIC_NAME) 
clean:
	-$(RM) -rf $(OUT_DIR) 

