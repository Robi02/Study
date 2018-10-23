CC = gcc
RM = del
SRCS = MySocket.c MyClient.c
OBJS = MySocket.o MyClient.o
HDRS = MySocket.h MyClient.h
TARGET = MyClient.exe

.SUFFIXES : .c .o

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -o $@ $(OBJS)

clean :
	$(RM) $(OBJS) $(TARGET)

$(OBJS) : $(SRCS) $(HDRS)

# << MAKE Rule >>
# [target="$(TARGET)"] : [prerequisites="$(OBJS)"]
#	[recipe="$(CC) -o $@ $(OBJS)"]
#	...
#	...

# << Make inner Variables >>
# 1. $@ : [target]
# 2. .SUFFIXES : Automatically compile .c files to .o files.

# Reference from: http://developinghappiness.com/?p=28