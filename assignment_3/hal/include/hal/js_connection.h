#ifndef JS_CONNECTION_H
#define JS_CONNECTION_H
/*
 * Filename: js_connection.h
 * Description: This header file defines the functions and structures required 
 *              for handling UDP-based communication between the BeagleY AI board 
 *              and the Node.js server. It facilitates sending and receiving 
 *              commands related to volume, tempo, mode, and playback.
 */
#include "hal/udp.h"
#include <pthread.h>
/*
 * Enum: jsonCmd
 * Defines the possible command types that can be received from the Node.js server.
 */
typedef enum {
    CMD_STOP,    // Command to stop the system
    CMD_VOLUME,  // Command to adjust the volume
    CMD_TEMPO,   // Command to adjust the BPM/tempo
    CMD_MODE,   
    CMD_PLAY     
} jsonCmd;
/*
 * Function: jSconnection_init
 * Initializes the UDP network connection and starts a separate communication thread.
 */
void jSconnection_init(void);

/*
 * Function: jSconnection_stop
 * Gracefully stops the UDP communication thread and closes the network socket.
 */
void jSconnection_stop(void);

/*
 * Function: jSconnection_parseMessage
 */
jsonCmd jSconnection_parseMessage(char *message);

/*
 * Function: jSconnection_generateResponse
 */
void jSconnection_generateResponse(jsonCmd command);

/*
 * Function: jSconnection_getCurrentCommand
 */
jsonCmd jSconnection_getCurrentCommand(void);

/*
 * Function: jSconnection_fromMain
 * Transfers local joystick and knob values to the network variables.
 * Ensures that the latest hardware input values are shared with the server.
 * 
 * Parameters:
 *   - vol: Pointer to the volume value.
 *   - tempo: Pointer to the tempo (BPM) value.
 *   - mode: Pointer to the drum mode value.
 *   - play: Pointer to the play state.
 */
void jSconnection_fromMain(int *vol, int *tempo, int *mode, int *play);

/*
 * Function: jSconnection_toMain
 * Updates local values (volume, BPM, mode, play state) from network commands.
 * Ensures that server commands override joystick inputs if applicable.
 */
void jSconnection_toMain(int *vol, int *tempo, int *mode, int *play);

#endif
