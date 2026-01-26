CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra
THREADFLAGS = -pthread

TARGET1 = psrs
TARGET2 = psrsparallel

SRC_SEQ = PSRS.cpp \
          KWayMerge.cpp \
          QuickSort.cpp

SRC_PAR = PSRSParallel.cpp \
          KWayMerge.cpp \
          QuickSort.cpp

HDR = PSRS.h \
      KWayMerge.h

OBJ_SEQ = PSRS.o KWayMerge.o QuickSort.o
OBJ_PAR = PSRSParallel.o KWayMerge.o QuickSort.o

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJ_SEQ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TARGET2): $(OBJ_PAR)
	$(CXX) $(CXXFLAGS) $(THREADFLAGS) -o $@ $^

%.o: %.cpp $(HDR)
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJ_SEQ) $(OBJ_PAR) $(TARGET1) $(TARGET2)

run: $(TARGET1)
	./$(TARGET1)

runparallel: $(TARGET2)
	./$(TARGET2)

