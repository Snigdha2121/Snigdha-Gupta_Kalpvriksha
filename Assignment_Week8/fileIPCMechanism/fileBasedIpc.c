#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#define MAX_SIZE 100
#define FILE_NAME "sharedData.txt"
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

int parseExactNumberOfIntegers(char *inputLine, int expectedCount, int array[]) {
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
        char inputBuffer[INPUT_BUFFER_SIZE];

        printf("Enter %d integers separated by space:\n", size);

        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL) {
            continue;
        }

        if (parseExactNumberOfIntegers(inputBuffer, size, array)) {
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

void writeArrayToFile(int array[], int size) {
    FILE *filePointer = fopen(FILE_NAME, "w");
    if (filePointer == NULL) {
        perror("File open failed");
        exit(1);
    }

    fprintf(filePointer, "%d\n", size);
    for (int i = 0; i < size; i++) {
        fprintf(filePointer, "%d ", array[i]);
    }

    fclose(filePointer);
}

int readArrayFromFile(int array[]) {
    FILE *filePointer = fopen(FILE_NAME, "r");
    if (filePointer == NULL) {
        perror("File open failed");
        exit(1);
    }

    int size;
    fscanf(filePointer, "%d", &size);

    for (int i = 0; i < size; i++) {
        fscanf(filePointer, "%d", &array[i]);
    }

    fclose(filePointer);
    return size;
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
    int array[MAX_SIZE];
    int size = readArrayFromUser(array);

    if (size == -1) {
        return 0;
    }

    printf("\nParent Process:\n");
    printf("Array before sorting: ");
    displayArray(array, size);

    writeArrayToFile(array, size);

    pid_t pid = fork();

    if (pid == 0) {
        int childArray[MAX_SIZE];
        int childSize = readArrayFromFile(childArray);
        sortArray(childArray, childSize);
        writeArrayToFile(childArray, childSize);
        exit(0);
    } else {
        wait(NULL);

        int sortedArray[MAX_SIZE];
        int sortedSize = readArrayFromFile(sortedArray);

        printf("\nParent Process:\n");
        printf("Array after sorting: ");
        displayArray(sortedArray, sortedSize);
    }

    return 0;
}