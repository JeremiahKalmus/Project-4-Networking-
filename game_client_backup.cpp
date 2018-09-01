//Jeremiah Kalmus
//game_client
#include <iostream> // I/O
#include <string.h>
#include <ctype.h>	//isdigit
#include <sys/types.h>	
#include <sys/socket.h>	//socket
#include <netinet/in.h>	//bind
#include <netdb.h> 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

bool send_to(int, char[]);

bool receive_from(int, char[]);

const int SENDSIZE = 1050;
int main(int argc, char* argv[])
{
	const int UPPER = 16849;
	const int LOWER = 16800;
	const int SENDSIZE = 1050;
	const string IP = "10.124.72.20";
	struct sockaddr_in serv_addr;
	struct hostent *server;
	int sockfd, turn_counter = 0;
	string correct = "Correct!";
	string incorrect = "Incorrect!";
	string done = "DONE!";
	string hidden_word, receive_string;
	char buffer[SENDSIZE];
	if((argc < 3) || (argc > 3)){
		cerr << "ERROR, not the proper amount of arguments" << endl;
		return -1;
	}
	if(argv[1] != IP){
		cerr << "ERROR, invalid IP" << endl;
		return -1;
	}
	server = gethostbyname(argv[1]);
	string temp = argv[2];
	for(unsigned int i = 0;i < temp.length();i++){
		if(!isdigit(temp[i])){
			cerr << "ERROR, not a valid port number" << endl;
			return -1;
		}			
	}
	int port = stoi(temp);
	if((port > UPPER) || (port < LOWER)){
		cerr << "ERROR, not a valid port number" << endl;
		return -1;
	}
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
			cerr << "ERROR, socket failed" << endl;
			return -1;
	}

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(port);
	if(connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) != 0){
		cerr << "ERROR, connection to host failed" << endl;
		return -1;
	}
	cout << "Welcome to Hangman!" << endl;
	cout << "Enter your name: ";
    memset(buffer,0,SENDSIZE);
    fgets(buffer,SENDSIZE,stdin);
	
	//Sends player name to server
	if(send_to(sockfd, buffer) == false){
		cerr << "Lost connection" << endl;
		return -1;
	}
	
	//Receives word size from server
	if(receive_from(sockfd, buffer) == false){
		cerr << "Lost connection" << endl;
		return -1;
	}
	receive_string = buffer;
	//word_length = stoi(receive_string);
	
	while(1){

		turn_counter++;
		//bzero(buffer,SENDSIZE);

		//Receive hidden word
	if(receive_from(sockfd, buffer) == false){
		cerr << "Lost connection" << endl;
		return -1;
	}
		hidden_word = buffer;
		
		cout << "Turn " << turn_counter << endl;
		cout << "Word: " << hidden_word << endl;
		cout << "Enter your guess: ";
		cin >> buffer;
		cout << endl;
		
		//Send server the guess
	if(send_to(sockfd, buffer) == false){
		cerr << "Lost connection" << endl;
		return -1;
	}
		//Receive the evaluation of the guess
	if(receive_from(sockfd, buffer) == false){
		cerr << "Lost connection" << endl;
		return -1;
	}
		//receive_string = buffer;
		if(buffer == done){
			break;
		}
		cout << buffer << endl << endl;
		
		if((buffer != correct) && (buffer != incorrect)){
			turn_counter--;
		}

	}
	//Receive full word from server
	if(receive_from(sockfd, buffer) == false){
		cerr << "Lost connection" << endl;
		return -1;
	}
	cout << "Congratulations! You guessed the word " << buffer << "!!" 
		 << endl;
	cout << "It took " << turn_counter << " turns to guess the word correctly."
		 << endl << endl;
	
	memset(buffer,0,SENDSIZE);
	sprintf(buffer,"%d",turn_counter);
	
	//Sending number of guesses to server
	if(send_to(sockfd, buffer) == false){
		cerr << "Lost connection" << endl;
		return -1;
	}
	
	//Receiveing leaderboard data from server
	if(receive_from(sockfd, buffer) == false){
		cerr << "Lost connection" << endl;
		return -1;
	}
	cout << buffer << endl;
	
	close(sockfd);
	return 0;
}
bool send_to(int sockfd, char buffer[])
{
	int num = 0;
	
	while(num < SENDSIZE){
		
		num = send(sockfd,buffer,SENDSIZE,0);
		if (num < 0){
			cerr << "ERROR sending to socket" << endl;
			return false;
		}
	}
	return true;
}

bool receive_from(int sockfd, char buffer[])
{
	int num = 0;
	memset(buffer,0,SENDSIZE);
	while(num < SENDSIZE){
		num = recv(sockfd,buffer,SENDSIZE,0);
		if (num <= 0){
			cerr << "ERROR receiving from socket" << endl;
			return false;
		}
	}
	return true;
}