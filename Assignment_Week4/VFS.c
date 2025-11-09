#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 512
#define MAX_NAME 50
#define MAX_BLOCKS 1024

int getLength(char *string)
{
    int length = 0;
    if (string == NULL)
        return 0;

    while (string[length] != '\0')
    {
        length++;
    }
    return length;
}

int compareString(char *first, char *second)
{
    int index = 0;
    while (first[index] && second[index])
    {
        if (first[index] != second[index])
            return first[index] - second[index];

        index++;
    }
    return first[index] - second[index];
}

void copyString(char *destination, char *source)
{
    int index = 0;
    while (source && source[index])
    {
        destination[index] = source[index];
        index++;
    }
    destination[index] = '\0';
}

typedef struct FreeBlock
{
    int index;
    struct FreeBlock *next;
    struct FreeBlock *previous;
} FreeBlock;

typedef struct FileNode
{
    char name[MAX_NAME + 1];
    int isDirectory;
    int size;
    int *blockList;
    int blockCount;
    struct FileNode *parent;
    struct FileNode *child;
    struct FileNode *next;
    struct FileNode *previous;
} FileNode;

unsigned char *diskMemory;
FreeBlock *freeBlockHead = NULL, *freeBlockTail = NULL;
FileNode *rootDirectory = NULL, *currentDirectory = NULL;
int totalBlockCount = MAX_BLOCKS;

void addFreeBlock(int blockIndex)
{
    FreeBlock *newBlock = malloc(sizeof(FreeBlock));
    if (newBlock == NULL)
    {
        printf("Memory allocation error\n");
        return;
    }
    newBlock->index = blockIndex;
    newBlock->next = NULL;
    newBlock->previous = freeBlockTail;
    if (freeBlockTail)
        freeBlockTail->next = newBlock;
    else
        freeBlockHead = newBlock;
    freeBlockTail = newBlock;
}

int getFreeBlock()
{
    if (freeBlockHead == NULL)
        return -1;
    int blockIndex = freeBlockHead->index;
    FreeBlock *temporary = freeBlockHead;
    freeBlockHead = freeBlockHead->next;
    if (freeBlockHead)
        freeBlockHead->previous = NULL;
    else
        freeBlockTail = NULL;
    free(temporary);
    return blockIndex;
}

int countFreeBlocks()
{
    int count = 0;
    FreeBlock *iterator = freeBlockHead;
    while (iterator)
    {
        count++;
        iterator = iterator->next;
    }
    return count;
}

FileNode *createFileNode(char *name, int isDirectory, FileNode *parent)
{
    FileNode *node = malloc(sizeof(FileNode));
    if (node == NULL)
    {
        printf("Memory allocation error\n");
        exit(1);
    }
    copyString(node->name, name);
    node->isDirectory = isDirectory;
    node->size = 0;
    node->blockList = NULL;
    node->blockCount = 0;
    node->child = NULL;
    node->next = NULL;
    node->previous = NULL;
    node->parent = parent;
    return node;
}

FileNode *findChild(FileNode *directory, char *childName)
{
    if (directory == NULL || directory->child == NULL)
        return NULL;
    FileNode *currentChild = directory->child;
    do
    {
        if (compareString(currentChild->name, childName) == 0)
            return currentChild;
        currentChild = currentChild->next;
    } while (currentChild != directory->child);
    return NULL;
}

void addChild(FileNode *directory, FileNode *childNode)
{
    if (directory->child == NULL)
    {
        directory->child = childNode;
        childNode->next = childNode->previous = childNode;
    }
    else
    {
        FileNode *firstChild = directory->child;
        FileNode *lastChild = firstChild->previous;
        lastChild->next = childNode;
        childNode->previous = lastChild;
        childNode->next = firstChild;
        firstChild->previous = childNode;
    }
    childNode->parent = directory;
}

void removeChild(FileNode *directory, FileNode *childNode)
{
    if (directory == NULL || childNode == NULL)
        return;
    if (childNode->next == childNode)
        directory->child = NULL;
    else
    {
        if (directory->child == childNode)
            directory->child = childNode->next;
        childNode->previous->next = childNode->next;
        childNode->next->previous = childNode->previous;
    }
}

void initializeDisk(int numberOfBlocks)
{
    if (numberOfBlocks < 1 || numberOfBlocks > 5000)
        numberOfBlocks = MAX_BLOCKS;
    totalBlockCount = numberOfBlocks;
    diskMemory = malloc(BLOCK_SIZE * totalBlockCount);
    for (int i = 0; i < totalBlockCount; i++)
        addFreeBlock(i);
    rootDirectory = createFileNode("/", 1, NULL);
    currentDirectory = rootDirectory;
}

