
PROJECT_PATH 	= $(shell pwd)
OBJ_PATH 	    = $(PROJECT_PATH)/obj
EXAMPLE_PATH    = $(PROJECT_PATH)/example

TCP_SERVER      = $(EXAMPLE_PATH)/tcp-server
TCP_CLIENT      = $(EXAMPLE_PATH)/tcp-client
KERNEL_LIST     = $(EXAMPLE_PATH)/kernel-list
CJSON           = $(EXAMPLE_PATH)/cJSON-test
NET_TRIGGER     = $(EXAMPLE_PATH)/net-trigger
NET_TIMER       = $(EXAMPLE_PATH)/net-timer
RING_BUFFER     = $(EXAMPLE_PATH)/ring-buffer
SHM_CACHE       = $(EXAMPLE_PATH)/shm-cache-demo
SHM_CACHE_WRITE = $(EXAMPLE_PATH)/shm-cache-write
SHM_CACHE_READER= $(EXAMPLE_PATH)/shm-cache-reader
SHM_QUEUE_WRITE = $(EXAMPLE_PATH)/shm-queue-write
SHM_QUEUE_READER= $(EXAMPLE_PATH)/shm-queue-reader
LOG_DEMO        = $(EXAMPLE_PATH)/log-demo

LIB_S_SCHUDULE  = libschudule.a
LIB_SCHUDULE    = libschudule.so

CFLAGS   	= -lpthread -O0  -DOS_LINUX -g -lm -lrt
LIBS     	= -I./include

SOURCE_C 	= $(wildcard $(PROJECT_PATH)/src/*.c)
SOURCE_O 	= $(patsubst %.c, $(OBJ_PATH)/%.o, $(notdir $(SOURCE_C)))

EXAMPLE_C   = $(wildcard $(EXAMPLE_PATH)/*.c)
EXAMPLE_O 	= $(patsubst %.c, $(OBJ_PATH)/%.o, $(notdir $(EXAMPLE_C)))

.PHONY: clean install obj

all: clean obj $(LIB_S_SCHUDULE) $(LIB_SCHUDULE) example install

example: $(TCP_CLIENT) $(TCP_SERVER) $(KERNEL_LIST) $(CJSON) $(NET_TRIGGER) $(NET_TIMER) $(RING_BUFFER) $(LOG_DEMO) $(SHM_CACHE) $(SHM_CACHE_WRITE) $(SHM_CACHE_READER) $(SHM_QUEUE_WRITE) $(SHM_QUEUE_READER) 

#########################################################################
$(SHM_QUEUE_WRITE): $(OBJ_PATH)/shm-queue-write.o $(SOURCE_O)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

$(SHM_QUEUE_READER): $(OBJ_PATH)/shm-queue-reader.o $(SOURCE_O)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

$(SHM_CACHE_WRITE): $(OBJ_PATH)/shm-cache-write.o $(SOURCE_O)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) 

$(SHM_CACHE_READER): $(OBJ_PATH)/shm-cache-reader.o $(SOURCE_O)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) 

$(SHM_CACHE): $(OBJ_PATH)/shm-cache-demo.o $(SOURCE_O)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) 
	
$(LOG_DEMO): $(OBJ_PATH)/log-demo.o $(SOURCE_O)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) 

$(RING_BUFFER): $(OBJ_PATH)/ringbuffer-demo.o $(SOURCE_O)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) 

$(NET_TIMER): $(OBJ_PATH)/net-timer.o $(SOURCE_O)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) 

$(NET_TRIGGER): $(OBJ_PATH)/net-trigger.o $(SOURCE_O)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) 

$(KERNEL_LIST): $(OBJ_PATH)/kernel-list.o $(SOURCE_O)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) 

$(TCP_SERVER): $(OBJ_PATH)/tcp-server.o $(SOURCE_O)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) 

$(TCP_CLIENT): $(OBJ_PATH)/tcp-client.o $(SOURCE_O)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) 

$(CJSON): $(OBJ_PATH)/cJSON-test.o $(SOURCE_O)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) 

#########################################################################
$(OBJ_PATH)/shm-queue-reader.o: $(EXAMPLE_PATH)/shm-queue-reader.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(LIBS)

$(OBJ_PATH)/shm-queue-write.o: $(EXAMPLE_PATH)/shm-queue-write.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(LIBS)

$(OBJ_PATH)/shm-cache-write.o: $(EXAMPLE_PATH)/shm-cache-write.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(LIBS)

$(OBJ_PATH)/shm-cache-reader.o: $(EXAMPLE_PATH)/shm-cache-reader.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(LIBS)

$(OBJ_PATH)/shm-cache-demo.o: $(EXAMPLE_PATH)/shm-cache-demo.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(LIBS)

$(OBJ_PATH)/log-demo.o: $(EXAMPLE_PATH)/log-demo.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(LIBS)

$(OBJ_PATH)/ringbuffer-demo.o: $(EXAMPLE_PATH)/ringbuffer-demo.c 
	$(CC) -c $^ -o $@ $(CFLAGS) $(LIBS)

$(OBJ_PATH)/net-timer.o: $(EXAMPLE_PATH)/net-timer.c 
	$(CC) -c $^ -o $@ $(CFLAGS) $(LIBS)

$(OBJ_PATH)/net-trigger.o: $(EXAMPLE_PATH)/net-trigger.c 
	$(CC) -c $^ -o $@ $(CFLAGS) $(LIBS)

$(OBJ_PATH)/kernel-list.o: $(EXAMPLE_PATH)/kernel-list.c 
	$(CC) -c $^ -o $@ $(CFLAGS) $(LIBS)

$(OBJ_PATH)/tcp-server.o: $(EXAMPLE_PATH)/tcp-server.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(LIBS)

$(OBJ_PATH)/tcp-client.o: $(EXAMPLE_PATH)/tcp-client.c 
	$(CC) -c $^ -o $@ $(CFLAGS) $(LIBS)

$(OBJ_PATH)/cJSON-test.o: $(EXAMPLE_PATH)/cJSON-test.c 
	$(CC) -c $^ -o $@ $(CFLAGS) $(LIBS)

#########################################################################
$(LIB_SCHUDULE): $(SOURCE_C)
	$(CC) -fPIC -shared -o $@ $^ $(LIBS)

$(LIB_S_SCHUDULE): $(SOURCE_O)
	$(AR) rcs $@ $^

$(OBJ_PATH)/%.o: $(PROJECT_PATH)/src/%.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(LIBS)

#########################################################################
obj:
ifeq ("$(wildcard $(OBJ_PATH))","")
	mkdir $(OBJ_PATH)
endif

clean:
	-$(RM) -r $(OBJ_PATH)/*.o 
	-$(RM) -r __install
	-$(RM) -r $(TCP_CLIENT) $(TCP_SERVER) $(KERNEL_LIST) $(CJSON) $(SHM_CACHE)
	-$(RM) -r $(LOG_DEMO)  $(NET_TRIGGER) $(NET_TIMER) $(RING_BUFFER) $(SHM_CACHE_WRITE) $(SHM_CACHE_READER) $(SHM_QUEUE_WRITE) $(SHM_QUEUE_READER)      

install:
	mkdir __install && mkdir -p  __install/lib
	mv $(LIB_SCHUDULE) __install/lib
	mv $(LIB_S_SCHUDULE)  __install/lib
	cp -r include __install
