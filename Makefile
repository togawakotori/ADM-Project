prime7b : prime7b.c
	mpicc prime7b.c -o prime7b -lm

clean : 
	rm prime7b 