void clearFileTree(FileNode *node)
{
    if (node == NULL)
        return;
    if (node->child)
    {
        FileNode *child = node->child;
        FileNode *iterator = child;
        do
        {
            FileNode *nextChild = iterator->next;
            clearFileTree(iterator);
            iterator = nextChild;
        } while (iterator != child);
    }
    if (!node->isDirectory && node->blockList)
        free(node->blockList);
    free(node);
}

void cleanupFileSystem()
{
    clearFileTree(rootDirectory);
    while (freeBlockHead)
    {
        FreeBlock *temp = freeBlockHead;
        freeBlockHead = freeBlockHead->next;
        free(temp);
    }
    if (diskMemory)
        free(diskMemory);
}

void createDirectory(char *name)
{
    if (!name || getLength(name) == 0)
    {
        printf("Invalid name\n");
        return;
    }
    if (findChild(currentDirectory, name))
    {
        printf("Directory already exists\n");
        return;
    }
    FileNode *newDirectory = createFileNode(name, 1, currentDirectory);
    addChild(currentDirectory, newDirectory);
    printf("Directory '%s' created successfully.\n", name);
}

void createFile(char *name)
{
    if (name == NULL || getLength(name) == 0)
    {
        printf("Invalid name\n");
        return;
    }
    if (findChild(currentDirectory, name))
    {
        printf("File already exists\n");
        return;
    }
    FileNode *newFile = createFileNode(name, 0, currentDirectory);
    addChild(currentDirectory, newFile);
    printf("File '%s' created successfuly.\n", name);
}

