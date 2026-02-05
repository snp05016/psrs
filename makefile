CXX = g++
OPTIMIZATION = -O3
CXXFLAGS = -std=c++17 $(OPTIMIZATION) -Wall -Wextra
THREADFLAGS = -pthread
# everything is done in the build directory since i kept running the files and it kept generating a lot of files, 
SRCDIR = src
OBJDIR = build
# this is to have seperate targets so i dont ahve ot recompile both everty time 
TARGET1 = psrs
TARGET2 = psrsparallel

SRC_SEQ = $(SRCDIR)/PSRS.cpp \
          $(SRCDIR)/KWayMerge.cpp \
          $(SRCDIR)/QuickSort.cpp

SRC_PAR = $(SRCDIR)/PSRSParallel.cpp \
          $(SRCDIR)/KWayMerge.cpp \
          $(SRCDIR)/QuickSort.cpp
# hdr is for the header files
HDR = $(SRCDIR)/PSRS.h \
      $(SRCDIR)/KWayMerge.h \
      $(SRCDIR)/QuickSort.h \
      $(SRCDIR)/DataGenerator.h \
      $(SRCDIR)/pthread_barrier.h

OBJ_SEQ = $(OBJDIR)/PSRS.o $(OBJDIR)/KWayMerge.o $(OBJDIR)/QuickSort.o
OBJ_PAR = $(OBJDIR)/PSRSParallel.o $(OBJDIR)/KWayMerge.o $(OBJDIR)/QuickSort.o

all: $(OBJDIR) $(TARGET1) $(TARGET2)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(TARGET1): $(OBJ_SEQ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TARGET2): $(OBJ_PAR)
	$(CXX) $(CXXFLAGS) $(THREADFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(HDR)
	$(CXX) $(CXXFLAGS) -I$(SRCDIR) -c $< -o $@

clean:
	rm -f $(OBJDIR)/*.o $(TARGET1) $(TARGET2)

run: $(TARGET1)
	./$(TARGET1)

runparallel: $(TARGET2)
	./$(TARGET2)

