/*
 * MIT License
 *
 * Copyright (c) 2018 Lewis Van Winkle
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "chap07.h"
#include "address_map_arm.h"

/******** Starting ADC ********/

/*******************************************************************************
 * This program demonstrates use of the ADC port.
 *
 * It performs the following:
 * 1. Performs a conversion operation on all eight channels.
 * 2. Displays the converted values on the terminal window.
*******************************************************************************/
int fd;                     // used to open /dev/mem
void *h2p_lw_virtual_base;          // the light weight buss base
volatile unsigned int * ADC_ptr = NULL;    // virtual address pointer to read ADC


// measure time
struct timeval t1 = (struct timeval){0};
struct timeval t2 = (struct timeval){0};
double elapsedTime = 0.0;
double elapsed_500 = 0.0;
bool samples_500 = false;
double sampling_rate = 0.0;

/******** Ending ADC ********/




/******** Starting Networking ********/
const char *get_content_type(const char* path) {
    const char *last_dot = strrchr(path, '.');
    if (last_dot) {
        if (strcmp(last_dot, ".css") == 0) return "text/css";
        if (strcmp(last_dot, ".csv") == 0) return "text/csv";
        if (strcmp(last_dot, ".gif") == 0) return "image/gif";
        if (strcmp(last_dot, ".htm") == 0) return "text/html";
        if (strcmp(last_dot, ".html") == 0) return "text/html";
        if (strcmp(last_dot, ".ico") == 0) return "image/x-icon";
        if (strcmp(last_dot, ".jpeg") == 0) return "image/jpeg";
        if (strcmp(last_dot, ".jpg") == 0) return "image/jpeg";
        if (strcmp(last_dot, ".js") == 0) return "application/javascript";
        if (strcmp(last_dot, ".json") == 0) return "application/json";
        if (strcmp(last_dot, ".png") == 0) return "image/png";
        if (strcmp(last_dot, ".pdf") == 0) return "application/pdf";
        if (strcmp(last_dot, ".svg") == 0) return "image/svg+xml";
        if (strcmp(last_dot, ".txt") == 0) return "text/plain";
    }

    return "application/octet-stream";
}


SOCKET create_socket(const char* host, const char *port) {
    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    getaddrinfo(host, port, &hints, &bind_address);

    printf("Creating socket...\n");
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family,
            bind_address->ai_socktype, bind_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_listen)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        exit(1);
    }

    printf("Binding socket to local address...\n");
    if (bind(socket_listen,
                bind_address->ai_addr, bind_address->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        exit(1);
    }
    freeaddrinfo(bind_address);

    printf("Listening...\n");
    if (listen(socket_listen, 10) < 0) {
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
        exit(1);
    }

    return socket_listen;
}



#define MAX_REQUEST_SIZE 2047

struct client_info {
    socklen_t address_length;
    struct sockaddr_storage address;
    char address_buffer[128];
    SOCKET socket;
    char request[MAX_REQUEST_SIZE + 1];
    int received;
    struct client_info *next;
};


struct client_info *get_client(struct client_info **client_list,
        SOCKET s) {
    struct client_info *ci = *client_list;

    while(ci) {
        if (ci->socket == s)
            break;
        ci = ci->next;
    }

    if (ci) return ci;
    struct client_info *n =
        (struct client_info*) calloc(1, sizeof(struct client_info));

    if (!n) {
        fprintf(stderr, "Out of memory.\n");
        exit(1);
    }

    n->address_length = sizeof(n->address);
    n->next = *client_list;
    *client_list = n;
    return n;
}


void drop_client(struct client_info **client_list,
        struct client_info *client) {
    CLOSESOCKET(client->socket);

    struct client_info **p = client_list;

    while(*p) {
        if (*p == client) {
            *p = client->next;
            free(client);
            return;
        }
        p = &(*p)->next;
    }

    fprintf(stderr, "drop_client not found.\n");
    exit(1);
}


const char *get_client_address(struct client_info *ci) {
    getnameinfo((struct sockaddr*)&ci->address,
            ci->address_length,
            ci->address_buffer, sizeof(ci->address_buffer), 0, 0,
            NI_NUMERICHOST);
    return ci->address_buffer;
}




fd_set wait_on_clients(struct client_info **client_list, SOCKET server) {
    fd_set reads;
    FD_ZERO(&reads);
    FD_SET(server, &reads);
    SOCKET max_socket = server;

    struct client_info *ci = *client_list;

    while(ci) {
        FD_SET(ci->socket, &reads);
        if (ci->socket > max_socket)
            max_socket = ci->socket;
        ci = ci->next;
    }

    if (select(max_socket+1, &reads, 0, 0, 0) < 0) {
        fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
        exit(1);
    }

    return reads;
}


