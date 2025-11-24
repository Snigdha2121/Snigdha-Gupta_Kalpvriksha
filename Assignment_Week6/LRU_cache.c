#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define HASH_SIZE 2003

typedef struct QueueNode
{
    int Key;
    char *Value;
    struct QueueNode *Prev;
    struct QueueNode *Next;
} QueueNode;

typedef struct HashNode
{
    int Key;
    QueueNode *QNode;
    struct HashNode *Next;
} HashNode;

typedef struct LruCache
{
    int Capacity;
    int Size;
    QueueNode *Head;
    QueueNode *Tail;
    HashNode *Hash[HASH_SIZE];
} LruCache;

int getLength(const char *string)
{
    int length = 0;
    while (string[length] != '\0')
        length++;

    return length;
}

int compareString(const char *first, const char *second)
{
    int index = 0;
    while (first[index] != '\0' && second[index] != '\0')
    {
        char firstChar = tolower(first[index]);
        char secondChar = tolower(second[index]);
        if (firstChar != secondChar)
            return firstChar - secondChar;

        index++;
    }
    return tolower(first[index]) - tolower(second[index]);
}

void copyString(char *destination, const char *source)
{
    int index = 0;
    while (source[index] != '\0')
    {
        destination[index] = source[index];
        index++;
    }
    destination[index] = '\0';
}

char* duplicateString(const char *source)
{
    int length = getLength(source);
    char *allocatedString = (char *)malloc(length + 1);
    if (allocatedString == NULL)
    {
        printf("Memory allocation failed\n");
        return NULL;
    }
    copyString(allocatedString, source);
    return allocatedString;
}

int hashFunction(int key)
{
    if (key < 0)
        key = -key;

    return key % HASH_SIZE;
}

QueueNode *createQueueNode(int key, const char *value)
{
    QueueNode *queueNodePointer = (QueueNode *)malloc(sizeof(QueueNode));
    if (queueNodePointer == NULL)
    {
        printf("Memory allocation failed\n");
        return NULL;
    }
    queueNodePointer->Key = key;
    queueNodePointer->Value = duplicateString(value);
    if (queueNodePointer->Value == NULL)
    {
        free(queueNodePointer);
        return NULL;
    }
    queueNodePointer->Prev = NULL;
    queueNodePointer->Next = NULL;
    return queueNodePointer;
}

void removeQueueNode(LruCache *cachePointer, QueueNode *queueNodePointer)
{
    if (queueNodePointer->Prev != NULL)
        queueNodePointer->Prev->Next = queueNodePointer->Next;

    else
        cachePointer->Head = queueNodePointer->Next;

    if (queueNodePointer->Next != NULL)
        queueNodePointer->Next->Prev = queueNodePointer->Prev;

    else
        cachePointer->Tail = queueNodePointer->Prev;
}

void insertQueueNodeAtHead(LruCache *cachePointer, QueueNode *queueNodePointer)
{
    queueNodePointer->Prev = NULL;
    queueNodePointer->Next = cachePointer->Head;
    if (cachePointer->Head != NULL)
        cachePointer->Head->Prev = queueNodePointer;

    else
        cachePointer->Tail = queueNodePointer;

    cachePointer->Head = queueNodePointer;
}

HashNode *findHashNode(LruCache *cachePointer, int key)
{
    int hashIndex = hashFunction(key);
    HashNode *hashNodePointer = cachePointer->Hash[hashIndex];
    while (hashNodePointer != NULL)
    {
        if (hashNodePointer->Key == key)
            return hashNodePointer;

        hashNodePointer = hashNodePointer->Next;
    }
    return NULL;
}

void removeHashNode(LruCache *cachePointer, int key)
{
    int hashIndex = hashFunction(key);
    HashNode *currentNodePointer = cachePointer->Hash[hashIndex];
    HashNode *previousNodePointer = NULL;

    while (currentNodePointer != NULL)
    {
        if (currentNodePointer->Key == key)
        {
            if (previousNodePointer != NULL)
                previousNodePointer->Next = currentNodePointer->Next;

            else
                cachePointer->Hash[hashIndex] = currentNodePointer->Next;

            free(currentNodePointer);
            return;
        }
        previousNodePointer = currentNodePointer;
        currentNodePointer = currentNodePointer->Next;
    }
}

void addHashNode(LruCache *cachePointer, int key, QueueNode *queueNodePointer)
{
    int hashIndex = hashFunction(key);
    HashNode *newHashNodePointer = (HashNode *)malloc(sizeof(HashNode));
    if (newHashNodePointer == NULL)
    {
        printf("Memory allocation failed\n");
        return;
    } 
    newHashNodePointer->Key = key;
    newHashNodePointer->QNode = queueNodePointer;
    newHashNodePointer->Next = cachePointer->Hash[hashIndex];
    cachePointer->Hash[hashIndex] = newHashNodePointer;
}

