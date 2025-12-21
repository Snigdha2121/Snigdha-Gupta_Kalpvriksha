#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#define MAX_SIZE 100
#define INPUT_BUFFER_SIZE 512

struct sharedMemoryContainer {
    int arraySize;
    int integerArray[MAX_SIZE];
};

int readSingleValidatedInteger() {
    char buffer[INPUT_BUFFER_SIZE];
    char *endPointer;
    long value;

    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            continue;
        }

        value = strtol(buffer, &endPointer, 10);

        while (isspace(*endPointer)) {
            endPointer++;
        }

        if (*endPointer == '\0' || *endPointer == '\n') {
            return (int)value;
        }

        printf("Invalid input. Enter an integer again: ");
    }
}

int readValidatedArraySize() {
    int size;

    printf("Enter number of elements: ");
    size = readSingleValidatedInteger();

    if (size <= 0 || size > MAX_SIZE) {
        printf("Invalid array size\n");
        return -1;
    }

    return size;
}

int parseExactIntegers(char *inputLine, int expectedCount, int array[]) {
    char *currentPointer = inputLine;
    char *endPointer;
    int count = 0;

    while (*currentPointer != '\0') {
        while (isspace(*currentPointer)) {
            currentPointer++;
        }

        if (*currentPointer == '\0' || *currentPointer == '\n') {
            break;
        }

        long value = strtol(currentPointer, &endPointer, 10);

        if (currentPointer == endPointer) {
            return 0;
        }

        if (count < expectedCount) {
            array[count] = (int)value;
        }

        count++;
        currentPointer = endPointer;
    }

    return count == expectedCount;
}

int readArrayFromUser(int array[]) {
    int size = readValidatedArraySize();
    if (size == -1) {
        return -1;
    }

    while (1) {
        char buffer[INPUT_BUFFER_SIZE];

        printf("Enter %d integers separated by space:\n", size);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            continue;
        }

        if (parseExactIntegers(buffer, size, array)) {
            return size;
        }

        printf("Invalid input. Exactly %d integers are required. Please re-enter.\n", size);
    }
}

void displayArray(int array[], int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
}

void sortArray(int array[], int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (array[j] > array[j + 1]) {
                int temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }
}

int main() {
    key_t sharedMemoryKey = ftok(".", 80);
    int sharedMemoryId = shmget(sharedMemoryKey,
                               sizeof(struct sharedMemoryContainer),
                               0666 | IPC_CREAT);

    if (sharedMemoryId == -1) {
        perror("Shared memory creation failed");
        exit(1);
    }

    struct sharedMemoryContainer *sharedMemory =
        (struct sharedMemoryContainer *)shmat(sharedMemoryId, NULL, 0);

    if (sharedMemory == (void *)-1) {
        perror("Shared memory attach failed");
        exit(1);
    }

    sharedMemory->arraySize =
        readArrayFromUser(sharedMemory->integerArray);

    if (sharedMemory->arraySize == -1) {
        shmdt(sharedMemory);
        shmctl(sharedMemoryId, IPC_RMID, NULL);
        return 0;
    }

    printf("\nParent Process:\n");
    printf("Array before sorting: ");
    displayArray(sharedMemory->integerArray, sharedMemory->arraySize);

    pid_t pid = fork();

    if (pid == 0) {
        sortArray(sharedMemory->integerArray,
                  sharedMemory->arraySize);
        exit(0);
    } else {
        wait(NULL);

        printf("\nParent Process:\n");
        printf("Array after sorting: ");
        displayArray(sharedMemory->integerArray,
                     sharedMemory->arraySize);

        shmdt(sharedMemory);
        shmctl(sharedMemoryId, IPC_RMID, NULL);
    }

    return 0;
}