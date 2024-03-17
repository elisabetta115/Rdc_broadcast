#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>


#define BUFSIZE 256
#define PORT 9500
#define DESTINATION_ADDRESS "255.255.255.255"



void show_error(const char *msg){
    perror(msg);
    exit(1);
}

int main(int argc, char** argv) {

    //initialization of variables for send socket
    int sock_fd, port_num, n, ret;
    char buffer[BUFSIZE];
    struct sockaddr_in srv_addr;
    struct hostent *srv;

    int new_sock_fd, cli_len;
    struct sockaddr_in new_srv_addr;

    //open send socket 
    port_num = PORT;
    sock_fd = socket (AF_INET, SOCK_DGRAM, 0);
    if(sock_fd<0) show_error ("ERROR while opening socket");

    int broadcastPermission = 1;
    ret = setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, sizeof(broadcastPermission));
    if (ret) show_error("setsockopt");

    memset((char *)&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = inet_addr(DESTINATION_ADDRESS);
    srv_addr.sin_port = htons(port_num);

    //open read socket
    new_sock_fd = socket (AF_INET, SOCK_DGRAM, 0);
    if(new_sock_fd<0) show_error ("ERROR while opening new_socket");

    int reuseAddressPermission = 1;
    ret = setsockopt(new_sock_fd, SOL_SOCKET, SO_REUSEADDR, (void *) &reuseAddressPermission, sizeof(reuseAddressPermission));
    if (ret) show_error("setsockopt");

    memset((char*)&new_srv_addr, 0, sizeof(new_srv_addr));
    port_num = PORT;
    new_srv_addr.sin_family = AF_INET;
    new_srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    new_srv_addr.sin_port = htons(port_num);

    
    
    ret = bind(new_sock_fd, (struct sockaddr *)&new_srv_addr, sizeof(new_srv_addr));
    if (ret<0) show_error("ERROR while binding socket");



    //initialization os sequence number
    int seq_number = 100000000;

    if(atoi(argv[1]) == 0){

        printf("Hi! I'm the leader node!\n");
        fflush(stdout);

        seq_number = 0;

        srand(time(NULL));
        sleep(1);
        int message = rand();
        sprintf(buffer, "0:%d:0", message);
        n = sendto(sock_fd, buffer, strlen(buffer), 0, (struct sockaddr *) &srv_addr, sizeof(srv_addr));
        if(n<0)show_error("ERROR sendto");
        printf ("Leader message %d\n", message);
        fflush(stdout);
    }




    while(1){
        memset ((char *)buffer, 0, BUFSIZE);

        //initialization of fields for message
        int sender_id,message, new_seq_number;

        //read from read socket
        n = read (new_sock_fd, buffer, BUFSIZE);
        if(n<0)show_error("ERROR read");


        //message division
        int r = sscanf (buffer, "%d:%d:%d\n",&sender_id, &message,&new_seq_number);
        if(r<3)show_error("Scanf Message");

        //send message
        if(sender_id !=atoi(argv[1]) && new_seq_number<seq_number){

            for (int i = 2; i<argc; i++){
                
                if(sender_id == atoi(argv[i])){
                    printf ("Message read: %d\n", message);
                    fflush(stdout);

                    seq_number = new_seq_number+1;
                    sprintf(buffer, "%d:%d:%d", atoi(argv[1]), message, seq_number);
                    n = sendto(sock_fd, buffer, strlen(buffer), 0, (struct sockaddr *) &srv_addr, sizeof(srv_addr));
                    if(n<0)show_error("ERROR sendto");
                
                }
            }
        } 
    }
    
    n = close (sock_fd);
    if(n<0)show_error("Error close sock_fd");

    n = close(new_sock_fd);
    if(n<0)show_error("Error close new_sock_fd");


    return(1);
}   



