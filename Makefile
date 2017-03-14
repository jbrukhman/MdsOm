#
#
#

all:
	make -C C++
	make -C C++/Chains

clean:
	make -C C++ clean
	make -C C++/Chains clean
