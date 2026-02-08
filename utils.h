#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#define SUCCESS 0
#define FAILURE 1
#define UDP_PORT 53
#define GOOGLE_DNS "8.8.8.8"
#define BUFF_LENGTH 1024
#define DOMAIN_BEGIN 12
#define IPv6_BYTE_SIZE 28
#define IPv4_BYTE_SIZE 16
#define TRUE 1
#define FALSE 0

typedef struct DNSHeader {
    uint16_t id;
    uint16_t flags;
} DNSHeader;

char *extract_domain_from_query(unsigned char buffer[BUFF_LENGTH]) {
    // Formats 0x03 w w w 0x06 g o o g l e .... into www.google.com and returns pointer
    static char domain_out[256]; 
    unsigned int src = DOMAIN_BEGIN;
    unsigned int dst = 0;

    while (buffer[src] != 0 && src < BUFF_LENGTH && dst < 255) {
        unsigned int label_len = buffer[src++]; 
        
        for (unsigned int i = 0; i < label_len && src < BUFF_LENGTH; i++) {
            domain_out[dst++] = buffer[src++];
        }

        // If the next byte isn't the end of the domain, add a dot
        if (buffer[src] != 0) {
            domain_out[dst++] = '.';
        }
    }
    domain_out[dst] = '\0';
    return domain_out;
}

uint16_t get_dns_question_type(unsigned char *buffer, ssize_t bytes_received) {
    int pos = DOMAIN_BEGIN; // Skip header
    // Walk through the labels
    while (buffer[pos] != 0 && pos < bytes_received) {
        pos += buffer[pos] + 1; // Walks through domain 06 g o o g l e 03 c o m 00
    }
    pos++; // Skip the null terminator (0x00)

    // Check if outside of bounds
    if (pos + 4 > bytes_received) return 0;
    
    // The next 2 bytes are the Type (0x0001 for A - 0x001C for AAAA)
    // Shift over bytes and return an int as to not ignore the rest of the type message
    return (uint16_t)((buffer[pos] << 8) | buffer[pos + 1]);
}

ssize_t modify_blocked_domain_buffer(unsigned char buffer[BUFF_LENGTH], ssize_t bytes_received) {
    // https://www.geeksforgeeks.org/computer-networks/dns-message-format/
    // http://www.tcpipguide.com/free/t_DNSMessageHeaderandQuestionSectionFormat.htm
    //  http://www.tcpipguide.com/free/t_DNSNameServerDataStorageResourceRecordsandClasses-3.htm#Table_166
    uint16_t qtype = get_dns_question_type(buffer, bytes_received);

    // Sets Response Flags
    buffer[2] = 0x84;
    buffer[3] = 0x00;
    // Questions being asked, 1
    buffer[4] = 0x00; 
    buffer[5] = 0x01; 
    buffer[6] = 0x00; 
    buffer[7] = 0x01; 
    // Clears additional content field
    memset(&buffer[8], 0, 4);

    int pos = DOMAIN_BEGIN; // Skip header
    while (buffer[pos] != 0 && pos < bytes_received) {
        pos += buffer[pos] + 1; // Walks through domain, ex. 06 g o o g l e 03 c o m 00
    }
    pos += 1 + 4; // Skip the null terminator (0x00) and then Type and Class of response, 2 bytes each
    unsigned char *ptr = &buffer[pos];
    unsigned int dns_response_size = pos; // Updates current size of response message to this point
    if ((pos + 28) >= BUFF_LENGTH) {
        return -1;
    }

    ptr[0] = 0xC0;  // Identifies this as a pointer, not a string
    ptr[1] = 0x0C;  // Address 12, where the name is stored from client DNS request
    ptr[2] = 0x00;  // Identifies type of repsosne, which is an address
    // ptr[3] specific ot ipv, below
    ptr[4] = 0x00;  // Identifies class of resource, typically always 1 for Internet ("IN")
    ptr[5] = 0x01;
    ptr[6] = 0x00;  // specifies Time To Live (TTL)
    ptr[7] = 0x00;  // How long a record should be retained, in seconds
    ptr[8] = 0x00;  // Typically 60 seconds
    ptr[9] = 0x3C;  // Not maxed out as would take forever to refresh
    if (qtype == 0x1C) { // AAAA (IPv6)
        ptr[3] = 0x1C; // Type AAAA
        ptr[10] = 0x00; ptr[11] = 0x10; // Data Length 16 bytes
        memset(&ptr[12], 0, 16);        // IPv6 "::" address
        return (ssize_t)(dns_response_size + 12 + 16); // size up until answer section + answer flags + answer ip length
    } else if (qtype == 0x01) { // Default to A (IPv4)
        ptr[3] = 0x01; // Type A
        ptr[10] = 0x00; ptr[11] = 0x04; // Data Length 4 bytes
        memset(&ptr[12], 0, 4);         // IPv4 "0.0.0.0" address
        return (ssize_t)(dns_response_size + 12 + 4); // size up until answer section + answer flags + answer ip length
    } else {
        return -1;
    }
}