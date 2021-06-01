CC := g++
DIR_HEADER := ./header
DIR_SOURCE := ./source
CFLAGS := -Wall -c -I ./header -O2
TARGET := main

${TARGET}:main.o data_structure.o graph.o
	${CC} -o ${TARGET} main.o data_structure.o graph.o -Wall

main.o:${DIR_SOURCE}/main.cpp ${DIR_HEADER}/graph.hpp
	${CC} ${CFLAGS} ${DIR_SOURCE}/main.cpp

graph.o:${DIR_SOURCE}/graph.cpp ${DIR_HEADER}/graph.hpp ${DIR_HEADER}/data_structure.hpp
	${CC} ${CFLAGS} ${DIR_SOURCE}/graph.cpp

data_structure.o:${DIR_SOURCE}/data_structure.cpp ${DIR_HEADER}/data_structure.hpp
	${CC} ${CFLAGS} ${DIR_SOURCE}/data_structure.cpp


clean:
	 rm -rf *.o
