MODULES=device		\
				adapter		\
				application
OBJECTS=$(foreach MODULE, ${MODULES}, build/${MODULE}.o)
PACKAGES = giomm-2.4 sigc++-2.0
CFLAGS   = -std=c++17 -O2 -Wall `pkg-config --cflags ${PACKAGES}` -g
LDFLAGS  = `pkg-config --libs ${PACKAGES}`
LIB			 = bluez

all: build ${LIB}

test: build ${LIB}
	g++ ${CFLAGS} ${LDFLAGS} test.cc -L. -l${LIB} -o test

${LIB}: ${OBJECTS}
	g++ -shared $^ -o lib$@.so ${LDFLAGS}

build/%.o : src/%.cc
	g++ -c $< -fpic -o $@ ${CFLAGS}

build:
	mkdir -p build

clean:
	rm -rf build
	rm -rf lib${LIB}.so
	rm -rf test
