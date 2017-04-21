all: db bin/cat_story

GCC=g++ -std=c++11 -s

db:
	mkdir -p db/s

bin:
	mkdir bin

bin/cat_story: bin src/cat_story.cc
	$(GCC) -o bin/cat_story src/cat_story.cc
