#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>

#define SERVER_PORT 8080

int readValidatedInteger() {
    char buffer[100];
    char *endPointer;
    long value;

    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            continue;
        }

        value = strtol(buffer, &endPointer, 10);

        while (*endPointer == ' ' || *endPointer == '\t') {
            endPointer++;
        }

        if (*endPointer == '\n' || *endPointer == '\0') {
            return (int)value;
        }

        printf("Invalid input. Enter a valid integer: ");
    }
}


int main() {
    int clientSocket;
    int requestType;
    int amount;
    int updatedBalance;
    struct sockaddr_in serverAddress;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("Socket creation failed");
        return 1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (connect(clientSocket,
                (struct sockaddr *)&serverAddress,
                sizeof(serverAddress)) < 0) {
        perror("Connection failed");
        close(clientSocket);
        return 1;
    }

    do {
        amount = 0;

        printf("\n1. Withdraw\n2. Deposit\n3. Display Balance\n4. Exit\n");
        printf("Enter choice: ");
        requestType = readValidatedInteger();

        if (requestType == 1 || requestType == 2) {
            printf("Enter amount: ");
            amount = readValidatedInteger();
        }

        write(clientSocket, &requestType, sizeof(int));
        write(clientSocket, &amount, sizeof(int));

        if (requestType == 4) {
            break;
        }

        if (read(clientSocket, &updatedBalance, sizeof(int)) <= 0) {
            printf("Server disconnected\n");
            break;
        }

        if (updatedBalance == -1) {
            printf("Transaction failed (insufficient balance or invalid request)\n");
        } else {
            printf("Current Balance: %d\n", updatedBalance);
        }

    } while (requestType != 4);

    close(clientSocket);
    printf("Client exited successfully.\n");
    return 0;
}