CXX = g++
CXXFLAGS = -g -std=c++11
LDFLAGS = -losg -losgDB -losgViewer -losgText
TARGET = visual
SRC = visual.cpp
PREFIX = /usr/local

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

install: $(TARGET)
	install -d $(PREFIX)/bin
	install $(TARGET) $(PREFIX)/bin/

clean:
	rm -f $(TARGET)