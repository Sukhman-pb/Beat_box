#ifndef UDP_H
#define UDP_H
#define RECVBUFLEN 64     //we will only take smaller messages
#define SENDBUFLEN 1500   //we need to send alot of data so we need a bigger buffer needs to be atleadt 1500 in order to recieve the fragmentation
#define SUCCESS 0
#define FAIL -1
#define PORT 12345

/**
 * thread_data
 * Structure to store buffer information for thread functions.
 *
 * This structure is used to pass buffer data to the send and receive functions.
 */
typedef struct{
    char* buff;
    int buff_len;
}buffer_info;

/**
 * Initializes and binds a UDP socket.
 * 
 * Creates a UDP socket, binds it to the specified port, and makes it ready for communication.
 *
 * SUCCESS (0) if the socket was initialized successfully, otherwise FAIL (1).
 */
int init_socket(void);

/**
 * Retrieves the socket descriptor.
 * 
 * Returns the file descriptor of the created UDP socket.
 *
 * The socket descriptor on success, or -1 on failure.
 */
int get_socket_descriptor(void);

/**
 * Receives a message from the UDP socket.
 * 
 * Listens for incoming UDP messages and stores them in the provided buffer.
 *
 * recv_data A pointer to a buffer_data struct containing the buffer and its length.
 * NULL (used for thread compatibility).
 */
int recv_message(buffer_info *recv_data);    

/**
 * Sends a message through the UDP socket.
 * 
 * Takes a message and sends it to the last known sender.
 *
 *  send_data A pointer to a buffer_data struct containing the message and its length.
 *  NULL (used for thread compatibility).
 */
void *send_message(buffer_info *send_data);

/**
 * Closes the UDP socket and frees resources.
 * 
 * Terminates the UDP connection by closing the socket.
 *
 * SUCCESS (0) if closed successfully, otherwise FAIL (1).
 */
int close_socket(void);
 
#endif