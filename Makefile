all:
	gcc  -m32 -no-pie -nostdlib -o fib fib.c
	gcc  -m32 -no-pie -nostdlib -o sum sum.c
	gcc -m32 -o loader loaderbonus.c
	gcc -m32 -o loader2 loader.c


clean:
	-@rm -f fib loader
