#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#define MAX_SIZE 100
#define INPUT_BUFFER_SIZE 512

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
    int parentArray[MAX_SIZE];
    int size = readArrayFromUser(parentArray);

    if (size == -1) {
        return 0;
    }

    printf("\nParent Process:\n");
    printf("Array before sorting: ");
    displayArray(parentArray, size);

    int parentToChildPipe[2];
    int childToParentPipe[2];

    if (pipe(parentToChildPipe) == -1 || pipe(childToParentPipe) == -1) {
        perror("Pipe creation failed");
        exit(1);
    }

    pid_t pid = fork();

    if (pid == 0) {
        close(parentToChildPipe[1]);
        close(childToParentPipe[0]);

        int childArray[MAX_SIZE];
        int receivedSize;

        read(parentToChildPipe[0], &receivedSize, sizeof(int));
        read(parentToChildPipe[0], childArray, receivedSize * sizeof(int));

        sortArray(childArray, receivedSize);

        write(childToParentPipe[1], &receivedSize, sizeof(int));
        write(childToParentPipe[1], childArray, receivedSize * sizeof(int));

        close(parentToChildPipe[0]);
        close(childToParentPipe[1]);
        exit(0);
    } else {
        close(parentToChildPipe[0]);
        close(childToParentPipe[1]);

        write(parentToChildPipe[1], &size, sizeof(int));
        write(parentToChildPipe[1], parentArray, size * sizeof(int));

        wait(NULL);

        int sortedArray[MAX_SIZE];
        int sortedSize;

        read(childToParentPipe[0], &sortedSize, sizeof(int));
        read(childToParentPipe[0], sortedArray, sortedSize * sizeof(int));

        printf("\nParent Process:\n");
        printf("Array after sorting: ");
        displayArray(sortedArray, sortedSize);

        close(parentToChildPipe[1]);
        close(childToParentPipe[0]);
    }

    return 0;
}