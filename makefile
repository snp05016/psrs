CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra

TARGET = psrs

SRC = PSRS.cpp \
      KWayMerge.cpp \
      QuickSort.cpp

HDR = PSRS.h \
      KWayMerge.h

OBJ = $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp $(HDR)
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET)

