thriftcassandra:
	scons thriftcassandra

agamemnon:
	scons agamemnon

all: thriftcassandra agamemnon

examples: thriftcassandra agamemnon
	scons examples

clean:
	scons --clean

.PHONY: all