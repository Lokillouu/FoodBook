CXX=g++
CXXFLAGS= -std=c++20 -Wall

FoodBook: all
	$(CXX) $(CXXFLAGS) -o main main.o user.o food.o filemanager.o io_fb.o errors.o date.o
all:
	$(CXX) $(CXXFLAGS) -c main.cpp src/user/user.cpp src/food/food.cpp src/filemanager/filemanager.cpp src/io/io_fb.cpp src/errors/errors.cpp src/date/date.cpp