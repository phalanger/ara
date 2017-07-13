CFLAGS = -I../ -std=c++11 -ftest-coverage -fprofile-arcs 
CLIBS = -L/home/cmdev/lib -lboost_system -lboost_thread -lboost_date_time -lpthread -lrt

VPATH = ../test:
UNITTESTOBJS = $(patsubst %.cpp,%.o,$(notdir $(wildcard ../test/*.cpp))) 

.SUFFIXES: .cpp
.cpp.o:
	$(CC) -c $(CFLAGS) $<

all:unittest

unittest: $(UNITTESTOBJS)
	g++ $(CFLAGS) -o unittest $(UNITTESTOBJS) $(CLIBS)

test: unittest
	rm -rf ./results
	lcov --directory . --zerocounters
	./unittest
	lcov --directory . --capture --output-file app.info
	genhtml -o results app.info

clean:
	rm -f *.o *.d unittest

## Commands to generate dependency files 
GEN_DEPS.c= $(CC) -E -MM $(DEFS) $(CFLAGS) 
GEN_DEPS.cc= $(CXX) -E -MM $(DEFS) $(CFLAGS) 
%.d : %.c 
	@echo "generating dependency information for $<" 
	@$(SHELL) -ec '$(GEN_DEPS.c) $< \
		| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
		[ -s $@ ] || rm -f $@'

%.d : %.cpp
	@echo "generating dependency information for $<"
	@$(SHELL) -ec '$(GEN_DEPS.cc) $< \
		| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
		[ -s $@ ] || rm -f $@' 

OBJSDEP = $(UNITTESTOBJS:.o=.d)

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

