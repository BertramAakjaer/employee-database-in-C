TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

run: clean default
	./$(TARGET) -n -f ./mydb.db
	./$(TARGET) -f ./mydb.db -a "Timmy A.,Sheshire Ln.,230"

default: $(TARGET)

clean:
	rm -f obj/*.o
	rm -f bin/*

$(TARGET): $(OBJ)
	gcc -std=c11 -o $@ $?

obj/%.o : src/%.c
	gcc -std=c11 -c $< -o $@ -Iinclude


