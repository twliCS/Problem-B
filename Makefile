CXX := g++
DIR_HEADER := ./header
DIR_SOURCE := ./source
CXXFLAGS := -std=c++11 -g -c -I ./header -O2
TARGET := main


LIBSRC += $(wildcard ${DIR_SOURCE}/*.cpp)
LIBOBJ = $(patsubst %.cpp,%.o,${LIBSRC})





all : $(LIBOBJ) main.cpp
	${CXX} -o main.exe $^


TEST-ROUTING : $(LIBOBJ) ROUTINGTEST.cpp
	${CXX} -o ROUTINGTEST.exe $^

#之後在想一下怎麼自動檢查對應的.hpp , 就可以改成%.o :%.hpp , 但我現在還沒想到
${DIR_SOURCE}/graph.o:${DIR_SOURCE}/graph.cpp ${DIR_HEADER}/graph.hpp ${DIR_HEADER}/data_structure.hpp
	${CXX} ${CXXFLAGS} ${DIR_SOURCE}/graph.cpp -o ${DIR_SOURCE}/graph.o

${DIR_SOURCE}/data_structure.o:${DIR_SOURCE}/data_structure.cpp ${DIR_HEADER}/data_structure.hpp
	${CXX} ${CXXFLAGS} ${DIR_SOURCE}/data_structure.cpp -o ${DIR_SOURCE}/data_structure.o

${DIR_SOURCE}/Routing.o:${DIR_SOURCE}/Routing.cpp ${DIR_HEADER}/data_structure.hpp ${DIR_HEADER}/graph.hpp
	${CXX} ${CXXFLAGS} ${DIR_SOURCE}/Routing.cpp -o ${DIR_SOURCE}/Routing.o

.PHONY: clean
clean:
	$(RM) $(LIBOBJ)
