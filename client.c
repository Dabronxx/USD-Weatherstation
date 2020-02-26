/**
 * client.c%s
 *
 * @author Branko Andrews
 *
 * USD COMP 375: Computer Networks
 * Project 1
 *
 * TODO: Program displays information from the USD weatherstation by
 * using the C socket API to extract information from the sensor network
 */

#define _XOPEN_SOURCE 600

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

long prompt();
int connectToHost(char *hostname, char *port);
void authenticate(int * server_fd, char * input, char * output);
void sendMessage(int sD, char * msgBuff);
void receiveMessage(int sD, char * msgBuff);
void mainLoop();
#define MAX_IN 100
#define MAX_OUTPUT 1024

int main()
{
    printf("WELCOME TO THE COMP375 SENSOR NETWORK\n\n\n");
    
    mainLoop();
    
    return 0;
}

/**
 * function sends message to client and checks to make sure a connection is established
 *
 * @param sD Socket file descriptor for communicating with the server
 * @param msgBuff Buffer to hold the message to be sent to the client
 */
void sendMessage(int sD, char * msgBuff)
{
  ssize_t sent;
  //Error checking for connection error
  if((sent = send(sD, msgBuff, strlen(msgBuff), 0)) == 0)
  {
    printf("Error, no bytes were sent");
    exit(1);
  }
  else if(sent == -1)
  {
    printf("Connection Error");
    exit(1);
  }
}
/**
 * function receives a message from the client and checks to make sure a connection is established
 *
 * @param sD Socket file descriptor for communicating with the server
 * @param msgBuff Buffer to hold the message to be received from the client
 */
void receiveMessage(int sD, char * msgBuff)
{
  ssize_t received;
  //Error checking for connection error
  if((received = recv(sD, msgBuff, MAX_OUTPUT, 0)) == 0)
  {
    printf("Error, no bytes were received");
    exit(1);
  }
  else if(received == -1)
  {
    printf("Connection Error");
    exit(1);
  }
  
}
/**
 * function authenticates a connection with hopper.sandiego.edu and
 * the weatherstation using the send and receive message functions
 * 
 * @param server_fd Socket file descriptor for communicating with the server
 * @param input Buffer to hold the message to be sent to the client
 * @param output Buffer to hold the message being received from the client
 */
void authenticate(int * server_fd, char * input, char * output)
{
    //Password is coppied to input buffer
    strcpy(input, "AUTH password123\n");
    
    //Password is sent to client
    sendMessage(*server_fd, input);
    
    //Response from client is received
    receiveMessage(*server_fd, output);
    
    //Successful connection is verified
    if(strstr(output, "CONNECT") == NULL)
    {
        printf("Authentication Failed\n");
        exit(0);
    }
    strcat(output, "\n");
    
    char domainName[MAX_IN];
    
    char socket[MAX_IN];
    
    char sensorCode[MAX_IN];
    
    //Domain name, socket, and sensorCode from the client response is extracted
    sscanf(output, "CONNECT %s %s %s", domainName, socket, sensorCode);
    
    //Connection to hopper.sandiego.edu is closed
    close(*server_fd);
    
    //Connection to the weatherstation is established
    *server_fd = connectToHost(domainName, socket);
    
    memset(input, 0, MAX_IN);
    
    strcpy(input, "AUTH ");
    strcat(input, sensorCode);
    strcat(input, "\n");
    
    //Password to the weatherstation is sent
    sendMessage(*server_fd, input);
    memset(output, 0, MAX_OUTPUT);
    
    receiveMessage(*server_fd, output);
    
    //Connection to the weather station is validated
    if(strstr(output, "SUCCESS") == NULL)
    {
        printf("Authentication Failed\n");
        exit(0);
    }
    memset(output, 0, MAX_OUTPUT);
    memset(input, 0, MAX_IN);
}
/**
 * Loop to keep asking user what they want to do and calling the appropriate
 * function to handle the selection.
 *
 */
