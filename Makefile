all:
	scons

thriftcassandra:
	scons thriftcassandra

agamemnon:
	scons agamemnon

examples:
	scons examples

clean:
	scons --clean

.PHONY: all