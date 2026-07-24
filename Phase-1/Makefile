CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Imodels -Iexternal

SRCS := manual_test.cpp models/Paragraph.cpp models/Pitch.cpp
TARGET := manual_test

run: $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