void writeFile(char *name, char *content)
{
    FileNode *file = findChild(currentDirectory, name);
    if (!file)
    {
        printf("File not found\n");
        return;
    }
    if (file->isDirectory)
    {
        printf("Cannot write to a directory\n");
        return;
    }
    int length = getLength(content);
    int blocksNeeded = (length + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (countFreeBlocks() < blocksNeeded)
    {
        printf("Not enough free space\n");
        return;
    }

    int *allocatedBlocks = malloc(sizeof(int) * blocksNeeded);
    for (int i = 0; i < blocksNeeded; i++)
    {
        int blockIndex = getFreeBlock();
        if (blockIndex < 0)
        {
            printf("Block allocation error\n");
            free(allocatedBlocks);
            return;
        }
        allocatedBlocks[i] = blockIndex;
        int offset = i * BLOCK_SIZE;
        for (int j = 0; j < BLOCK_SIZE; j++)
        {
            int position = offset + j;
            diskMemory[blockIndex * BLOCK_SIZE + j] = (position < length) ? content[position] : 0;
        }
    }

    for (int i = 0; i < file->blockCount; i++)
        addFreeBlock(file->blockList[i]);
    if (file->blockList)
        free(file->blockList);

    file->blockList = allocatedBlocks;
    file->blockCount = blocksNeeded;
    file->size = length;
    printf("Data written successfully (size =%d bytes).\n", length - 1);
}

void readFile(char *name)
{
    FileNode *file = findChild(currentDirectory, name);
    if (!file)
    {
        printf("File not found\n");
        return;
    }
    if (file->isDirectory)
    {
        printf("Cannot read a directory\n");
        return;
    }
    int remainingBytes = file->size;
    for (int i = 0; i < file->blockCount && remainingBytes > 0; i++)
    {
        int blockIndex = file->blockList[i];
        int bytesToRead = (remainingBytes < BLOCK_SIZE) ? remainingBytes : BLOCK_SIZE;
        for (int j = 0; j < bytesToRead; j++)
            putchar(diskMemory[blockIndex * BLOCK_SIZE + j]);
        remainingBytes -= bytesToRead;
    }
    putchar('\n');
}

void deleteFile(char *name)
{
    FileNode *file = findChild(currentDirectory, name);
    if (!file)
    {
        printf("File not found\n");
        return;
    }
    if (file->isDirectory)
    {
        printf("Not a file\n");
        return;
    }
    for (int i = 0; i < file->blockCount; i++)
        addFreeBlock(file->blockList[i]);
    if (file->blockList)
        free(file->blockList);
    removeChild(currentDirectory, file);
    free(file);
    printf("File deleted successfully\n");
}

void removeDirectory(char *name)
{
    FileNode *directory = findChild(currentDirectory, name);
    if (!directory)
    {
        printf("Directory not found\n");
        return;
    }
    if (!directory->isDirectory)
    {
        printf("Not a directory\n");
        return;
    }
    if (directory->child)
    {
        printf("Directory not empty\n");
        return;
    }
    removeChild(currentDirectory, directory);
    free(directory);
    printf("Directory removed successfully.\n");
}

void listFiles()
{
    if (currentDirectory->child == NULL)
    {
        printf("(empty)\n");
        return;
    }
    FileNode *entry = currentDirectory->child;
    do
    {
        printf("%s%s\n", entry->name, entry->isDirectory ? "/" : "");
        entry = entry->next;
    } while (entry != currentDirectory->child);
}

void showCurrentPath()
{
    char pathBuffer[512];
    int pathLength = 0;
    FileNode *node = currentDirectory;
    while (node && node->parent)
    {
        int nameLength = getLength(node->name);
        for (int i = pathLength - 1; i >= 0; i--)
            pathBuffer[i + nameLength + 1] = pathBuffer[i];
        pathBuffer[0] = '/';
        for (int i = 0; i < nameLength; i++)
            pathBuffer[i + 1] = node->name[i];
        pathLength += nameLength + 1;
        node = node->parent;
    }
    if (pathLength == 0)
        printf("/\n");
    else
    {
        pathBuffer[pathLength] = '\0';
        printf("%s\n", pathBuffer);
    }
}

void changeDirectory(char *name)
{
    if (!name)
        return;

    if (compareString(name, "/") == 0)
    {
        currentDirectory = rootDirectory;
        printf("Moved to /\n");
        return;
    }

    if (compareString(name, "..") == 0)
    {
        if (currentDirectory->parent)
            currentDirectory = currentDirectory->parent;
        printf("Moved to ");
        showCurrentPath();
        return;
    }

    FileNode *targetDirectory = findChild(currentDirectory, name);
    if (!targetDirectory)
    {
        printf("Directory not found\n");
        return;
    }
    if (!targetDirectory->isDirectory)
    {
        printf("Not a directory\n");
        return;
    }

    currentDirectory = targetDirectory;

    printf("Moved to ");
    showCurrentPath();
}

void showDiskUsage()
{
    int freeBlockCount = countFreeBlocks();
    int usedBlocks = totalBlockCount - freeBlockCount;
    double usedPercentage = (totalBlockCount == 0) ? 0 : (100.0 * usedBlocks) / totalBlockCount;
    printf("Total Blocks: %d\n  Used Blocks: %d\n  Free Blocks: %d\n Disk Usage: %.2f%%\n", totalBlockCount, usedBlocks, freeBlockCount, usedPercentage);
}

int main()
{
    initializeDisk(1024);
    printf("$ ./vfs\nCompact VFS - ready. Type 'exit' to quit. .\n");

    char command[100], argument1[100], argument2[512];

    while (1)
    {
        if (currentDirectory == rootDirectory)
            printf("/ > ");
        else
            printf("%s > ", currentDirectory->name);

        if (scanf("%s", command) != 1)
            break;

        if (compareString(command, "mkdir") == 0)
            scanf("%s", argument1), createDirectory(argument1);
        else if (compareString(command, "create") == 0)
            scanf("%s", argument1), createFile(argument1);
        else if (compareString(command, "write") == 0)
        {
            scanf("%s", argument1);
            getchar();
            fgets(argument2, 512, stdin);
            writeFile(argument1, argument2);
        }
        else if (compareString(command, "read") == 0)
            scanf("%s", argument1), readFile(argument1);
        else if (compareString(command, "delete") == 0)
            scanf("%s", argument1), deleteFile(argument1);
        else if (compareString(command, "rmdir") == 0)
            scanf("%s", argument1), removeDirectory(argument1);
        else if (compareString(command, "ls") == 0)
            listFiles();
        else if (compareString(command, "cd") == 0)
            scanf("%s", argument1), changeDirectory(argument1);
        else if (compareString(command, "pwd") == 0)
            showCurrentPath();
        else if (compareString(command, "df") == 0)
            showDiskUsage();
        else if (compareString(command, "exit") == 0)
        {
            printf("Memory released. Exiting program... ");
            cleanupFileSystem();
            return 0;
        }
        else
            printf("Unknown command\n");
    }
    return 0;
}