void send_400(struct client_info **client_list,
        struct client_info *client) {
    const char *c400 = "HTTP/1.1 400 Bad Request\r\n"
        "Connection: close\r\n"
        "Content-Length: 11\r\n\r\nBad Request";
    send(client->socket, c400, strlen(c400), 0);
    drop_client(client_list, client);
}

void send_404(struct client_info **client_list,
        struct client_info *client) {
    const char *c404 = "HTTP/1.1 404 Not Found\r\n"
        "Connection: close\r\n"
        "Content-Length: 9\r\n\r\nNot Found";
    send(client->socket, c404, strlen(c404), 0);
    drop_client(client_list, client);
}



void serve_resource(struct client_info **client_list,
        struct client_info *client, const char *path, volatile unsigned int *ADC_ptr) {
#define BSIZE 1024
    volatile unsigned int *ADC_pointer = ADC_ptr;

    char buffer_to_send[BSIZE];


    sprintf(buffer_to_send, "{\r\n");
    int i;
    for(i = 0; i < 7; i++) {
        sprintf(buffer_to_send + strlen(buffer_to_send), "\"c%d\":%f,\r\n", i, (float) (*(ADC_pointer+i)&0xFFF)/1000.0);
    }
    sprintf(buffer_to_send + strlen(buffer_to_send), "\"c%d\":%f\r\n", i, (float) (*(ADC_pointer+i)&0xFFF)/1000.0);
    sprintf(buffer_to_send + strlen(buffer_to_send), "}");

    if (strlen(path) > 100) {
        send_400(client_list, client);
        return;
    }

    if (strstr(path, "..")) {
        send_404(client_list, client);
        return;
    }

#if defined(_WIN32)
    char *p = full_path;
    while (*p) {
        if (*p == '/') *p = '\\';
        ++p;
    }
#endif

    char buffer[BSIZE];

    sprintf(buffer, "HTTP/1.1 200 OK\r\n");
    send(client->socket, buffer, strlen(buffer), 0);

    sprintf(buffer, "Connection: close\r\n");
    send(client->socket, buffer, strlen(buffer), 0);


    sprintf(buffer, "{""channel_0"":""1.2345644""}");


    sprintf(buffer, "Content-Length: %u\r\n", sizeof(buffer_to_send)-1);
    send(client->socket, buffer, strlen(buffer), 0);

    sprintf(buffer, "Content-Type: application/json\r\n");
    send(client->socket, buffer, strlen(buffer), 0);

    sprintf(buffer, "\r\n");
    send(client->socket, buffer, strlen(buffer), 0);
    

    send(client->socket, buffer_to_send, strlen(buffer_to_send), 0);


    drop_client(client_list, client);
}
/******** Ending Networking ********/



