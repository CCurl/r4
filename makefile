app := r4

CXX := clang
CXXFLAGS := -m64 -O3 -D __LINUX__

srcfiles := $(shell find . -name "*.cpp")
incfiles := $(shell find . -name "*.h")
LDLIBS   := -lm

all: $(app)

$(app): $(srcfiles) $(incfiles)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(app) $(srcfiles) $(LDLIBS)
	ls -l $(app)

clean:
	rm -f $(app)

rebuild: clean all

bin: $(app)
	cp -u -p $(app) ~/bin/
