#include "hashmap.h"
#include "utils.h"

// Signal handler to close socket for unexpected shutdown
int socket_fd; 
void handle_sigint(int sig) {
    printf("\nReceived %d\nShutting down DNS server...\n", sig);
    close(socket_fd);
    exit(0);
}

int main() {
    srand(time(NULL));
    signal(SIGINT, handle_sigint);
    hashmap table = init_hash_table();
    print_table(table);

    /*------------------- SOCKET SETUP --------------------*/
    struct sockaddr_in6 server_address;
    struct sockaddr_storage client_address;
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
    server_address.sin6_addr = in6addr_any;

    socklen_t addr_len = sizeof(server_address);
    if (bind(socket_fd, (struct sockaddr *) &server_address, addr_len) < 0) {
        perror("Bind Failed");
        exit(EXIT_FAILURE);
    }

    /*----------------- RECV / SND ------------------*/

    while (1) {
        printf("Waiting for query. . .\n");
        memset(buffer, 0, BUFF_LENGTH);
        addr_len = sizeof(client_address);
        ssize_t bytes_received = recvfrom(socket_fd, buffer, BUFF_LENGTH, MSG_WAITALL,
                (struct sockaddr *)&client_address, &addr_len);

        if (bytes_received< 0) {
                    perror("Recvfrom Error:");
        }
        
        DNSHeader *dns_header = (DNSHeader *)buffer;
        if ((dns_header->flags & 0x8000) == 0) {
            // value is a request as flag is not set
            char *domain = extract_domain_from_query(buffer);
            printf("[DOMAIN] - %s\n", domain);
            if (table_lookup(table, domain) == SUCCESS) {
                printf("[BLOCKED] - %s\n", domain);
                 ssize_t bytes_to_send = modify_blocked_domain_buffer(buffer, bytes_received);
                if (bytes_to_send < 0) {
                    perror("Cannot handle Type of request. Not 0x01 or 0x1C\n");
                    continue;
                }
                // Send DNS response to query with buffer formatted based on blocklist logic
                ssize_t bytes_sent = sendto(socket_fd, (void *)buffer, bytes_to_send, 0,
                    (struct sockaddr *)&client_address, addr_len);
                    
                if (bytes_sent < 0) {
                    perror("Error Sending Bytes");
                    continue;
                }

            } else {
                printf("[ACCEPTED] - %s\n", domain);
                // Generate socket to forward request
                int upstream_sock;
                if ((upstream_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                        perror("Failed to create socket");
                        continue;
                }
                
                // Create timeout
                struct timeval tv;
                tv.tv_sec = 2; // 2 seconds
                tv.tv_usec = 0;
                setsockopt(upstream_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

                // Google's address
                struct sockaddr_in google_addr;
                google_addr.sin_family = AF_INET;
                google_addr.sin_port = htons(53);
                inet_pton(AF_INET, GOOGLE_DNS, &google_addr.sin_addr);

                // Send dns request to Google
                ssize_t upstream_bytes_sent = sendto(
                    upstream_sock, (void *)buffer, bytes_received, 0,
                    (struct sockaddr *)&google_addr, sizeof(google_addr)
                );

                if (upstream_bytes_sent < 0) {
                    perror("Error forwarding request to Google DNS 8.8.8.8");
                    continue;
                }
                
                struct sockaddr_in from_google;
                socklen_t google_len = sizeof(from_google);

                // Response from Google
                ssize_t google_bytes_to_send = recvfrom(
                    upstream_sock, buffer, BUFF_LENGTH, 0,
                    (struct sockaddr *)&from_google, &google_len
                );

                if (google_bytes_to_send < 0) {
                    perror("Error receiving request from Google DNS server");
                    continue;
                }

                // Send DNS response to query with buffer formatted based on blocklist logic
                ssize_t bytes_sent = sendto(
                    socket_fd, (void *)buffer, google_bytes_to_send, 0,
                    (struct sockaddr *)&client_address, addr_len
                );
                close(upstream_sock);

                if (bytes_sent < 0) {
                    perror("Error Sending Bytes");
                    continue;
                }
            }
        }
    }
    close(socket_fd);
    return SUCCESS;
}