LruCache *createLruCache(int capacity)
{
    LruCache *cachePointer = (LruCache *)malloc(sizeof(LruCache));
    if (cachePointer == NULL)
    {
        printf("Memory allocation failed\n");
        return NULL;
    }
    cachePointer->Capacity = capacity;
    cachePointer->Size = 0;
    cachePointer->Head = NULL;
    cachePointer->Tail = NULL;
    int hashArrayIndex = 0;
    while (hashArrayIndex < HASH_SIZE)
    {
        cachePointer->Hash[hashArrayIndex] = NULL;
        hashArrayIndex++;
    }
    return cachePointer;
}

char *getValueFromCache(LruCache *cachePointer, int key)
{
    HashNode *hashNodePointer = findHashNode(cachePointer, key);
    if (hashNodePointer == NULL)
        return NULL;

    QueueNode *queueNodePointer = hashNodePointer->QNode;
    removeQueueNode(cachePointer, queueNodePointer);
    insertQueueNodeAtHead(cachePointer, queueNodePointer);
    return queueNodePointer->Value;
}

void evictLeastRecentlyUsedNode(LruCache *cachePointer)
{
    QueueNode *leastRecentlyUsedNodePointer = cachePointer->Tail;
    if (leastRecentlyUsedNodePointer == NULL)
        return;

    removeHashNode(cachePointer, leastRecentlyUsedNodePointer->Key);
    removeQueueNode(cachePointer, leastRecentlyUsedNodePointer);
    free(leastRecentlyUsedNodePointer->Value);
    free(leastRecentlyUsedNodePointer);
    cachePointer->Size--;
}

void insertNewKeyValuePair(LruCache *cachePointer, int key, const char *value)
{
    QueueNode *newQueueNodePointer = createQueueNode(key, value);
    if (newQueueNodePointer == NULL)
    {
        printf("Memory allocation failed\n");
        return;
    }

    insertQueueNodeAtHead(cachePointer, newQueueNodePointer);
    addHashNode(cachePointer, key, newQueueNodePointer);
    cachePointer->Size++;
}

void updateQueueNodeValue(QueueNode *queueNodePointer, const char *value)
{
    char *newValue = duplicateString(value);
    if (newValue == NULL)
    {
        printf("Memory allocation failed\n");
        return;
    }
    free(queueNodePointer->Value);
    queueNodePointer->Value = newValue;
}

void putKeyValueInCache(LruCache *cachePointer, int key, const char *value)
{
    HashNode *hashNodePointer = findHashNode(cachePointer, key);
    if (hashNodePointer != NULL)
    {
        QueueNode *queueNodePointer = hashNodePointer->QNode;
        updateQueueNodeValue(queueNodePointer, value);
        removeQueueNode(cachePointer, queueNodePointer);
        insertQueueNodeAtHead(cachePointer, queueNodePointer);
        return;
    }
    if (cachePointer->Size == cachePointer->Capacity)
        evictLeastRecentlyUsedNode(cachePointer);

    insertNewKeyValuePair(cachePointer, key, value);
}

void discardRemainingInput()
{
    int inputCharacter = getchar();
    while (inputCharacter != '\n' && inputCharacter != EOF)
        inputCharacter = getchar();
}

int validInteger(int *destinationIntegerPointer)
{
    int readCount = scanf("%d", destinationIntegerPointer);
    if (readCount != 1)
    {
        printf("Invalid integer input\n");
        discardRemainingInput();
        return 0;
    }
    return 1;
}

int validString(char *destinationBuffer, int bufferSize)
{
    int readCount = scanf("%s", destinationBuffer);
    if (readCount != 1)
    {
        printf("Invalid string input\n");
        discardRemainingInput();
        return 0;
    }
    destinationBuffer[bufferSize - 1] = '\0';
    return 1;
}

int main()
{
    LruCache *cachePointer = NULL;
    char commandBuffer[32];
    char valueBuffer[256];
    int key;
    int capacity;

    while (1)
    {
        if (!validString(commandBuffer, sizeof(commandBuffer)))
            continue;

        if (compareString(commandBuffer, "createCache") == 0)
        {
            if (!validInteger(&capacity))
                continue;

            if (capacity <= 0 || capacity > 1000)
            {
                printf("Invalid cache capacity\n");
                continue;
            }
            cachePointer = createLruCache(capacity);
        }
        else if (compareString(commandBuffer, "put") == 0)
        {
            if (cachePointer == NULL)
            {
                printf("Cache not created\n");
                continue;
            }
            if (!validInteger(&key))
                continue;

            if (scanf(" %255[^\n]", valueBuffer) != 1)
            {
                printf("Invalid string input\n");
                discardRemainingInput();
                continue;
            }
            putKeyValueInCache(cachePointer, key, valueBuffer);
        }
        else if (compareString(commandBuffer, "get") == 0)
        {
            if (cachePointer == NULL)
            {
                printf("Cache not created\n");
                continue;
            }
            if (!validInteger(&key))
                continue;

            char *resultValuePointer = getValueFromCache(cachePointer, key);
            if (resultValuePointer != NULL)
                printf("%s\n", resultValuePointer);
            else
                printf("NULL\n");
        }
        else if (compareString(commandBuffer, "exit") == 0)
            break;

        else
        {
            printf("Unknown command\n");
            discardRemainingInput();
        }
    }
    return 0;
}