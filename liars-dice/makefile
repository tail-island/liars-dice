CXXFLAGS = -Ofast -Wall -std=c++17 -march=native -pthread -lboost_filesystem -lboost_system

TARGET   = liars-dice
SRCS     = $(shell find . -maxdepth 1 -name *.cpp)
OBJS     = $(SRCS:%.cpp=%.o)
DEPS     = $(SRCS:%.cpp=%.d)
SUBS     = fool hardhead optimist pessimist smart timid yours

all: $(TARGET) $(SUBS)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

-include $(DEPS)

$(OBJS): %.o: %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS) -MMD -MP

$(SUBS): FORCE
	$(MAKE) -C $@ $(MAKECMDGOALS)

FORCE:

clean: $(SUBS)
	$(RM) $(TARGET) $(OBJS) $(DEPS)
