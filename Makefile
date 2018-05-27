all: db bin bin/common bin/cat_story bin/nav_bar

clean:
	rm -r bin

GCC=g++ -std=c++11 -s

db:
	mkdir -p db/s

bin:
	mkdir bin

bin/cat_story: src/cat_story.cc
	$(GCC) -o bin/cat_story src/cat_story.cc bin/common

bin/nav_bar: src/nav_bar.cc
	$(GCC) -o bin/nav_bar src/nav_bar.cc bin/common

bin/common: src/common.h src/common.cc
	$(GCC) -o bin/common -c src/common.cc