void mainLoop()
{
    //Buffer variables are initiated
    char inputBuffer[MAX_IN];
    char outputBuffer[MAX_OUTPUT];
    
    //Variable to hold the time value received from the weather stattion
    long time;
    
    //Variable to hold the time value as a string
    char * timeString;
    
    //Variable to hold the value for the particular weather measurement
    int value;
    
    //variable holds the unit of measurement for measurement value
    char symbol[5];
    
    //Variable holds the sensor type to be used and passed to the weather station
    char * sensorType = NULL;
    int server_fd;
    while (1)
    {
        long selection = prompt();
        
        switch (selection)
        {
            case 1:
                sensorType = "AIR TEMPERATURE\n";
                break;
            case 2:
                sensorType = "RELATIVE HUMIDITY\n";
                break;
            case 3:
                sensorType = "WIND SPEED\n";
                break;
            case 4:
                printf("GOODBYE!\n");
                goto EndWhile;
            default:
                fprintf(stderr, "ERROR: Invalid selection\n");
                break;
            
        }
        //Connection to hopper.sandiego.edu is established
        server_fd = connectToHost("hopper.sandiego.edu", "7030");
	memset(outputBuffer, 0, MAX_OUTPUT);
	memset(inputBuffer, 0, MAX_IN);
	
	//Connection is authenticated
        authenticate(&server_fd, inputBuffer, outputBuffer);
        strcpy(inputBuffer, sensorType);
	
	//Sensor type is passed to the weather station
        sendMessage(server_fd, inputBuffer);
        receiveMessage(server_fd, outputBuffer);
	
	//Weather values are extracted from the outputBuffer
        sscanf(outputBuffer, "%ld %i %s", &time, &value, symbol);
	
	//Time value is converted to a string
        timeString = ctime(&time);
	
	//Variable of the length of the Sensor Type string minus 1 so the \n is not printed in the final output
	int msgLength = strlen(sensorType) -1;
        printf("\nThe last %*.*s reading was %i %s at %s\n", msgLength, msgLength, sensorType, value, symbol, timeString);
	
	//Connection to the weather station is closed
        close(server_fd);
        memset(outputBuffer, 0, MAX_OUTPUT);
        memset(symbol, 0, 5);
    }
    EndWhile: ;

}

/**
 * Print command prompt to user and obtain user input.
 *%s
 * @return The user's desired selection, or -1 if invalid selection.
 */
long prompt() {
    // TODO: add printfs to print out the options
    
    printf("Which sensor would you like to read: \n\n");
    printf("\t(1) Air temperature\n");
    printf("\t(2) Relative Humidity\n");
    printf("\t(3) Wind Speed\n");
    printf("\t(4) Quit Program\n\n");
    printf("Selection: ");

    // Read in a value from standard input
    char input[10];
    memset(input, 0, 10); // set all characters in input to '\0' (i.e. nul)
    char *read_str = fgets(input, 10, stdin);

    // Check if EOF or an error, exiting the program in both cases.
    if (read_str == NULL) {
        if (feof(stdin)) {
            exit(0);
        }
        else if (ferror(stdin)) {
            perror("fgets");
            exit(1);
        }
    }

    // get rid of newline, if there is one
    char *new_line = strchr(input, '\n');
    if (new_line != NULL) new_line[0] = '\0';

    // convert string to a long int
    char *end;
    long selection = strtol(input, &end, 10);

    if (end == input || *end != '\0') {
        selection = -1;
    }

    return selection;
}

/**
 * Socket implementation of connecting to a host at a specific port.
 *
 * @param hostname The name of the host to connect to (e.g. "foo.sandiego.edu")
 * @param port The port number to connect to
 * @return File descriptor of new socket to use.
 */
int connectToHost(char *hostname, char *port)
{
    // Step 1: fill in the address info in preparation for setting
    //   up the socket
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;  // will point to the results

    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_INET;       // Use IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

    // get ready to connect
    if ((status = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // Step 2: Make a call to socket
    int fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (fd == -1) {
        perror("socket");
        exit(1);
    }

    // Step 3: connect!
    if (connect(fd, servinfo->ai_addr, servinfo->ai_addrlen) != 0) {
        perror("connect");
        exit(1);
    }

    freeaddrinfo(servinfo); // free's the memory allocated by getaddrinfo

    return fd;
}

