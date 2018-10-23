#+##############################################################

# [ CLI Commands ] #
CC = gcc
RM = del

# [ Variable Params ] #
TARGET = MyClient.exe
SRCS = MySocket.c MyClient.c MyThread.c MyMutex.c MySemaphore.c
OBJS = $(SRCS:%.c=%.o)
LIBS = -lws2_32

# [ Dependency File ] #
DP_FILE = dp_file

################################################################

.SUFFIXES : .c .o

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

$(DP_FILE):
	$(CC) -MM $(SRCS) > $(DP_FILE)

clean :
	$(RM) $(TARGET) $(OBJS) $(DP_FILE)

################################################################

ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), $(DP_FILE))
ifneq ($(SRCS),)
include $(DP_FILE)
endif
endif
endif

################################################################

# Reference from: http://developinghappiness.com/?p=174

################################################################