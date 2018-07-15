.PHONY:clean
main : nosql_client.o serialization.o deserialization.o link_list.o
	gcc $^ -o $@
nosql_client.o : nosql_client.c
	gcc -c $^ -o $@

serialization.o : ../src/serialization.c
	gcc -c $^ -o $@

deserialization.o : ../src/deserialization.c
	gcc -c $^ -o $@

link_list.o : ../src/link_list.c
	gcc -c $^ -o $@

clean :
	rm -f *.o main
