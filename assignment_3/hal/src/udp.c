#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "hal/udp.h"
#include <assert.h>

//the basic idea for this file is taken from the lecture slides and the following link
//https://opencoursehub.cs.sfu.ca/bfraser/grav-cms/cmpt433/notes/files/06-LinuxProgramming-c.pdf slide# 19-24
//global variables to be used in all the functions to set and get the values
static struct sockaddr_in sin_;
static int socketDescriptor = -1;
static struct sockaddr_in sinRemote;
static unsigned int sin_len = sizeof(sinRemote);
static char message_rc[RECVBUFLEN];
bool is_initialized = false;


// Function to create and bind  UDP socket
int init_socket(void) 
{
    assert(!is_initialized);
    socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketDescriptor == -1) {
        perror("socket failure \n");
        return FAIL;
    }

    // Initialize the local socket address structure
    memset(&sin_, 0, sizeof(sin_));
    sin_.sin_family = AF_INET;
    sin_.sin_addr.s_addr = htonl(INADDR_ANY);
    sin_.sin_port = htons(PORT);
    //  Bind the socket to the specified port(12345)
    if (bind(socketDescriptor, (struct sockaddr *)&sin_, sizeof(sin_)) < 0)
    {
        perror("bind failure \n");
        return FAIL;
    }
    is_initialized = true;
    return SUCCESS;  
}

int get_socket_descriptor(void)
{
    assert(is_initialized);
    return socketDescriptor;
}

// Function to receive message from the socket
//returns the byte received from the socket for debugging purposes returns -1 if failed at any point
int recv_message(buffer_info *recv_data)
{
    assert(is_initialized);
    //copying the buffer and buffer length to local variables
    char* buffer = recv_data->buff;
    int buffer_length = recv_data->buff_len;
    if(socketDescriptor == -1)
    {
        perror("socket descriptor is not initialized \n");
        return FAIL;
    }
    //Recieve the message from the socket
    int bytes_received = recvfrom(socketDescriptor, message_rc, RECVBUFLEN - 1, 0, (struct sockaddr *)&sinRemote, &sin_len);
    
    if(bytes_received == -1){
        perror("recieve error \n");
        return FAIL;
    }
    //Null terminate the message
    message_rc[bytes_received] = 0;
    //Check if the provided buffer is large enough 
    if(buffer_length < bytes_received)
    {
        perror("buffer length is smaller than the message length \n");
        return FAIL;
    }
    //Copy the recieved message into the user-provided buffer
    snprintf(buffer, buffer_length, "%s", message_rc);
    //this should run in an infinite loop to listen for messages and then generate response.
    return bytes_received;
}

// Function to send message to the socket
//buffer is the message to be sent
//buffer_length is the length of the message
void* send_message(buffer_info *send_data)
{
    assert(is_initialized);
    char* buffer = send_data->buff;
    int buffer_length = send_data->buff_len;
    if(socketDescriptor == -1)
    {
        perror("socket descriptor is not initialized \n");
        return NULL;
    }

    // Validate the buffer length
    if(buffer_length > SENDBUFLEN)
    {
        perror("buffer length is greater than the message length \n");
        return NULL;
    }

    // Send the message using the stored sender address
    int bytes_sent = sendto(socketDescriptor, buffer, buffer_length, 0, (struct sockaddr *)&sinRemote, sin_len);
    if (bytes_sent == -1) {
        perror("sending error \n");
        return NULL;
    }
    return NULL;
}

int close_socket(void)
{
   assert(is_initialized);
   if(close(socketDescriptor)){
        is_initialized = false;
        return SUCCESS;
   }
   return FAIL;
   
}
