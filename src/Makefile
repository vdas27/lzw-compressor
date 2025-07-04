HEADERS = decode.h encode.h
OBJECTS = program.o decode.o encode.o

default: program

program.o: main.c $(HEADERS)
	gcc -c main.c -o program.o

decode.o: decode.c decode.h
	gcc -c decode.c -o decode.o

encode.o: encode.c encode.h
	gcc -c encode.c -o encode.o

program: $(OBJECTS)
	touch encode
	touch decode
	rm -f decode
	rm -f encode
	
	gcc $(OBJECTS) -o program

	ln -s program decode
	ln -s program encode

clean:
	-rm -f $(OBJECTS)
	-rm -f program
	-rm -f decode
	-rm -f encode