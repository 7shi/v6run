TARGET   = v6run
PREFIX   = /usr/local
V6ROOT   = $(PREFIX)/v6root
CXX      = g++
CXXFLAGS = -Wall -O2 -g
LDFLAGS  = 
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
	rm -f $(TARGET) $(TARGET).exe $(OBJECTS) *core

install: $(TARGET)
	mkdir -p $(PREFIX)/bin
	install -cs $(TARGET) $(PREFIX)/bin
	for cmd in ar as cc ld nm strip; do \
	  sh mkwrap.sh $(TARGET) $(V6ROOT) $(PREFIX)/bin/v6 $$cmd; done

uninstall:
	cd $(PREFIX)/bin && rm -f $(TARGET) $(TARGET).exe v6ar v6as v6cc v6ld v6nm v6strip

depend:
	rm -f dependencies
	for cpp in $(SOURCES); \
	do \
		g++ -MM $(CXXFLAGS) $$cpp >> dependencies; \
	done

-include dependencies
