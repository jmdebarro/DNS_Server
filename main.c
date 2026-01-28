#include <stdio.h>
#include <sys/socket.h>
#include <time.h>
#include "hashmap.h"
#include "utils.h"


/* 
How DNS requests will go
Laptop -> Pi -> Internet -> Pi -> Laptop

DHCP Configuration
*/

int main() {
    srand(time(NULL));
    hashmap table = init_hash_table();

    /*------------------- SOCKET SETUP --------------------*/
    struct sockaddr_storage address;
    unsigned char buffer[BUFF_LENGTH];
    int socket_fd;
    int opt = 1;

    if ((socket_fd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
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

    address.sin6_family = AF_INET6;
    address.sin6_port = htons(UDP_PORT);
    address.sin6_addr.s_addr = INADDR_ANY;

    socklen_t addr_len = sizeof(address);
    if (bind(socket_fd, (struct sockaddr *)&address, sizeof(addr_len)) < 0) {
        perror("Bind Failed");
        exit(EXIT_FAILURE);
    }

    /*----------------- RECV / SND ------------------*/

    while (1) {
        ssize_t bytes_received = recvfrom(socket_fd, buffer, BUFF_LENGTH, MSG_WAITALL,
                (struct sockaddr *) &address, sizeof(address));

        if (bytes_received< 0) {
                    perror("Recvfrom Error:");
        }
        
        DNSQuery *dns_header = (DNSQuery *)buffer;
        if ((dns_header->flags & 0x8000) == 0) {
            // value is a request as flag is not set
            char *domain = extract_domain_from_query(buffer);
            if (table_lookup(table, domain) == SUCCESS) {
                modify_blocked_domain_buffer(buffer, bytes_received);
                ssize_t bytes_sent = sendto(socket_fd, buffer, bytes_received + BYTES_ADDED, 0,
                     (struct sockaddr *) &address, sizeof(addr_len))
                
            } else {
                // Generate a port between 49152 and 65535
                int random_port = 49152 + (rand() % (65535 - 49152 + 1));
                // forward response to dns provider
                // Wait for response on high port
                // send response to laptop
            }
        }
    }

    return SUCCESS;
}