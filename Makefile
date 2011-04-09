TARGET   = v6run
PREFIX   = /usr/local
CXXFLAGS = -Wall -O2
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
	g++ $(CXXFLAGS) -o $@ -c $<

$(TARGET): $(OBJECTS)
	g++ -s $(CXXFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJECTS)

install: $(TARGET)
	cp $< $(PREFIX)/bin

depend:
	rm -f dependencies
	for cpp in $(SOURCES); \
	do \
		g++ -MM $(CXXFLAGS) $$cpp >> dependencies; \
	done

-include dependencies
