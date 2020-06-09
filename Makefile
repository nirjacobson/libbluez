MODULES=device	\
				adapter
TEST_MODULES=application \
				     test
OBJECTS=$(foreach MODULE, ${MODULES}, build/${MODULE}.o)
TEST_OBJECTS=$(foreach MODULE, ${TEST_MODULES}, build/${MODULE}.o)
TEST_EXEC=test
PACKAGES = giomm-2.4 sigc++-2.0
CFLAGS   = -std=c++17 -O2 -Wall `pkg-config --cflags ${PACKAGES}` -g
LDFLAGS  = `pkg-config --libs ${PACKAGES}`
LIB			 = bluez
LIB_FILE = lib${LIB}.so

all: ${LIB_FILE}

test: ${LIB_FILE} ${TEST_OBJECTS}
	g++ ${CFLAGS} ${LDFLAGS} ${TEST_OBJECTS} -L. -l${LIB} -o ${TEST_EXEC}

${LIB_FILE}: ${OBJECTS}
	g++ -shared $^ -o ${LIB_FILE} ${LDFLAGS}

format:
	astyle -rnCS *.{h,cc}

build/%.o : builddir src/%.cc
	g++ -c $(word 2, $^) -fpic -o $@ ${CFLAGS}

builddir:
	mkdir -p build

clean:
	rm -rf build
	rm -rf ${LIB_FILE}
	rm -rf ${TEST_EXEC}
