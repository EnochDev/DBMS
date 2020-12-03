exce.o: main.cc pml_hash.cc pml_hash.h
	g++ -Wall main.cc pml_hash.cc -o exce.o -lpmem
