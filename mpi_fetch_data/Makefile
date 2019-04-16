all:
	rm -rf test_*
	rm -rf *.gch
	g++  -g -std=gnu++11 common.cc tcp_store.h tcp_store.cc  -o test_tcp_store
	mpicxx -g -std=gnu++11  -o  test_mpi_process  common.cc mpi_processor.h mpi_processor.cc
clean:
	rm -rf test*
	rm -rf *.gch
