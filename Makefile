TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

run: clean default
	./$(TARGET) -n -f ./mydb.db
	./$(TARGET) -f ./mydb.db

default: $(TARGET)

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db

$(TARGET): $(OBJ)
	gcc -std=c11 -o $@ $?

obj/%.o : src/%.c
	gcc -std=c11 -c $< -o $@ -Iinclude


