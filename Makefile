TARGET		:= TSDex
OUTDIR		:= .
OBJDIR 		:= ./objs
NVERSION	:= 0.1
INCLUDES	:= 
LIBS		:= -lpthread
FILE_SUFFIX	:= .cpp
CFLAGS      +=  -O3 -Wall -Werror -g
#DCFLAGS	:= $(DCFLAGS_PASS)
DCFLAGS     += #-DUSING_RTC
#LDFLAG		:= $(LDFLAGS_PASS)
#DLDFLAGS	:= $(DLDFLAGS_PASS)
#CC			:= g++
CXX			:=	g++
CXX := ccache $(CXX)
#release	:= 1

SRCS 		:= $(wildcard *$(FILE_SUFFIX))
OBJS 		:= $(patsubst %$(FILE_SUFFIX),%.o,$(SRCS))
all:$(TARGET)
	
$(TARGET):$(OBJS)
	$(CXX) $^ -o $@ $(LIBS)
$(OBJS): %.o : %$(FILE_SUFFIX)
	$(CXX) -c $(CFLAGS) $< -o $@ 

.PHONY:run clean test trun stat
run:
	rm -f /tmp/ramdisk/* && ./$(TARGET)
clean:
	-rm -f $(OBJS) ./$(TARGET) *.nal tags && cd test && $(MAKE) clean
test:
	$(MAKE) -C test
trun:
	cd test && $(MAKE) run
stat:
	cloc . --exclude-dir=gtest,.git,test,ash

#cloc . --exclude-dir=gtest,test && cd test && $(MAKE) stat
