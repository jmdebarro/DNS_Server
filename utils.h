#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <arpa/inet.h>

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
    unsigned int index = DOMAIN_BEGIN;
    int first = TRUE;
    while (buffer[index] != 0) {
        unsigned int string_len = buffer[index];
        if (first == FALSE) {
            buffer[index] = '.';
        }
        index += string_len;
        first = TRUE;
    }

    return (char *)(buffer + DOMAIN_BEGIN + 1);
}

uint16_t get_dns_question_type(unsigned char *buffer, ssize_t bytes_received) {
    int pos = 12; // Skip header
    // Walk through the labels
    while (buffer[pos] != 0 && pos < bytes_received) {
        pos += buffer[pos] + 1; // Walks through domain 06 g o o g l e 03 c o m 00
    }
    pos++; // Skip the null terminator (0x00)
    
    // The next 2 bytes are the Type (e.g., 0x0001 for A, 0x001C for AAAA)
    return (buffer[pos] << 8) | buffer[pos + 1];
}

ssize_t modify_blocked_domain_buffer(unsigned char buffer[BUFF_LENGTH], ssize_t bytes_received) {
    // https://www.geeksforgeeks.org/computer-networks/dns-message-format/
    // http://www.tcpipguide.com/free/t_DNSMessageHeaderandQuestionSectionFormat.htm
    //  http://www.tcpipguide.com/free/t_DNSNameServerDataStorageResourceRecordsandClasses-3.htm#Table_166
    uint16_t qtype = get_dns_question_type(buffer, bytes_received);

    // Sets response flag and Authority
    buffer[2] =( buffer[2] & 0x81) | 0x81;
    buffer[3] = (buffer[3] & 0x80) | 0x80;
    buffer[7] = (buffer[7] & 0x01) | 0x01;

    unsigned char *ptr = &buffer[bytes_received];
    ptr[0] = 0xC0;  // Identifies this as a pointer, not a string
    ptr[1] = 0x0C;  // Address 12, where the name is stored from client DNS request
    ptr[2] = 0x00;  // Identifies type of repsosne, which is an address
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
        return bytes_received + 28;     // Answer is 28 bytes total
    } else { // Default to A (IPv4)
        ptr[3] = 0x01; // Type A
        ptr[10] = 0x00; ptr[11] = 0x04; // Data Length 4 bytes
        memset(&ptr[12], 0, 4);         // IPv4 "0.0.0.0" address
        return bytes_received + 16;
    }
}