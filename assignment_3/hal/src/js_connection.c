#include "hal/js_connection.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>


pthread_t communication_thread; 
pthread_mutex_t command_mutex = PTHREAD_MUTEX_INITIALIZER;

static bool node_command_pending = false;

buffer_info recv_data;
buffer_info send_data;
char recv_buffer[RECVBUFLEN];
char send_buffer[SENDBUFLEN];
//for splitting the message
char *co = NULL;
char *arg = NULL;
jsonCmd returning_com = CMD_VOLUME;
static int vol_, tempo_, mode_, play_;
//converts the message to lower case
static void lower(char *message){
    if(message == NULL) return;
    for(int i=0; message[i] != '\0'; i++){
        message[i] = tolower(message[i]);
    }
}

//thread function to be called inside the init
void* comm_func(void* data){
    //typecasting the void pointer to the buffer_info struct
    buffer_info*  cach = (buffer_info*) data;
    jsonCmd comm = CMD_VOLUME;
    while(comm != CMD_STOP){
        recv_message(data);                             //receive the message
        comm = jSconnection_parseMessage(cach->buff);               //parse the message to get the command
        jSconnection_generateResponse(comm);                        //generate the response
        send_message(&send_data);                       //send the response
    }

    return NULL;
}

//this function will be used to start the connection and create the thread
void jSconnection_init(void){
    if(init_socket() == FAIL){
        perror("socket initialization failed\n");
        exit(1);
    }
    //initializing the buffer_info struct
    recv_data.buff = recv_buffer;
    recv_data.buff_len = RECVBUFLEN;
    send_data.buff = send_buffer;
    send_data.buff_len = SENDBUFLEN;

    //pthread_mutex_init(&command_mutex, NULL);
    if(pthread_create(&communication_thread, NULL, comm_func, (void*)&recv_data) != 0){
        perror("thread creation failed\n");
        exit(1);
    }
}

void jSconnection_stop(void){
    pthread_join(communication_thread, NULL);
    //this is to make sure that the even after closing the thread, main also exits the socket
    returning_com = CMD_STOP;
    close_socket();
}

jsonCmd jSconnection_parseMessage(char *message){
    lower(message);
    jsonCmd command;
    if(strstr(message, "stop")!=NULL){command =  CMD_STOP;}
    else{
        co = strtok(message, " "); 
        arg = strtok(NULL, " ");
        pthread_mutex_lock(&command_mutex);
        if(strstr(message, "volume")!=NULL){command = CMD_VOLUME; if(strcmp(arg, "null") != 0){vol_ = atoi(arg); node_command_pending = true; }}
        else if(strstr(message, "tempo")!=NULL){command =  CMD_TEMPO;if(strcmp(arg, "null") != 0){tempo_ = atoi(arg); node_command_pending = true; }}
        else if(strstr(message, "mode")!=NULL){command = CMD_MODE;if(strcmp(arg, "null") != 0 ){mode_ = atoi(arg); node_command_pending = true; }}   
        else if(strstr(message, "play")!=NULL){command =  CMD_PLAY;if(strcmp(arg, "null") != 0 ){play_ = atoi(arg); node_command_pending = true; }}
        else{command =  CMD_STOP;}   

             
        pthread_mutex_unlock(&command_mutex);
    }

    returning_com = command;
    return command;
}

void jSconnection_generateResponse(jsonCmd command){
    switch(command){
        case CMD_STOP:
            snprintf(send_data.buff, SENDBUFLEN, "Program terminating.\n");
            send_data.buff_len = strlen(send_data.buff);
            break;
        case CMD_VOLUME:
            snprintf(send_data.buff, SENDBUFLEN, "%d\n", vol_);
            send_data.buff_len = strlen(send_data.buff);
            break;
        case CMD_TEMPO:
            snprintf(send_data.buff, SENDBUFLEN, "%d\n", tempo_);
            send_data.buff_len = strlen(send_data.buff);
            break;
        case CMD_MODE:
            snprintf(send_data.buff, SENDBUFLEN, "%d\n", mode_);
            send_data.buff_len = strlen(send_data.buff);
            break;
        case CMD_PLAY:
            snprintf(send_data.buff, SENDBUFLEN, "%d\n", play_);
            send_data.buff_len = strlen(send_data.buff);
            break;
    }
}

jsonCmd jSconnection_getCurrentCommand(void){
    return returning_com;
}

void jSconnection_fromMain(int *vol, int *tempo, int *mode, int *play){
    pthread_mutex_lock(&command_mutex);
    if (!node_command_pending) {
        vol_ = *vol;
        tempo_ = *tempo;
        mode_ = *mode;  // Adjust mapping as needed (original code: mode or beat_num)
        play_ = *play;
    }
    pthread_mutex_unlock(&command_mutex);
}

void jSconnection_toMain(int *vol, int *tempo, int *mode, int *play){
    pthread_mutex_lock(&command_mutex);
    if (node_command_pending) {
        *vol = vol_;
        *tempo = tempo_;
        *mode = mode_;
        *play = play_;
        // Clear the flag after applying the Node.js update once
        node_command_pending = false;
    }
    pthread_mutex_unlock(&command_mutex);
}
