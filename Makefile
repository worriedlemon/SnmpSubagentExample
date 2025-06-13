CMAKE=cmake
CMAKE_BUILD_DIR=build

load/apt:
	apt install snmp snmpd libsnmp-base libsnmp-dev libsnmp40 g++

configure:
	./configure.sh

build:
	$(CMAKE) -S . -B ${CMAKE_BUILD_DIR} -G "Unix Makefiles" && make -C ${CMAKE_BUILD_DIR}

install: build
	make -C ${CMAKE_BUILD_DIR} install

all:: configure build install

test/snmpget:
	@echo -e \\nTesting SNMPGET...
	snmpget -v2c -c public localhost:8888 1.3.6.1.2.1.25.4.2.1.2.1 

test/snmpwalk:
	@echo -e \\nTesting SNMPWALK...
	snmpwalk -v2c -c public localhost:8888 1.3.6.1.2.1.25 

test/snmptable:
	@echo -e \\nTesting SNMPTABLE...
	snmptable -v2c -c public localhost:8888 1.3.6.1.2.1.25.4.2 -M $(realpath mibs) -m ALL || true
	snmptable -v2c -c public localhost:8888 1.3.6.1.2.1.25.5.1 -M $(realpath mibs) -m ALL || true

test:: test/snmpget test/snmpwalk test/snmptable


.PHONY : configure build install test load/debian
