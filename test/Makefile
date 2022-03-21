
CC = gcc
CXX = g++ -std=c++14
TARGET = test
DIRS := $(shell find . -maxdepth 5 -type d)
SRCS = $(foreach dir,$(DIRS),$(wildcard $(dir)/*.cpp))
OBJS = $(patsubst %cpp,%o,$(SRCS))
LIBSPATH = $(shell dirname ~/Work/libs)
UNAME = $(shell uname)

ARA_FLAGS = -I../
BOOST_FLAGS = -I$(LIBSPATH)/libs/boost_1_72_0/
BOOST_LIBS= -L$(LIBSPATH)/libs/boost_1_72_0/stage/lib -lboost_chrono -lboost_date_time -lboost_thread -lboost_system 
OPENSSL_FLAGS = -I$(LIBSPATH)/libs/openssl-1.1.1g/include
OPENSSL_LIBS = -L$(LIBSPATH)/libs/openssl-1.1.1g -lssl -lcrypto
ZLIB_FLAGS = -I$(LIBSPATH)/libs/zlib-1.2.11
ZLIB_LIBS = -L$(LIBSPATH)/libs/zlib-1.2.11 -lz
RAPIDJSON_FLAGS = -I$(LIBSPATH)/libs/rapidjson-1.1.0/include
CATCH_FLAGS = -I../3rd/ara/3rd/Catch/include

CFLAGS = -g -Wall $(OPT)  $(DEBUG_FLAG) $(CPUFLAGS) -Wno-dangling-else -I./ -I../ $(ARA_FLAGS) $(BOOST_FLAGS) $(OPENSSL_FLAGS) $(ZLIB_FLAGS) $(RAPIDJSON_FLAGS) $(CATCH_FLAGS)
LIBS = $(BOOST_LIBS) $(OPENSSL_LIBS) $(ZLIB_LIBS) -lpthread -ldl

.SUFFIXES: .cpp .c .no

all: $(TARGET)
	
.cpp.o:
	$(CXX) -c $(CFLAGS) $(CONFIG_CFLAGS) $< -o $@
.c.o:
	$(CC) -c $(CFLAGS) $(CONFIG_CFLAGS) $< -o $@

copyfile:
	mkdir -p $(TARGET_PATH)
	test -e $(TARGET) && mv -f $(TARGET) $(TARGET_PATH)
ifeq ($(UNAME), Darwin)
	cp $(LIBSPATH)/libs/openssl-1.1.1g/libssl.1.1.dylib $(TARGET_PATH)
	cp $(LIBSPATH)/libs/openssl-1.1.1g/libcrypto.1.1.dylib $(TARGET_PATH)
	cp $(LIBSPATH)/libs/zlib-1.2.11/libz.1.dylib $(TARGET_PATH)
	install_name_tool -change /usr/local/lib/libssl.1.1.dylib @executable_path/libssl.1.1.dylib $(TARGET_PATH)/$(TARGET)
	install_name_tool -change /usr/local/lib/libcrypto.1.1.dylib @executable_path/libcrypto.1.1.dylib $(TARGET_PATH)/$(TARGET)
	install_name_tool -change /usr/local/lib/libz.1.dylib @executable_path/libz.1.dylib $(TARGET_PATH)/$(TARGET)
	install_name_tool -change /usr/local/lib/libcrypto.1.1.dylib @executable_path/libcrypto.1.1.dylib $(TARGET_PATH)/libssl.1.1.dylib
endif
ifeq ($(UNAME), Linux)
	cp -f $(LIBSPATH)/libs/openssl-1.1.1g/libssl.so.1.1 $(TARGET_PATH)
	cp -f $(LIBSPATH)/libs/openssl-1.1.1g/libcrypto.so.1.1 $(TARGET_PATH)
	cp -f $(LIBSPATH)/libs/zlib-1.2.11/libz.so.1 $(TARGET_PATH)
endif

debug:
	make DEBUG_FLAG=-D_DEBUG all
	make TARGET_PATH=Debug copyfile

release: 
	make DEBUG_FLAG=-DNDEBUG all
	make TARGET_PATH=Release copyfile

$(TARGET):$(OBJS)
	$(CXX) $(OPT) $(SYSLIBS) -o $@ $^  $(LIBS)
	
clean:
	rm -rf $(TARGET) *.o *.d
	find . -name \*.o -exec rm -f {} \;
	find . -name \*.d -exec rm -f {} \;
	
r: clean all

## Commands to generate dependency files 
GEN_DEPS.c= $(CC) -E -MM $(DEFS) $(CFLAGS) 
GEN_DEPS.cc= $(CXX) -E -MM $(DEFS) $(CFLAGS) 
%.d : %.c 
	@echo "generating dependency information for $< $@"
	@set -e; \
	$(GEN_DEPS.c) $< | \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' > $@

%.d : %.cpp
	@echo "generating dependency information for $< $@"
	@set -e; \
	$(GEN_DEPS.cc) $< | \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' > $@

OBJSDEP = $(OBJS:.o=.d)

ifeq (,$(MAKECMDGOALS))
-include $(OBJSDEP)
else
  ifeq (test,$(MAKECMDGOALS))
  -include $(OBJSDEP)
  else
    ifneq ($(%clean,,$(MAKECMDGOALS)),)
    -include $(OBJSDEP)
    endif
  endif
endif

