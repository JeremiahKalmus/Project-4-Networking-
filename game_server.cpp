//Jeremiah Kalmus
//game_server
#include <iostream> // I/O
#include <string.h>	//strings, stoi
#include <ctype.h>	//isdigit, toupper, isalpha
#include <sys/types.h>	
#include <sys/socket.h>	//Socket
#include <netinet/in.h>	//Bind
#include <arpa/inet.h> //htonl, htons, ntohl, ntohs
#include <pthread.h> //Thread
#include <fstream>
#include <unistd.h> //Close
#include <stdlib.h> //Srand, rand
#include <time.h> //Time
#include <stdio.h> //NULL
#include <iomanip> //setprecision

#define STARTING_SIZE 3
using namespace std;

pthread_mutex_t ldrbrd_mutex;

const int SENDSIZE = 1050;
struct leaderboard{
	char first_name[SENDSIZE];
	char second_name[SENDSIZE];
	char third_name[SENDSIZE];
	float first_score;
	float second_score;
	float third_score;
};
leaderboard board;

void* game_thread(void *);

bool send_to(int , char[]);

bool receive_from(int , char[]);

void leader_board(char [], float);

int main(int argc, char* argv[])
{
	const int UPPER = 16849;
	const int LOWER = 16800;
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;
	ldrbrd_mutex = PTHREAD_MUTEX_INITIALIZER;
	
	int sockfd, newsockfd = 0;
	
	board.first_score = 0.0;
	board.second_score = 0.0;
	board.third_score = 0.0;
	
	if((argc < 2) || (argc > 2)){
		cerr << "ERROR, not the proper amount of arguments!" << endl;
		return -1;
	}
	else{
		string temp = argv[1];
		for(unsigned int i = 0;i < temp.length();i++){
			if(!isdigit(temp[i])){
				cerr << "ERROR, not a valid port number!" << endl;
				return -1;
			}			
		}
		int port = stoi(temp);
		if((port > UPPER) || (port < LOWER)){
			cerr << "ERROR, not a valid port number!" << endl;
			return -1;
		}
		
		if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
			cerr << "ERROR, socket failed!" << endl;
			return -1;
		}
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr.sin_port = htons(port);
		
		if(bind(sockfd, (struct sockaddr *)&serv_addr, 
                                 sizeof(serv_addr))<0)
		{
			cerr << "ERROR, binding failed!" << endl;
			return -1;
		}
		if(listen(sockfd, 5) != 0){
			cerr << "ERROR, failed to etablish listening socket for clients!" << endl;
			return -1;
		}	
		
		cout << "Listening..." << endl;
		//Loop continues forever until program is aborted manually
		while(1){
			clilen = sizeof(cli_addr);
			newsockfd = accept(sockfd, 
					(struct sockaddr *) &cli_addr, 
					&clilen);
			if (newsockfd < 0){
				cerr << "ERROR, failed to accept client!" << endl;
			}
			
			pthread_t game_t;
			if(0 != pthread_create(&game_t, NULL, game_thread,
				(void *) &newsockfd)){
				cerr << "ERROR, failed to create thread!" << endl;
				return -1;
			}
		}
	}
	close(sockfd);
	return 0;
}
void* game_thread(void *client)
{
	pthread_detach(pthread_self());
	
	int file_index, result, num_guess, index = 0;
	int done = 0;
	float score = 0.0;
	unsigned int incorrect_count, word_length = 0;
	const int FILESIZE = 57489;
	int newsockfd = *(int*) client;
	char buffer[SENDSIZE];
	char lett;
	char client_name[SENDSIZE];
	string line, word, message, size;
	string word_list[FILESIZE];
	srand(time(NULL));
	const string FILE = "/home/fac/lillethd/cpsc3500/projects/p4/words.txt";
	cout<<"Client connection established"<<endl;
	
	//Opening file containing words for the game
	ifstream infile;
	infile.open(FILE);
	if(infile.fail()){
		cerr << "ERROR opening file!" << endl;    
		return NULL;
	}
	
	//Filling array with words from file
	for(int i = 0;i < FILESIZE;i++){
		getline(infile, line);
		word_list[i] = line; 
	}
	file_index = (rand() % FILESIZE);
	
	//Receiving name from client
	if(receive_from(newsockfd, buffer) == false){
		cerr << "Lost connection" << endl;
		return NULL;
	}
	//client_name = buffer;
	strcpy(client_name,buffer);
	
	//Loading string into buffer
	memset(buffer,0,SENDSIZE);
	word = word_list[file_index];
	word_length = word.length();
	cout << word << endl;
	sprintf(buffer,"%d" ,word_length);

	//Sending the word length in buffer array to client
	if(send_to(newsockfd, buffer) == false){
		cerr << "Lost connection" << endl;
		return NULL;
	}
	
	char *correct_guesses = new char[word.length()];
	
	
	for(unsigned int i = 0;i < word.length();i++){
		correct_guesses[i] = 0;
	}
	char *arr = new char[word_length+1];
	for(unsigned int i = 0;i < word_length;i++){
		arr[i] = '-';
		
	}
	arr[word_length] = '\0';
	done = word.length();
	while(done != 0){
		
		message = "Correct!";
		
		//send hidden word to client
		memset(buffer,0,SENDSIZE);
		strcpy(buffer, arr);
	if(send_to(newsockfd, buffer) == false){
		cerr << "Lost connection" << endl;
		return NULL;
	}
		
		//Receiving the clients guessed letter
	if(receive_from(newsockfd, buffer) == false){
		cerr << "Lost connection" << endl;
		return NULL;
	}
		lett = buffer[0];
		size = buffer;
		if(size.length() > 1){
			message = "Please only guess one character at a time!";
			incorrect_count = word.length();
		}
		
		//Checking the guess to see if it is correct
		if(isalpha(lett) == 0){
			message = "Incorrect!";
			incorrect_count = word.length();
		}
		//Checking if alphabetical
		lett = toupper(lett);
		for(unsigned int i = 0;i < word.length();i++){
			if(correct_guesses[i] == lett){
				message = "Already guessed this letter correctly!";
				incorrect_count = word.length();
			}
		}
		//If the letter guess was correct then replace all proper dashes
		//the correct character
		if(message == "Correct!"){
			incorrect_count = 0;
			for(unsigned int i = 0;i < word_length;i++){
				if(word[i] == lett){
					arr[i] = lett;
				}
			}
			for(unsigned int i = 0;i < word.length();i++){
				if(word[i] != lett){
					incorrect_count++;
				}			
			}
			if(incorrect_count == word.length()){
				message = "Incorrect!";
			}
			if(message == "Correct!"){
				while(correct_guesses[index] != 0){
					index++;
				}
				correct_guesses[index] = lett;
				index = 0;
			}
		}
		result = word.length() - incorrect_count;
		done = done - result;
		
		//If done, tell the client to stap loop.
		if(done == 0){
			message = "DONE!";
		}
		//Sending the outcome of the client's guess
		memset(buffer,0,SENDSIZE);
		for(unsigned int i = 0;i < message.length();i++){
			buffer[i] = message[i];
		}
		if(send_to(newsockfd, buffer) == false){
			cerr << "Lost connection" << endl;
			return NULL;
		}
	
		
		incorrect_count = 0;
	}
	
	//Sending original word
	memset(buffer,0,SENDSIZE);
	for(unsigned int i = 0;i < word.length();i++){
		buffer[i] = word[i];
	}
	if(send_to(newsockfd, buffer) == false){
		cerr << "Lost connection" << endl;
		return NULL;
	}
	
	//Update leaderboard and send it to client
	//receive total number of guesses
	if(receive_from(newsockfd, buffer) == false){
		cerr << "Lost connection" << endl;
		return NULL;
	}
	num_guess = atof(buffer);
	score = (float)num_guess / (float)word.length();
	
	//update leaderboard
	if(0 != pthread_mutex_lock(&ldrbrd_mutex)){
		cerr << "ERROR fail to lock thread!" << endl;
		return NULL;
	}
	leader_board(client_name, score);

	if(0 != pthread_mutex_unlock(&ldrbrd_mutex)){
		cerr << "ERROR fail to unlock thread!" << endl;
		return NULL;
	}
		memset(buffer,0,SENDSIZE);
		//Load leaderboard into the buffer 
		sprintf(buffer,"%sLeaderboard: \n1: %s %.2f\n",buffer, board.first_name, board.first_score);
		if(send_to(newsockfd, buffer) == false){
			cerr << "Lost connection" << endl;
			return NULL;
		}
		
		memset(buffer,0,SENDSIZE);
		if(board.second_score != 0){
			sprintf(buffer,"%s2: %s %.2f\n",buffer, board.second_name, board.second_score);
		}
		if(send_to(newsockfd, buffer) == false){
			cerr << "Lost connection" << endl;
			return NULL;
		}
		memset(buffer,0,SENDSIZE);
		if(board.third_score != 0){
			sprintf(buffer,"%s3: %s %.2f\n",buffer, board.third_name, board.third_score);
		}
		if(send_to(newsockfd, buffer) == false){
			cerr << "Lost connection" << endl;
			return NULL;
		}
	
	close(newsockfd);
	delete[] correct_guesses;
	delete[] arr;
	return NULL;
}
bool send_to(int newsockfd, char buffer[])
{
	int num = 0;
	while(num < SENDSIZE){
		num = send(newsockfd,buffer,SENDSIZE,0);
		if (num < 0){
			cerr << "ERROR sending to socket" << endl;
			return false;
		}
	}
	return true;
}
bool receive_from(int newsockfd, char buffer[])
{
	int num = 0;
	memset(buffer,0,SENDSIZE);
	while(num < SENDSIZE){		
		num = recv(newsockfd,buffer,SENDSIZE,0);
		if (num <= 0) {
			cerr << "ERROR receiving from socket" << endl;
			return false;
		}
	}
	return true;
}
void leader_board(char player_name[], float player_score)
{
	if((player_score < board.first_score) || (board.first_score == 0)){
		strcpy(board.third_name,board.second_name);
		board.third_score = board.second_score;
		strcpy(board.second_name,board.first_name);
		board.second_score = board.first_score;
		strcpy(board.first_name,player_name);
		board.first_score = player_score;
	}
	else if((player_score < board.second_score) || (board.second_score == 0)){
		strcpy(board.third_name,board.second_name);
		board.third_score = board.second_score;
		strcpy(board.second_name,player_name);
		board.second_score = player_score;
	}
	else if((player_score < board.third_score) || (board.third_score == 0)){
		strcpy(board.third_name,player_name);
		board.third_score = player_score;
	}
}