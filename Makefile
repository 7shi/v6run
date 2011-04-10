TARGET   = v6run
PREFIX   = /usr/local
V6ROOT   = $(PREFIX)/v6root
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
	sh mkwrap.sh $(TARGET) $(V6ROOT) $(PREFIX)/bin/v6ar ar
	sh mkwrap.sh $(TARGET) $(V6ROOT) $(PREFIX)/bin/v6as as
	sh mkwrap.sh $(TARGET) $(V6ROOT) $(PREFIX)/bin/v6cc cc
	sh mkwrap.sh $(TARGET) $(V6ROOT) $(PREFIX)/bin/v6ld ld
	sh mkwrap.sh $(TARGET) $(V6ROOT) $(PREFIX)/bin/v6strip strip

uninstall:
	cd $(PREFIX)/bin && rm -f $(TARGET) v6ar v6as v6cc v6ld v6strip

depend:
	rm -f dependencies
	for cpp in $(SOURCES); \
	do \
		g++ -MM $(CXXFLAGS) $$cpp >> dependencies; \
	done

-include dependencies
