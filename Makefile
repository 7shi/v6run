TARGET   = v6run
PREFIX   = /usr/local
CXX      = g++
CXXFLAGS = -Wall -O2 -g
LDFLAGS  = 
STRIP    = strip
OBJECTS  = $(SOURCES:%.cpp=%.o)
SOURCES  = \
	AOut.cpp \
	Operand.cpp \
	VM.cpp \
	VM.inst.cpp \
	VM.signal.cpp \
	VM.sys.cpp \
	main.cpp \
	utils.cpp

all: $(TARGET)

.SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJECTS) *core

install: $(TARGET)
	cp $(TARGET) $(PREFIX)/bin
	$(STRIP) $(PREFIX)/bin/$(TARGET)

depend:
	rm -f dependencies
	for cpp in $(SOURCES); \
	do \
		g++ -MM $(CXXFLAGS) $$cpp >> dependencies; \
	done

-include dependencies
