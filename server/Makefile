.PHONY:clean

H_PATH = ../src/

main : nosql_server.o link_list.o db_dict.o deserialization.o serialization.o nosql_object.o string_object.o skip_list.o
	gcc $^ -o $@

nosql_server.o : nosql_server.c
	gcc -c $^ -o $@

deserialization.o : $(H_PATH)deserialization.c
	gcc -c $^ -o $@

link_list.o : $(H_PATH)link_list.c
	gcc -c $^ -o $@

serialization.o : $(H_PATH)serialization.c
	gcc -c $^ -o $@

db_dict.o : $(H_PATH)db_dict.c
	gcc -c $^ -o $@

nosql_object.o : ../src/nosql_object.c
	gcc -c $^ -o $@

string_object.o : $(H_PATH)string_object.c
	gcc -c $^ -o $@

skip_list.o : $(H_PATH)skip_list.c
	gcc -c $^ -o $@


clean:
	rm -f *.o


