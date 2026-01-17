#include <stdio.h>
#include <sys/socket.h>
#include "secrets.h"

#define SUCCESS 0
#define FAILURE 1
#define UDP_PORT 53

/* 
Redirect DNS UDP to PI
Block ads
Send back to router to all devices on network
*/

int main() {
    struct sockaddr_in address;
    int socket_fd;
    int opt = 1;

    if (socket_fd = socket(AF_INET, SOCK_DGRAM, 0) < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 53
    if (setsockopt(socket_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_port(UDP_PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    return SUCCESS;
}