#include <stdio.h>
#include <sys/socket.h>

#define SUCCESS 0
#define FAILURE 1
#define PORT 53

int main() {
    int socket_fd;

    if (socket_fd = socket(AF_INET, SOCK_DGRAM, 0) < 0) {
        printf("Failed to create socket");
        return FAILURE;
    }

    return SUCCESS;
}