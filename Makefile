CXX = g++
CXXFLAGS = -std=c++2a -Wall -I h -I /usr/local/include/gtest/ -O3 -c
CLLFLAGS = -std=c++2a -Wall -I h -I /usr/local/include/gtest/ -O3 -fPIC -shared
LXXFLAGS = -std=c++2a -I h -pthread -ltbb -lpuzzle -L./obj -Wl,-rpath=./obj
OBJECTS = ./obj/main.o
LIBS = ./obj/libpuzzle.dll
TARGET = main



$(TARGET): $(OBJECTS) $(LIBS)
	$(CXX) $(LXXFLAGS) -o $(TARGET) $(OBJECTS)
./obj/main.o: ./cpp/main.cpp ./h/puzzle.h
	$(CXX) $(CXXFLAGS) ./cpp/main.cpp -o ./obj/main.o
./obj/libpuzzle.dll: ./cpp/puzzle.cpp
	$(CXX) $(CLLFLAGS) ./cpp/puzzle.cpp -o ./obj/libpuzzle.dll
clean:
	rm -fv $(TARGET) $(OBJECTS) $(LIBS)
