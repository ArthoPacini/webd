make:
	clang++ main.cpp -o main -std=c++20 -O0 -g -lssl -lcrypto -lboost_system -march=native -pthread

#-Wconversion -Wall -Wpedantic -W 