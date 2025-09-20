all: obe_test

obe_test: src/test.cpp src/OrderBook.cpp
	g++ -std=c++17 -Wall -Iinclude -o obe_test src/test.cpp src/OrderBook.cpp

test: obe_test
	./obe_test

clean:
	rm -f obe_test