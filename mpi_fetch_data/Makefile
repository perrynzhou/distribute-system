all:
	rm -rf test_*
	rm -rf *.gch
	gcc  -g -std=gnu99 common.c tcp_store.h tcp_store.c  -o test_tcp_store
	mpicc -g -std=gnu99  -o  test_mpi_process  common.c mpi_processor.h mpi_processor.c
clean:
	rm -rf test*
	rm -rf *.gch
