.PHONY:clean

test : test.o skip_list.o string_object.o link_list.o nosql_object.o serialization.o deserialization.o db_dict.o 
	gcc $^ -o $@

test.o : test.c
	gcc -c test.c -o test.o

string_object.o : string_object.c
	gcc -c $^ -o $@

nosql_object.o : nosql_object.c
	gcc -c $^ -o $@

skip_list.o : skip_list.c
	gcc -c $^ -o $@

serialization.o : serialization.c
	gcc -c $^ -o $@

deserialization.o : deserialization.c
	gcc -c $^ -o $@

link_list.o : link_list.c
	gcc -c $^ -o $@

db_dict.o : db_dict.c
	gcc -c $^ -o $@

clean :
	rm -f *.o test
