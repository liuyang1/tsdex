GTEST_DIR     = ../gtest
USER_DIR      = .
SRC_DIR       = ..

INCS		 += -I$(GTEST_DIR)/include -I$(SRC_DIR)
CXXFLAGS     += -g -Wall -Wextra

CXX				= g++

FILE_SUFFIX   	= cpp
TEST_SUFFIX		= test
# buffer.test.cpp
TESTSRCS  		:= $(wildcard *.$(FILE_SUFFIX))
# buffer.test.o
TESTOBJS		:= $(patsubst %.$(FILE_SUFFIX),%.o,$(TESTSRCS))
# buffer.test
TESTS			:= $(patsubst %.$(FILE_SUFFIX),%,$(TESTSRCS))
# buffer.o
OBJS			:= $(patsubst %.$(TEST_SUFFIX),%.o,$(TESTS))


GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h
all : gtest.a gtest_main.a $(TESTS) 

card:
	echo $(OBJS) && echo $(TESTS) && echo $(TESTOBJS) && echo $(TESTSRCS)

GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(INCS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(INCS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

# Builds a sample test.  A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.

# buffer.o : $(SRC_DIR)/buffer.cpp $(SRC_DIR)/buffer.h 
# 	$(CXX) $(INCS) $(CXXFLAGS) -c $(SRC_DIR)/buffer.cpp
# buffer_test.o:$(USER_DIR)/buffer_test.cpp $(SRC_DIR)/buffer.cpp
# 	$(CXX) $(INCS) $(CXXFLAGS) -c $(USER_DIR)/buffer_test.cpp
# buffer_test:buffer_test.o buffer.o gtest_main.a
# 	$(CXX) $(INCS) $(CXXFLAGS) $^ -o $@ -lpthread

$(TESTOBJS): %.o : %.$(FILE_SUFFIX) 
	$(CXX) -c $(INCS) $< -o $@
$(OBJS): %.o : ../%.$(FILE_SUFFIX)
	$(CXX) -c $(INCS) $< -o $@
$(TESTS): % : %.o  gtest_main.a $(OBJS) ../mylog.o ../dexconf.o ../myconf.o
	$(CXX) $(CXXFLAGS) $^ -o $@ -lpthread

.PHONY:run clean stat
run:
	./base.test
	./ts.test

clean :
	rm -f $(TESTS) gtest.a gtest_main.a *.o core core.*
stat:
	cloc .
