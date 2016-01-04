BIN=bin/

MOD1=src/m1/
SRCM1=$(shell find src/m1/ -name *.c)
MOD2=src/m2/
SRCM2=$(shell find src/m2/ -name *.c)
MOD3=src/m3/
SRCM3=$(shell find src/m3/ -name *.c)
MOD4=src/m4/
SRCM4=$(shell find src/m4/ -name *.c)

all: moduloUno moduloDos moduloTres moduloCuatro

moduloUno: $(MOD1)cliente.c
	gcc -o $(BIN)cliente $(SRCM1) -I.

#moduloDos: $(MOD2)cliente.c
#	gcc -o $(BIN)cliente $(SRCM2) -I.

#moduloTres: $(MOD3)cliente.c
#	gcc -o $(BIN)cliente $(SRCM3) -I.
	
#moduloCuatro: $(MOD4)cliente.c
#	gcc -o $(BIN)cliente $(SRCM4) -I.

clean:
	rm $(BIN)cliente 
