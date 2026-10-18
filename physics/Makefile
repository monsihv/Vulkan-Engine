make build:
	clang -std=c23 -fenable-matrix main.c -o main

run:
	./main | awk -F'[:,] *' '{print $$2, $$4, $$6}' > data.dat

plot:
	gnuplot plot.gp && open plot.png

all:
	make build make run make plot
