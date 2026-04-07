MODULES      = adapter device
TEST_MODULES = application test
OBJECTS      = $(foreach MODULE, ${MODULES}, build/${MODULE}.o)
TEST_OBJECTS = $(foreach MODULE, ${TEST_MODULES}, build/${MODULE}.o)
TEST_EXEC    = test
PACKAGES     = giomm-2.68 sigc++-3.0
CFLAGS       = -std=c++17 -O2 -Wall `pkg-config --cflags ${PACKAGES}` -g
LDFLAGS      = `pkg-config --libs ${PACKAGES}`
LIB			 = bluez
LIB_FILE     = lib${LIB}.so
LIB_DIR      = /usr/lib64
INCLUDE_DIR  = /usr/include/${LIB}

.PHONY: docs

all: build/ ${LIB_FILE}

docs:
	rm -rf docs/
	doxygen doxygen.txt
	
install: ${LIB_FILE}
	install -D $< -t ${DESTDIR}${LIB_DIR}
	install -D src/*.h -t ${DESTDIR}${INCLUDE_DIR}

test: build/ ${LIB_FILE} ${TEST_OBJECTS}
	g++ ${CFLAGS} ${LDFLAGS} ${TEST_OBJECTS} -L. -l${LIB} -o ${TEST_EXEC}

${LIB_FILE}: ${OBJECTS}
	g++ -shared $^ -o ${LIB_FILE} ${LDFLAGS}

format:
	astyle -rnNCS *.{h,cc}

build/%.o : src/%.cc
	g++ -c $< -fpic -o $@ ${CFLAGS}

build/:
	mkdir -p build

clean:
	rm -rf build
	rm -f ${LIB_FILE}
	rm -f ${TEST_EXEC}

distclean:
	rm -f /usr/lib/${LIB_FILE}
	rm -rf ${INCLUDE_DIR}
