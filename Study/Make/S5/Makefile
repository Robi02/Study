################################################################

### [ Build Params ] ###
BP_MODE = __DEBUG__
BP_SYS  = __X86_SYS__
BP_END  = __LIT_END_CPU__

### [ Variable Params (Library Build) ] ###
LIB_NAME = MyProtocol
LIB_SRCS = MySocket.c MyThread.c MyMutex.c MySemaphore.c

### [ Variable Params (Target Build) ] ###
TARGET_SRCS  = MyClient.c TestConnection.c

### [ Variable Params (Library Linking) ] ###
LIBS = -l$(LIB_NAME) 
LIBS += -lws2_32
LIB_DIRS = -L.

################################################################

# [ CLI Commands ] #
CC = gcc
AR = ar
RM = del
EXE = .exe

# [ Constant Params (Library Build) ] #
LIB_FULL_NAME = $(LIB_NAME).lib
LIB_OBJS = $(LIB_SRCS:%.c=%.o)

# [ Constant Params (Target Build) ] #
TARGET_OBJS   = $(TARGET_SRCS:%.c=%.o)
TARGET_NAMES  = $(TARGET_SRCS:%.c=%)

# [ Dependency File ] #
DP_FILE = dp_file

################################################################

# [ Build Suffixes ] #
.SECONDEXPANSION:
.SUFFIXES: .c .o

################################################################

# [ Build ALL ] #
all : $(LIB_FULL_NAME) $(TARGET_NAMES)

# [ Compile Soruces ] #
.c.o : 
	$(CC) $(CFLARGS) -c $< -o $@

# [ Build Libraries ] #
$(LIB_FULL_NAME) : $(LIB_OBJS)
	$(AR) rcv $@ $(LIB_OBJS)
	$(AR) s $@

# [ Build Targets ] #
$(TARGET_NAMES) : $$@.o
	$(CC) -o $@ $< $(LIBS) $(LIB_DIRS)

# [ Make Dependency File ] #
$(DP_FILE):
	$(CC) -MM $(LIB_SRCS) $(TARGET_SRCS) > $(DP_FILE)

# [ Delete Build Relative Files ] #
clean :
	$(RM) $(LIB_FULL_NAME) $(LIB_OBJS) $(addsuffix $(EXE), $(TARGET_NAMES)) $(TARGET_OBJS) $(DP_FILE)

################################################################

# [ Dependency Include ] #
ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), $(DP_FILE))
ifneq ($(strip $(LIB_SRCS) $(TARGET_SRCS)),)
include $(DP_FILE)
endif
endif
endif

# [ Build Mode ] #
ifneq ($(BP_MODE), __DEBUG__)
CFLARGS += -O2 -D__RELEASE__
else
CFLARGS += -g -O0 -D__DEBUG__
endif
CFLARGS += -D$(BP_SYS)
CFLARGS += -D$(BP_END)

################################################################

# Reference from:
# 1. http://developinghappiness.com/?p=282
# 2. http://kwangja.tistory.com/entry/Makefile-%EB%82%B4%EC%97%90%EC%84%9C-%EC%82%AC%EC%9A%A9%EB%90%98%EB%8A%94-%EB%AC%B8%EB%B2%95-%EC%A0%95%EB%A6%AC

################################################################