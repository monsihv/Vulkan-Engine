make build:
	clang++ -std=c++20 -I/opt/homebrew/opt/glm/include main.cpp -o main

run:
	./main | awk -F'[:,] *' '{print $$2, $$4, $$6}' > data.dat

plot:
	gnuplot plot.gp && open plot.png

all:
	make build make run make plot
