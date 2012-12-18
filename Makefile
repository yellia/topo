
#output file name
EXECUTABLE := topo_server#<<<===========change output file name here!

#add include files and libs search path 
INCLUDES_PATH := -I /usr/local/boost/include -L /usr/local/boost/lib

#add libraries files directories 
LIBS := -lboost_system

#compile flags
CXX := g++
CXXFLAGS := -Wall -g -DDEBUG $(INCLUDES_PATH) $(LIBS)

#project directory
PROJECT_DIR := $(shell pwd)

#add src files directories
SOURCES_DIR := $(PROJECT_DIR)/src

#add src files accroding to src files directories
SOURCES := $(wildcard $(SOURCES_DIR)/*.cpp)

############################################################
#		Common actions start from here
###########################################################

OBJS := $(patsubst %.cpp,%.o, $(SOURCES))
DEPS := $(patsubst %.o,%.d, $(OBJS))

all:$(EXECUTABLE)

$(EXECUTABLE):$(DEPS) $(OBJS)
	$(CXX) -o $@ $(OBJS) $(CXXFLAGS)

%.d:%.cpp
	$(CXX) -MM $< > $@
	$(CXX) -MM $< -MQ $@ >> $@

%.o:%.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)	

depend:$(DEPS)
	@echo "Dependencies are now up-to-date."

-include $(DEPS)

.PHONY:clean
clean:
	-rm $(EXECUTABLE)
	-rm $(OBJS)
	-rm $(DEPS)


