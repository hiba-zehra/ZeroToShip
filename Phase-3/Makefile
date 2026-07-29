CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -I. -Imodels -Iservices -Iexternal

SRCS := manual_test.cpp \
        models/Paragraph.cpp \
        models/Pitch.cpp \
        services/auth.cpp \
        services/database.cpp \
        services/gatekeeper.cpp \
        services/book_core.cpp
TARGET := manual_test

run: $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
