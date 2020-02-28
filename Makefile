all : memory_manager memory_manager_2

memory_manager : memory_manager.o 
	cc -o memory_manager memory_manager.o

memory_manager_2 : memory_manager_2.o
	cc -o memory_manager_2 memory_manager_2.o

memory_manager.o : memory_manager.c 
	cc -c memory_manager.c
memory_manager_2.o : memory_manager_2.c
	cc -c memory_manager_2.c
clean :
	rm memory_manager memory_manager_2 memory_manager.o memory_manager_2.o
