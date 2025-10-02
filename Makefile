CXX = g++
CXXFLAGS = -g -std=c++11 -I.
LDFLAGS = -losg -losgDB -losgViewer -losgText -losgGA
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