//============================================================================
// Name        : lab8.cpp
// Author      : Jacob Dorpinghaus
//============================================================================

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>

#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

class message{
public:
	message();
	message(string messageString);
	virtual ~message();
	virtual void print();
protected:
	string messageText;
};

class morseCodeMessage: public message{
public:
	morseCodeMessage();
	morseCodeMessage(string messageString);
	~morseCodeMessage();
	virtual void print();
	void morseCodeToLights(unsigned long* PBDR, unsigned long* PBDDR);
private:
	void translate();
	string messageCode;
};

class messageStack{
public:
	messageStack(message* firstMessage);
	~messageStack();
	void push(message* messageObject);
	message* pop(void);
	void printStack(void);
private:
	vector <message*> stack;
};



int main(int argc, char **argv) {
	int fd;		// for the file descriptor of the special file we need to open.
	unsigned long *BasePtr;		// base pointer, for the beginning of the memory page (mmap)
	unsigned long *PBDR, *PBDDR;	// pointers for port B DR/DDR

	fd = open("/dev/mem", O_RDWR|O_SYNC);	// open the special file /dem/mem
	if(fd == -1){
		printf("\n error\n");
		return(-1);  // failed open
	}

	BasePtr = (unsigned long*)mmap(NULL,getpagesize(),PROT_READ|PROT_WRITE,MAP_SHARED,fd,0x80840000);
	if(BasePtr == MAP_FAILED){
		printf("\n Unable to map memory space \n");
		return(-2);
	}  // failed mmap

	PBDR = BasePtr + 1;		// Address of port B DR is 0x80840004
	PBDDR = BasePtr + 5;	// Address of port B DDR is 0x80840014

	*PBDDR |= 0xE0;		// configures port B7,B6,B5 as outputs
	*PBDR |= 0x80;	// ON: write a 1 to port B7. Mask all other bits.
	usleep(500000);
	*PBDR |= 0x40;	// ON: write a 1 to port B6. Mask all other bits.
	usleep(500000);
	*PBDR |= 0x20;	// ON: write a 1 to port B5. Mask all other bits.
	usleep(500000);
	*PBDR &= ~0xE0;	// OFF: write a 0 to port B5,6,7. Mask all other bits.
	usleep(500000);

	morseCodeMessage m1("this is my morse message");
	m1.print();
	m1.morseCodeToLights(PBDR, PBDDR);

	close(fd);	// close the special file
	return 0;
}



message::message(){
	cout << "\nEnter your message: " << endl;
	getline(cin, messageText, '\n');
}

message::~message(){ //nothing to free
}

message::message(string messageString){
	messageText = messageString;
}

void message::print(){
	cout << messageText << endl;
}

morseCodeMessage::morseCodeMessage(string messageString) : message(messageString){ //calls message constructor to set message parameter
	translate();
}

morseCodeMessage::morseCodeMessage(){
	translate();
}

morseCodeMessage::~morseCodeMessage(){ //nothing to free
}

void morseCodeMessage::print(void){
	cout << messageText << endl;
	cout << messageCode << endl;
}

void morseCodeMessage::translate(){
	string morse[26] = {".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....",
	"..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.",
	"...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.."}; //only alpha characters supported with this array
	unsigned int i;
	string tempString;
	int position;
	for(i=0;i<messageText.length();i++){
		//if(!isalpha(messageText[i])){
		//	cout << "Error: only alpha characters allowed to be translated" << endl;
		//	exit(0);
		//}
		if(islower(messageText[i])){ //handle lowercase letters
			position = messageText[i] - 'a';
			messageCode += morse[position];
			messageCode += " ";
		} else if(isupper(messageText[i])){ //handle capital letters
			position = messageText[i] - 'A';
			messageCode += morse[position];
			messageCode += " ";
		} else{
			switch(messageText[i]){ //special cases for handling punctuation
			case '.':
				tempString = ".-.-.- ";
				break;
			case ',':
				tempString = "--..-- ";
				break;
			case '?':
				tempString = "..--.. ";
				break;
			case ' ':
				tempString = " ";
				break;
			case '!':
				tempString = "-.-.-- ";
				break;
			case '\'':
				tempString = ".----. ";
				break;
			default:
				cout << "ERROR invalid character \"" << messageText[i] << "\"" << endl; //only alpha characters and some punctuation supported
				exit(0);
			}
			messageCode += tempString;
		}
	}
}

void morseCodeMessage::morseCodeToLights(unsigned long* PBDR, unsigned long* PBDDR){
	int i;
	int length;
	length = messageCode.length();
	for(i=0;i<length;i++){
		if(messageCode[i] == '.'){
			*PBDR |= 0x80; //turn on green led
			usleep(250000);
			*PBDR &= ~0x80; //turn off green led
			usleep(100000);
		} else if(messageCode[i] == '-'){
			*PBDR |= 0x80; //turn on green led
			usleep(500000);
			*PBDR &= ~0x80; //turn off green led
			usleep(100000);
		} else if(messageCode[i] == ' '){
			if((i+1 != length) && messageCode[i+1] == ' '){
				*PBDR |= 0x20; //turn on red led
				usleep(250000);
				*PBDR &= ~0x20; //turn off red led
				usleep(100000);
				i++;
			} else {
				*PBDR |= 0x40; //turn on yellow led
				usleep(250000);
				*PBDR &= ~0x40; //turn off yellow led
				usleep(100000);
			}
		}
	}
}

messageStack::messageStack(message* firstMessage){
	stack.push_back(firstMessage);
}

messageStack::~messageStack(){
}

void messageStack::push(message* newMessage){
	stack.push_back(newMessage);
}

message* messageStack::pop(void){
	message* tempMessage;
	tempMessage = stack.back();
	stack.pop_back();
	return tempMessage;
}

void messageStack::printStack(void){
	int i;
	int size = stack.size();
	for(i=1;i<=size;i++){
		stack[size-i]->print();
	}
}
