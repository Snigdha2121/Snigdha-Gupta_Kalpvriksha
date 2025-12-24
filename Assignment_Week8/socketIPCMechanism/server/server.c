#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>

#define SERVER_PORT 8080
#define ACCOUNT_FILE "../resource/accountDB.txt"

pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;

int isValidRequestType(int requestType) {
    return requestType >= 1 && requestType <= 4;
}

int isValidAmount(int amount) {
    return amount >= 0;
}

void *handleClientRequest(void *arg) {
    int clientSocket = *(int *)arg;
    free(arg);

    int requestType;
    int amount;
    int balance;
    FILE *accountFilePointer;

    do {
        if (read(clientSocket, &requestType, sizeof(int)) <= 0) {
            break;
        }

        if (read(clientSocket, &amount, sizeof(int)) <= 0) {
            break;
        }

        if (!isValidRequestType(requestType) || !isValidAmount(amount)) {
            balance = -1;
            write(clientSocket, &balance, sizeof(int));
            continue;
        }

        if (requestType == 4) {
            break;
        }

        pthread_mutex_lock(&fileMutex);

        accountFilePointer = fopen(ACCOUNT_FILE, "r");
        if (accountFilePointer == NULL) {
            pthread_mutex_unlock(&fileMutex);
            break;
        }

        fscanf(accountFilePointer, "%d", &balance);
        fclose(accountFilePointer);

        if (requestType == 1) {
            if (amount > balance) {
                balance = -1;
            } else {
                balance -= amount;
            }
        } else if (requestType == 2) {
            balance += amount;
        }

        if ((requestType == 1 && balance != -1) || requestType == 2) {
            accountFilePointer = fopen(ACCOUNT_FILE, "w");
            fprintf(accountFilePointer, "%d", balance);
            fclose(accountFilePointer);
        }

        pthread_mutex_unlock(&fileMutex);

        write(clientSocket, &balance, sizeof(int));

    } while (requestType != 4);

    close(clientSocket);
    return NULL;
}

int main() {
    int serverSocket;
    struct sockaddr_in serverAddress;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT);

    if (bind(serverSocket,
             (struct sockaddr *)&serverAddress,
             sizeof(serverAddress)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        exit(1);
    }

    if (listen(serverSocket, 5) < 0) {
        perror("Listen failed");
        close(serverSocket);
        exit(1);
    }

    printf("ATM Server running on port %d...\n", SERVER_PORT);

    while (1) {
        int *clientSocket = malloc(sizeof(int));
        *clientSocket = accept(serverSocket, NULL, NULL);

        if (*clientSocket < 0) {
            free(clientSocket);
            continue;
        }

        pthread_t clientThread;
        pthread_create(&clientThread, NULL, handleClientRequest, clientSocket);
        pthread_detach(clientThread);
    }

    close(serverSocket);
    return 0;
}