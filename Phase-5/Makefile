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

PHASE4_SRCS := phase4_demo.cpp \
        models/Paragraph.cpp \
        models/Pitch.cpp \
        services/presentation.cpp
PHASE4_TARGET := phase4_demo

APP_SRCS := main.cpp \
        models/Paragraph.cpp \
        models/Pitch.cpp \
        services/auth.cpp \
        services/database.cpp \
        services/gatekeeper.cpp \
        services/book_core.cpp \
        services/presentation.cpp
APP_TARGET := storyforge

run: $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)
	./$(TARGET)

phase4: $(PHASE4_SRCS)
	$(CXX) $(CXXFLAGS) $(PHASE4_SRCS) -o $(PHASE4_TARGET)
	./$(PHASE4_TARGET)

app: $(APP_SRCS)
	$(CXX) $(CXXFLAGS) $(APP_SRCS) -o $(APP_TARGET)
	./$(APP_TARGET)

clean:
	rm -f $(TARGET) $(PHASE4_TARGET) $(APP_TARGET)
