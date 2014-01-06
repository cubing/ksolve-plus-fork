
# Javascript options.
JS_OPT = -O2
JS_MEMORY = -s TOTAL_MEMORY=536870912
JS_USE_ASM_JS = 1
JS_FUNCTIONS = "['_solve']"

all: ksolve js

# Use GCC for OpenMP support (parallelization), since clang doesn't support it.
# Need to specify GCC version to avoid triggering clang on OSX. :-(
ksolve:
	g++-4.2 -o ksolve -O3 -fopenmp ./source/main.cpp

.PHONY: js
js:
	emcc -Wall \
		$(JS_OPT) -s \
		ASM_JS=$(JS_USE_ASM_JS) \
		$(JS_MEMORY) \
		-s EXPORTED_FUNCTIONS=$(JS_FUNCTIONS) \
		-o ./html/ksolve.js \
		./source/main.cpp

.PHONY: serve
serve:
	cd html ; open "http://localhost:$${1:-8080}/" ; python -m SimpleHTTPServer $${1:-8080}

.PHONY: cpp-test
cpp-test:
	./ksolve puzzles/3x3x3_RFU.def puzzles/3x3x3_RFU.txt

.PHONY: clean
clean:
	rm -f *.o ksolve ./html/ksolve.js
