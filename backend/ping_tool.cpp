#include "ping_tool.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <string>

#define PING_PKT_SIZE 64
#define ICMP_HEADER_LEN 8
#define TIMEOUT 1

// calculate the checksum for the ICMP packet
unsigned short checksum(void *b, int len) {
    unsigned short *buf = (unsigned short *)b;
    unsigned int sum = 0;
    unsigned short result;

    // calculate sum of all 16-bit words
    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;

    // add remaining byte if exists
    if (len == 1)
        sum += *(unsigned char *)buf;

    // fold 32-bit sum to 16 bits
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

// create an ICMP Echo Request packet
void create_ping_packet(char *packet, int pid) {
    struct icmp *icmp_hdr = (struct icmp *) packet;
    icmp_hdr->icmp_type = ICMP_ECHO; // ICMP Echo Request
    icmp_hdr->icmp_code = 0;
    icmp_hdr->icmp_id = pid; // identifier
    icmp_hdr->icmp_seq = 1;
    icmp_hdr->icmp_cksum = 0; // initial checksum
    icmp_hdr->icmp_cksum = checksum(icmp_hdr, sizeof(struct icmp));
}

// Ping function that returns a string response
std::string pingTool(const std::string& host) {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if (sockfd < 0) {
        return "Error: Unable to create socket";
    }

    struct sockaddr_in target_addr;
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = 0;
    target_addr.sin_addr.s_addr = inet_addr(host.c_str());

    // set socket timeout
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &timeout, sizeof(timeout)) < 0) {
        close(sockfd);
        return "Error: Unable to set socket timeout";
    }

    char packet[PING_PKT_SIZE];
    int pid = getpid(); // get process ID as the identifier

    // send ICMP request
    create_ping_packet(packet, pid);
    int sent_bytes = sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&target_addr, sizeof(target_addr));
    if (sent_bytes < 0) {
        close(sockfd);
        return "Error: Unable to send ICMP packet";
    }

    char buffer[1024];
    struct sockaddr_in from_addr;
    socklen_t addr_len = sizeof(from_addr);

    // wait for a reply
    int recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&from_addr, &addr_len);
    if (recv_len < 0) {
        close(sockfd);
        return "Error: Timeout waiting for reply";
    }

    struct ip *ip_hdr = (struct ip *)buffer;
    struct icmp *icmp_hdr = (struct icmp *)(buffer + (ip_hdr->ip_hl << 2));

    // Check if it's an ICMP Echo Reply
    if (icmp_hdr->icmp_type == ICMP_ECHOREPLY) {
        close(sockfd);
        return "Reply received from " + host;
    } else {
        close(sockfd);
        return "Received unknown ICMP response from " + host;
    }
}