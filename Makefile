CC := g++ -std=c++11
DIR_HEADER := ./header
DIR_SOURCE := ./source
CFLAGS :=  -g -c -I ./header -O2
TARGET := main


LIBSRC += $(wildcard ${DIR_SOURCE}/*.cpp)
LIBOBJ = $(patsubst %.cpp,%.o,${LIBSRC})





all : $(LIBOBJ) main.cpp
	${CC} -o main.exe $^


TEST-ROUTING : $(LIBOBJ) ROUTINGTEST.cpp
	${CC} -o ROUTINGTEST.exe $^

#之後在想一下怎麼自動檢查對應的.hpp , 就可以改成%.o :%.hpp , 但我現在還沒想到
graph.o:${DIR_SOURCE}/graph.cpp ${DIR_HEADER}/graph.hpp ${DIR_HEADER}/data_structure.hpp
	${CC} ${CFLAGS} ${DIR_SOURCE}/graph.cpp ${DIR_SOURCE}/graph.o

data_structure.o:${DIR_SOURCE}/data_structure.cpp ${DIR_HEADER}/data_structure.hpp
	${CC} ${CFLAGS} ${DIR_SOURCE}/data_structure.cpp ${DIR_SOURCE}/data_structure.o

Routing.o:${DIR_SOURCE}/Routing.cpp ${DIR_HEADER}/data_structure.hpp ${DIR_HEADER}/graph.hpp
	${CC} ${CFLAGS} ${DIR_SOURCE}/Routing.cpp  ${DIR_SOURCE}/Routing.o


clean:
	 rm -rf $(LIBOBJ)
