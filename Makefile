BIN=bin/

MOD1=src/m1/
MOD2=src/m2/
MOD3=src/m3/
MOD4=src/m4/

all: moduloUno moduloDos moduloTres moduloCuatro

moduloUno: $(MOD1)cliente.c
	gcc -o $(BIN)cliente $(MOD1)cliente.c $(MOD1)protocolo.c -I.

#moduloDos: $(MOD1)cliente.c
#	gcc -o $(BIN)cliente $(MOD2)cliente.c -I.

#moduloTres: $(SRC)cliente.c
#	gcc -o $(BIN)cliente $(MOD3)cliente.c -I.
	
#moduloCuatro: $(SRC)cliente.c
#	gcc -o $(BIN)cliente $(MOD4)cliente.c -I.

clean:
	rm $(BIN)cliente 
