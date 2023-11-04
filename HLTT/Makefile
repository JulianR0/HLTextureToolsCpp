CXX = g++
CXXFLAGS = -O2
WXCOMP = `wx-config --cxxflags`
WXLINK = `wx-config --libs`

OBJ = HLTT.o WAD3Loader.o SpriteLoader.o

HLTT: ${OBJ}
	${CXX} ${OBJ} -o $@ ${LDFLAGS} ${WXLINK}

clean:
	rm *.o

%.o:	%.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@ $(WXCOMP)