int main() {

    // creating a child process
    int pid = fork();

    // executed by the child process
    if (pid == 0) {

        // delay the server 
        volatile int delay = 0;
        for(delay = 0; delay <= 10000; delay++);

        int fd;

        // Open /dev/mem
        if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) 	{
            printf( "ERROR: could not open \"/dev/mem\"...\n" );
            return( 1 );
        }

        // get virtual addr that maps to physical
        void *h2p_lw_virtual_base = mmap( NULL, LW_BRIDGE_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, LW_BRIDGE_BASE );	
        if( h2p_lw_virtual_base == MAP_FAILED ) {
            printf( "ERROR: mmap1() failed...\n" );
            close( fd );
            return(1);
        }

        // Set virtual address pointer to I/O port
        volatile unsigned int *ADC_ptr = (unsigned int *) (h2p_lw_virtual_base + ADC_BASE);

        #if defined(_WIN32)
            WSADATA d;
            if (WSAStartup(MAKEWORD(2, 2), &d)) {
                fprintf(stderr, "Failed to initialize.\n");
                return 1;
            }
        #endif

            SOCKET server = create_socket(0, "8080");

            struct client_info *client_list = 0;

            while(1) {

                fd_set reads;
                reads = wait_on_clients(&client_list, server);

                if (FD_ISSET(server, &reads)) {
                    struct client_info *client = get_client(&client_list, -1);

                    client->socket = accept(server,
                            (struct sockaddr*) &(client->address),
                            &(client->address_length));

                    if (!ISVALIDSOCKET(client->socket)) {
                        fprintf(stderr, "accept() failed. (%d)\n",
                                GETSOCKETERRNO());
                        return 1;
                    }


                    printf("New connection from %s.\n",
                            get_client_address(client));
                }


                struct client_info *client = client_list;
                while(client) {
                    struct client_info *next = client->next;

                    if (FD_ISSET(client->socket, &reads)) {

                        if (MAX_REQUEST_SIZE == client->received) {
                            send_400(&client_list, client);
                            client = next;
                            continue;
                        }

                        int r = recv(client->socket,
                                client->request + client->received,
                                MAX_REQUEST_SIZE - client->received, 0);

                        if (r < 1) {
                            printf("Unexpected disconnect from %s.\n",
                                    get_client_address(client));
                            drop_client(&client_list, client);

                        } else {
                            client->received += r;
                            client->request[client->received] = 0;

                            char *q = strstr(client->request, "\r\n\r\n");
                            if (q) {
                                *q = 0;

                                if (strncmp("GET /", client->request, 5)) {
                                    send_400(&client_list, client);
                                } else {
                                    char *path = client->request + 4;
                                    char *end_path = strstr(path, " ");
                                    if (!end_path) {
                                        send_400(&client_list, client);
                                    } else {
                                        *end_path = 0;
                                        serve_resource(&client_list, client, path, ADC_ptr);
                                    }
                                }
                            } //if (q)
                        }
                    }

                    client = next;
                }

            } //while(1)


            printf("\nClosing socket...\n");
            CLOSESOCKET(server);


        #if defined(_WIN32)
            WSACleanup();
        #endif

            printf("Finished.\n");
            return 0;
    } 
    
    
    







    // executed by the parent process
    else {
        // Open /dev/mem
        if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) 	{
            printf( "ERROR: could not open \"/dev/mem\"...\n" );
            return( 1 );
        }

        // get virtual addr that maps to physical
        h2p_lw_virtual_base = mmap( NULL, LW_BRIDGE_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, LW_BRIDGE_BASE );	
        if( h2p_lw_virtual_base == MAP_FAILED ) {
            printf( "ERROR: mmap1() failed...\n" );
            close( fd );
            return(1);
        }

        // Set virtual address pointer to I/O port
        ADC_ptr = (unsigned int *) (h2p_lw_virtual_base + ADC_BASE);

        volatile int delay_count; // volatile so that the C compiler doesn't remove the loop
        int port, value;

        unsigned int num_samples_by_all = 0;       // sampling all 8 channels counts as one sample
        unsigned int num_samples_by_single = 0;       // sampling all 8 channels counts as one sample

        printf("\033[2J"); // erase terminal window
        printf("\033[H");  // move cursor to 0,0 position

        *(ADC_ptr + 1) = 1; // Sets the ADC up to automatically perform conversions.

        // start timer for the first time
        gettimeofday(&t1, NULL); 
        while (1) {
            if ((*ADC_ptr) & 0x8000) // check the refresh bit (bit 15) of port 1 to
                                    // check for update
            {
                printf("\033[H");
                //printf("\033[2J"); // sets the cursor to the top-left of the terminal
                                // window
                for (port = 0; port < 8; ++port) {
                    value = (*(ADC_ptr + port)) & 0xFFF;
                    printf("ADC port %d: %1.4f Volts\n", port, value/1000.0);
                    num_samples_by_single++;    // one sample taken
                }


                /****** statistics computations ******/
                num_samples_by_all++;       // 8 samples taken

                // time
                gettimeofday(&t2, NULL);
                elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
                elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
                elapsed_500 += elapsedTime;
                gettimeofday(&t1, NULL);

                printf("\n\n");
                printf("************ stats *************\n");

                printf("individual samples: %09u X8 samples\n", num_samples_by_single);
                printf("individual samples: %09u samples\n\n", num_samples_by_all);

                printf("period of sample: %4.5lf ms\n", elapsedTime);
                samples_500 = num_samples_by_all%500==0;    // check if consecutive 500 samples were taken
                sampling_rate = samples_500 ? (double) 500/elapsed_500*1000 : sampling_rate;
                printf("sampling rate (all 8 channels): %6.0lf samples/sec\n", sampling_rate);
                elapsed_500 = samples_500 ? 0.0 : elapsed_500;
            }

            //for (delay_count = 1500000; delay_count != 0; --delay_count); // delay loop
        }

        return 0;





    }
}