makefile: serverf2.c clientf2.c
	gcc -o serverf2 serverf2.c -lpthread
	gcc -o clientf2 clientf2.c
