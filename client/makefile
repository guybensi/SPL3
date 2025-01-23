CFLAGS:=-c -Wall -Weffc++ -g -std=c++14 -Iinclude
LDFLAGS:=-lboost_system -lpthread

# Build the StompEMIClient
all: StompEMIClient

StompEMIClient: bin/ConnectionHandler.o bin/StompClient.o bin/StompProtocol.o bin/Frame.o bin/EmergencyEvent.o bin/Event.o bin/StompConnectionHandler.o
	g++ -o bin/StompEMIClient bin/ConnectionHandler.o bin/StompClient.o bin/StompProtocol.o bin/Frame.o bin/EmergencyEvent.o bin/Event.o bin/StompConnectionHandler.o $(LDFLAGS)

bin/ConnectionHandler.o: src/ConnectionHandler.cpp
	g++ $(CFLAGS) -o bin/ConnectionHandler.o src/ConnectionHandler.cpp

bin/StompClient.o: src/StompClient.cpp
	g++ $(CFLAGS) -o bin/StompClient.o src/StompClient.cpp

bin/StompProtocol.o: src/StompProtocol.cpp
	g++ $(CFLAGS) -o bin/StompProtocol.o src/StompProtocol.cpp

bin/Frame.o: src/Frame.cpp
	g++ $(CFLAGS) -o bin/Frame.o src/Frame.cpp

bin/EmergencyEvent.o: src/EmergencyEvent.cpp
	g++ $(CFLAGS) -o bin/EmergencyEvent.o src/EmergencyEvent.cpp

bin/Event.o: src/event.cpp
	g++ $(CFLAGS) -o bin/Event.o src/event.cpp

bin/StompConnectionHandler.o: src/StompConnectionHandler.cpp
	g++ $(CFLAGS) -o bin/StompConnectionHandler.o src/StompConnectionHandler.cpp

.PHONY: clean
clean:
	rm -f bin/*
