#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "lib/tdns/tdns-c.h"

/* A few macros that might be useful */
/* Feel free to add macros you want */
#define DNS_PORT 53
#define BUFFER_SIZE 2048 

/*
|---------------------NOTE---------------------|
| I have not used clinet_addr,                 |
| I don't know what the difference between     |
| that and server_addr is                      |
|---------------------NOTE---------------------|
*/

int main() {
    /* A few variable declarations that might be useful */
    /* You can add anything you want */
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    /* PART1 TODO: Implement a DNS nameserver for the utexas.edu zone */
    
    /* 1. Create an **UDP** socket */
    // socket ( IPv4, UPD, Protocol)
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    /* 2. Initialize server address (INADDR_ANY, DNS_PORT) */
    /* Then bind the socket to it */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(DNS_PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    /* 3. Initialize a server context using TDNSInit() */
    /* This context will be used for future TDNS library function calls */
    struct TDNSServerContext * ctx = TDNSInit(); 
    if (!ctx) {
        perror("TDNSInit failed\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    /* 4. Create the utexas.edu zone using TDNSCreateZone() */
    /* Add an IP address for www.utexas.edu domain using TDNSAddRecord() */
    /* Add the UTCS nameserver ns.cs.utexas.edu using using TDNSAddRecord() */
    /* Add an IP address for ns.cs.utexas.edu domain using TDNSAddRecord() */
    
    // creating utexas.edu zone
    TDNSCreateZone(ctx, "utexas.edu");
    // add an IP address for www.utexas.edu
    TDNSAddRecord(ctx, "utexas.edu", "www", "40.0.0.10", NULL);
    // add UTCS nameserver
    TDNSAddRecord(ctx, "utexas.edu", "cs", NULL, "ns.cs.utexas.edu");
    TDNSAddRecord(ctx, "cs.utexas.edu", "ns", "50.0.0.30", NULL);

    /* 5. Receive a message continuously and parse it using TDNSParseMsg() */
    while (1) {
        ssize_t n = recvfrom(sockfd,
                             buffer,
                             BUFFER_SIZE,
                             0,
                             (struct sockaddr*)&client_addr,
                             &client_len);
        if (n < 0) {
            perror("recvfrom");
            continue;
        }

        struct TDNSParseResult parsed;
        /* parse: 0==query, 1==response */
        if (TDNSParseMsg(buffer, n, &parsed) == TDNS_RESPONSE) {
            continue;  /* ignore incoming responses */
        }

        /* only handle A, AAAA, NS queries */
        if (parsed.qtype == A ||
            parsed.qtype == AAAA ||
            parsed.qtype == NS) {
            struct TDNSFindResult result;
            if (TDNSFind(ctx, &parsed, &result)) {
                /* send back the serialized DNS response */
                if (sendto(sockfd,
                           result.serialized,
                           result.len,
                           0,
                           (struct sockaddr*)&client_addr,
                           client_len) < 0) {
                    perror("sendto");
                }
            }
        }
        /* else: ignore unsupported query types */
    }

    /* 6. If it is a query for A, AAAA, NS DNS record */
    /* find the corresponding record using TDNSFind() and send the response back */
    /* Otherwise, just ignore it. */
    close(sockfd);
    return 0;
}

