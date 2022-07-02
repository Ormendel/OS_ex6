CC=gcc
CCPP= g++
FLAGS=-pthread -fPIC
OBJECTS=guard.o reactor.o singleton.o

all: main1 client guardtest singleton lib.so

# Questions 1-3

main1: main1.o 
	$(CC) $(FLAGS) main1.o -o main1


main1.o: main1.c 
	$(CC) $(FLAGS) -c main1.c

client: client.o 
	$(CC) $(FLAGS) client.o -o client


client.o: client.c 
	$(CC) $(FLAGS) -c client.c
	

# Question 4-6

guard.o: guard.cpp 
	$(CCPP) $(FLAGS) -c guard.cpp 
	
guardtest.o: guardtest.cpp 
	$(CCPP) $(FLAGS) -c guardtest.cpp 

guardtest: guard.o guardtest.o
	$(CCPP) $(FLAGS) guardtest.o guard.o -o guardtest
	
singleton.o: singleton.cpp 
	$(CCPP) $(FLAGS) -c singleton.cpp 

singleton: singleton.o
	$(CCPP) $(FLAGS) singleton.o -o singleton
	
reactor.o: reactor.cpp 
	$(CCPP) $(FLAGS) -c reactor.cpp 	

### Shared library
lib.so: $(OBJECTS)
	$(CC) --shared -fPIC -pthread $(OBJECTS) -o lib.so

.PHONY: clean all

clean:
	rm -f *.o *.so main1 client guardtest singleton