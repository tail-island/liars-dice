CXXFLAGS = -Ofast -Wall -std=c++17 -march=native

TARGET   = fool
SRCS     = $(shell find . -name *.cpp)
OBJS     = $(SRCS:%.cpp=%.o)
DEPS     = $(SRCS:%.cpp=%.d)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

-include $(DEPS)

$(OBJS): %.o: %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS) -MMD -MP

clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)
