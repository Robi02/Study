################################################################

### [ Build Params ] ###
BP_MODE = __DEBUG__
BP_OS   = __CENTOS__
BP_SYS  = __X64_SYS__
BP_END  = __LITTLE_ENDIAN_CPU__

### [ Variable Params (Library Build) ] ###
LIB_NAME = 
LIB_SRCS = 

### [ Variable Params (Target Build) ] ###
TARGET_SRCS  = MyClient.c TestConnection.c

### [ Variable Params (Library Linking) ] ###
LIBS = -l$(LIB_NAME) 
LIB_DIRS = -L$(OBJS_DIR)

################################################################

# [ Build Mode ] #
ifneq ($(BP_MODE), __DEBUG__)
OBJS_DIR = release
CFLARGS += -O2 -D__RELEASE__
else
OBJS_DIR = debug
CFLARGS += -g -O0 -D__DEBUG__
endif
CFLARGS += -D$(BP_SYS)
CFLARGS += -D$(BP_END)

# [ Dependency File ] #
DP_FILE = $(OBJS_DIR)/dp_file

################################################################

# [ CLI Commands ] #
CC = gcc
AR = ar
ifeq ($(BP_OS), __WINDOWS__)
RM = del /S /q
RMDIR = rmdir /S /q
EXE = .exe
else
RM = rm -f
RMDIR = rmdir /f
EXE =
endif
MKDIR = mkdir

# [ Constant Params (Library Build) ] #
LIB_FULL_NAME = $(OBJS_DIR)/$(LIB_NAME).lib
LIB_OBJS = $(LIB_SRCS:%.c=$(OBJS_DIR)/%.o)

# [ Constant Params (Target Build) ] #
TARGET_OBJS   = $(TARGET_SRCS:%.c=$(OBJS_DIR)/%.o)
TARGET_NAMES  = $(TARGET_SRCS:%.c=$(OBJS_DIR)/%)

################################################################

# [ Build Suffixes ] #
.SECONDEXPANSION:
.SUFFIXES: .c .o

################################################################

# [ Build ALL ] #
all : $(LIB_FULL_NAME) $(TARGET_NAMES)

# [ Compile Soruces ] #
$(OBJS_DIR)/%.o : %.c
	$(CHK_MKDIR)
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
	@echo "> Making dp_file..."
	$(CHK_MKDIR)
	$(CHK_DP)
	@echo "> Making dp_file done!"

# [ Delete Build Relative Files ] #
clean :
	$(CHK_RMDIR)

################################################################

# [ CHK_MKDIR ] #
CHK_MKDIR = @
ifeq ($(BP_OS), __WINDOWS__)
CHK_MKDIR += if not exist $(OBJS_DIR) ( $(MKDIR) $(OBJS_DIR) )
else
CHK_MKDIR += `[ -d $(OBJS_DIR) ] || $(MKDIR) $(OBJS_DIR)`
endif

# [ CHK_RMDIR ] #
CHK_RMDIR = @
ifeq ($(BP_OS), __WINDOWS__)
CHK_RMDIR += if exist $(OBJS_DIR) ( $(RMDIR) $(OBJS_DIR) ) else ( echo "make-clean : There is nothing to clean.")
else
CHK_RMDIR += `[ -d $(OBJS_DIR) ] && $(RMDIR) $(OBJS_DIR)`
endif

# [ CHK_DP ] #
CHK_DP = @
ifeq ($(BP_OS), __WINDOWS__)
CHK_DP += for %%f in ($(LIB_SRCS:%.c=%) $(TARGET_SRCS:%.c=%)) do \
			$(CC) -MM -MT $(OBJS_DIR)/%%f.o %%f.c >> $(DP_FILE)
else
CHK_DP += for FILE in $(LIB_SRCS:%.c=%) $(TARGET_SRCS:%.c=%); do \
			$(CC) -MM -MT $(OBJS_DIR)/$$FILE.o $$FILE.c >> $(DP_FILE); \
		  done
endif
	
# [ Dependency Include ] #
ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), $(DP_FILE))
ifneq ($(strip $(LIB_SRCS) $(TARGET_SRCS)),)
include $(DP_FILE)
endif
endif
endif

################################################################

#	@echo "========================================"
#	@echo "         [ Compiling Soruces ]          "
#	@echo "                                        "
#	@echo " > Mode:   $(BP_MODE)                   "
#	@echo " > Target: $(BP_OS)($(BP_SYS))          "
#	@echo " > Endian: $(BP_END)                    "
#	@echo "========================================"

# Reference from:
# 1. http://developinghappiness.com/?p=282
# 2. http://kwangja.tistory.com/entry/Makefile-%EB%82%B4%EC%97%90%EC%84%9C-%EC%82%AC%EC%9A%A9%EB%90%98%EB%8A%94-%EB%AC%B8%EB%B2%95-%EC%A0%95%EB%A6%AC

################################################################