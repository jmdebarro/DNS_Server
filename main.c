#include <stdio.h>
#include <sys/socket.h>

#define SUCCESS 0
#define FAILURE 1
#define UDP_PORT 53
#define GOOGLE_DNS "8.8.8.8"
#define BUFF_LENGTH 1024

/* 
How DNS requests will go
Laptop -> Pi -> Internet -> Pi -> Laptop

DHCP Configuration
*/

int main() {

    /*------------------- SOCKET SETUP --------------------*/
    struct sockaddr_in address;
    char buffer[BUFF_LENGTH];
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

    if (bind(socket_fd, &sockaddr_in, sizeof(sockaddr_in)) < 0) {
        perror("Bind Failed");
        exit(EXIT_FAILURE);
    }

    /*----------------- RECV / SND ------------------*/

    while (1) {
        if (recvfrom(socket_fd, (char *)buffer, BUFF_LENGTH, MSG_WAITALL,
                (struct sockaddr *) &address, sizeof(address)) > 0) {
                    perror("Recvfrom Error:");
        }

        // Check if request if from DNS provider out local device
        if (local_device_request(buffer)) {
            // From local device
            // Code for checking if request is on a blocked list
            if (check_blocklist(buffer)) {
                // Use Hash map and bloom filter
                // if on blacklist, return 0.0.0.0.0
            } else {
                // Pass data onward
            }
        } else {
            // Code for sending data back to local devices

        }
    }


    return SUCCESS;
}