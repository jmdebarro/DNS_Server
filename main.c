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
    struct sockaddr_in6 server_address;
    struct sockaddr_storage server_address;
    unsigned char buffer[BUFF_LENGTH];
    int socket_fd;
    int opt = 1;

    if ((socket_fd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    int off = 0;
    // Turn off IPv6 only to allow for IPv4 connections
    setsockopt(socket_fd, IPPROTO_IPV6, IPV6_V6ONLY, &off, sizeof(off));
    // Forcefully attaching socket to the port 53
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    server_address.sin6_family = AF_INET6;
    server_address.sin6_port = htons(UDP_PORT);
    server_address.sin6_addr.s_addr = in6addre_any;

    socklen_t addr_len = sizeof(address);
    if (bind(socket_fd, (struct sockaddr *)&address, sizeof(addr_len)) < 0) {
        perror("Bind Failed");
        exit(EXIT_FAILURE);
    }

    /*----------------- RECV / SND ------------------*/

    while (1) {
        socklen_t addr_len = sizeof(client_addr);
        ssize_t bytes_received = recvfrom(socket_fd, buffer, BUFF_LENGTH, MSG_WAITALL,
                (struct sockaddr *) &address, &addr_len);

        if (bytes_received< 0) {
                    perror("Recvfrom Error:");
        }
        
        DNSQuery *dns_header = (DNSQuery *)buffer;
        if ((dns_header->flags & 0x8000) == 0) {
            // value is a request as flag is not set
            char *domain = extract_domain_from_query(buffer);
            if (table_lookup(table, domain) == SUCCESS) {
                ssize_t bytes_to_send = modify_blocked_domain_buffer(buffer, bytes_received);
                ssize_t bytes_sent = sendto(socket_fd, buffer, bytes_to_send, 0,
                     (struct sockaddr *) &client_address, sizeof(addr_len));
                if (bytes_sent < 0) {
                    perror("Error Sending Bytes");
                    exit(EXIT_FAILURE);
                }
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