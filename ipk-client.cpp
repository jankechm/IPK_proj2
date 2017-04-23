/**
 * File:        ipk-client.cpp
 * Author:      Marek Jankech
 * Dátum:       23.4.2017
 * Project:     Client for the computation of mathematic operations
 * Description: The client connects to the server, receives a mathematic task,
 *              solves it and sends the solution to the server
 */

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <unistd.h>
#include <string>
#include <cstring>
#include <climits>
#include <regex.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#include "md5.h"

#define BUFSIZE 1024
#define DEC 10

using namespace std;

/**
 *Global constants
 */
const string login = "xjanke01";
const char *serverPort = "55555";
const char *solveRegex = "\n? *SOLVE ([+-]?[0-9]+ [+-\\*\\/] [+-]?[0-9]+)\n";
const char *byeRegex = "\n? *BYE ([^\n]*)\n";

/**
 *Prototypes of functions
 */
bool match(const char *pattern, char *haystack, string &matched);
vector<string> tokenize(string str);
void errTerminate(string msg);
int evaluate(vector<string> arr, long double *result);
string lDouble2Str(long double number);

int main(int argc, char *argv[])
{
  //Check the number of arguments
  if (argc != 2 ) {
    errTerminate("arguments");
  }

  string loginHash = md5(login);
  string msg = "";
  string exercise = "";
  string secret = "";
  vector<string> arr;
  char buffer[BUFSIZE];
  struct addrinfo hints;
  struct addrinfo *result;
  int clientSocket, ecode, bytesrx;
  long double solution;

  //Reset data structures
  memset(&hints, 0, sizeof(struct addrinfo));
  memset(buffer, 0, sizeof(buffer));
  //Specify criteria
  hints.ai_flags = 0;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  //Address translation for socket
  if ((ecode = (getaddrinfo(argv[1], serverPort, &hints, &result))) != 0) {
    errTerminate(gai_strerror(ecode));
  }
  //Create socket
  if ((clientSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) == -1) {
		errTerminate("socket");
	}
  //Establish a connection to the server
  if (connect(clientSocket, result->ai_addr, result->ai_addrlen) == -1) {
    errTerminate("connect");
  }
  //Free addrinfo structure
  freeaddrinfo(result);
  //Send HELLO message
  msg = "HELLO " + loginHash + "\n";
  if ((send(clientSocket, msg.c_str(), msg.size(), 0)) == -1) {
    errTerminate("send");
  }
  //Communicate with the server
  while(1) {
    bytesrx = recv(clientSocket, buffer, BUFSIZE, 0);
    if (bytesrx == -1) {
      errTerminate("recv");
    }
    else {
      //Terminate string with '\0'
      buffer[bytesrx] = '\0';
      //BYE message from the server - print the secret and close connection
      if (match(byeRegex, buffer, secret)) {
        cout << secret;
        break;
      }
      //SOLVE message from the server - answer it
      else if (match(solveRegex, buffer, exercise)) {
        arr = tokenize(exercise);
        //The client cannot solve the task - sending ERROR
        if ((evaluate(arr, &solution)) == -1) {
          msg = "RESULT ERROR\n";
        }
        //The client can solve the task - sending solution
        else {
          msg = "RESULT " + lDouble2Str(solution) + "\n";
        }
        //cout << exercise << "\n";
        //cout << msg << "\n";
        //Sending response to the server
        if ((send(clientSocket, msg.c_str(), msg.size(), 0)) == -1) {
          close(clientSocket);
          errTerminate("send");
        }
      }
    }
  }
  close(clientSocket);
}

/**
 * Function for terminating the program with an error message
 */
void errTerminate(string msg) {
  cerr << "Error: " << msg << "\n";
  exit(EXIT_FAILURE);
}

/**
 * Function for matching a pattern in the string
 */
bool match(const char *pattern, char *haystack, string &matched) {
  bool match = false;
  const int nmatch = 2;
  regmatch_t pmatch[nmatch];
  regex_t regexp;
  int status;
  char buf[BUFSIZE];

  //Compile regex
  if ((status = regcomp(&regexp, pattern, REG_EXTENDED)) != 0) {
    regerror(status, &regexp, buf, BUFSIZE);
    printf("error = %s\n", buf);
    errTerminate("regcomp");
  }
  //Execute the regex for the string
  if ((regexec(&regexp, haystack, nmatch, pmatch, 0)) == 0) {
    match = true;
    matched = ((string)haystack).substr(pmatch[1].rm_so, pmatch[1].rm_eo - pmatch[1].rm_so);

  }
  regfree(&regexp);
  return match;
}

/**
 * Function for tokenizing string
 */
vector<string> tokenize(string str) {
  vector<string> arr;
  stringstream ss(str);
  string tmp;

  while (ss >> tmp) {
    arr.push_back(tmp);
  }
  return arr;
}

/**
 * Function for evaluating an expression in vector
 */
int evaluate(vector<string> arr, long double *result) {
  const int denominator = 100;
  long long int operand1;
  long long int operand2;
  stringstream ss;
  char operation;
  long long int tmp;

  //Convert strings to values
  operand1 = strtoll(arr[0].c_str(), NULL, DEC);
  operand2 = strtoll(arr[2].c_str(), NULL, DEC);
  ss << arr[1];
  ss >> operation;
  //Test limits
  if (operand1 == LLONG_MAX || operand1 == LLONG_MIN || operand2 == LLONG_MAX || operand2 == LLONG_MIN) {
    return -1;
  }
  //Evaluate
  switch (operation) {
    case '+':
      *result = operand1 + operand2;
      break;
    case '-':
      *result = operand1 - operand2;
      break;
    case '*':
      *result = operand1 * operand2;
      break;
    case '/':
      if (operand2 == 0) {
        return -1;
      }
      *result = operand1 / (long double)operand2;
      break;
  }
  //truncate result
  tmp = *result * denominator;
  *result = tmp / double(denominator);
  return 0;
}

/**
 * Function for converting long double to string with 2 two decimal spaces
 */
string lDouble2Str(long double number) {
  ostringstream ostrs;
  ostrs.precision(2);
  ostrs << fixed << number;
  return ostrs.str();
}
