CC=gcc
CXX=g++
CFLAGS=-Wall -Werror -O2
CXXFLAGS=${CFLAGS}

SOURCES = ${wildcard *.c *.cc}

default: find

find: ${SOURCES}

testdir: testdir.tar.gz
	-tar -xf testdir.tar.gz 2>/dev/null

.PHONY: test
test: find testdir
	tar -xf testdir.tar.gz
	-./test.e1
	-./test.e2
	-./test.e3
	-./test.r1
	-./test.r2
	-./test.r3
	-./test.r4
	-./test.r5
	-./test.r6
	-./test.r7
	-./test.r8
	-./test.r9
	-./test.r10
	-./test.r11
	-./test.r12
	-./test.r13
	-./test.r14
	-./test.r15
	-./test.r16

clean:
	rm -f *.o find test.*.correct test.*.myoutput
