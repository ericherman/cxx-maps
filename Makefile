BOOST_PATH=/data0/eherman/boost/boost_1_64_0

CXX=g++ -std=c++11 -g -Wall -Wextra -pedantic
CXX_FLAGS += -I$(BOOST_PATH)


# $@ : target label
# $< : the first prerequisite after the colon
# $^ : all of the prerequisite files
# $* : wildcard matched part

boost-test: boost-test.cpp
	$(CXX) $(CXX_FLAGS) -o $@ $^

check: boost-test
	./$<

clean:
	rm -vf boost-test *~
