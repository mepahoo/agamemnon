thriftcassandra:
	scons thriftcassandra

agamemnon:
	scons agamemnon

all: thriftcassandra agamemnon

clean:
	scons --clean

.PHONY: all