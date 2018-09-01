.PHONY:all
all:
	g++ -g -std=c++11 -Wall -Werror -pthread game_server.cpp -o game_server
	g++ -g -std=c++11 -Wall -Werror game_client.cpp -o game_client
game_server: 
	g++ -g -std=c++11 -Wall -Werror -pthread game_server.cpp -o game_server
	
game_client:
	g++ -g -std=c++11 -Wall -Werror game_client.cpp -o game_client
clean:
	rm -f game_client game_server
	