BOOST_PATH=/data0/eherman/boost/boost_1_64_0

CXX=g++ -std=c++11 -g -Wall -Wextra -pedantic
CXX_FLAGS += -I$(BOOST_PATH)

boost-test: boost-test.cpp
	$(CXX) $(CXX_FLAGS) -o boost-test boost-test.cpp

check: boost-test
	./boost-test

clean:
	rm -vf boost-test *~
