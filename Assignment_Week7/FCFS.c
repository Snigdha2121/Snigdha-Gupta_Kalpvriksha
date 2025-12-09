#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_PROCESSES 128
#define MAX_KILL_EVENTS 128
#define HASH_SIZE 257
#define MAX_NAME_LENGTH 32
#define MAX_LINE_LENGTH 256

typedef enum
{
    StateReady = 0,
    StateRunning,
    StateWaiting,
    StateTerminated
} ProcessState;

typedef struct
{
    int pid;
    char name[MAX_NAME_LENGTH];
    int cpuBurst;
    int ioStart;
    int ioDuration;
    int ioRemaining;
    int executedCpu;
    int completionTime;
    int killedFlag;
    ProcessState state;
} ProcessControlBlock;

typedef struct
{
    int pid;
    int killTime;
} KillEvent;

typedef struct
{
    int usedFlag;
    int pidKey;
    ProcessControlBlock *processControlBlockPointer;
} HashEntry;

typedef struct QueueNode
{
    ProcessControlBlock *processControlBlockPointer;
    struct QueueNode *nextNodePointer;
} QueueNode;

typedef struct
{
    QueueNode *frontNodePointer;
    QueueNode *rearNodePointer;
} ProcessQueue;

HashEntry hashTable[HASH_SIZE];
ProcessControlBlock *sortArrayPointer;

int compareString(const char *s1, const char *s2)
{
    while (*s1 && *s2) 
    {
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) return (tolower((unsigned char)*s1) - tolower((unsigned char)*s2));
        s1++; s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

void copyStringSafe(char *destination, const char *source, size_t maxLen)
{
    size_t i = 0;
    while (i < maxLen - 1 && source[i] != '\0') 
    {
        destination[i] = source[i];
        i++;
    }
    destination[i] = '\0';
}

int isBlankLine(const char *linePointer)
{
    for (int index = 0; linePointer[index] != '\0'; index++) 
    {
        if (!isspace((unsigned char)linePointer[index])) return 0;
    }
    return 1;
}

int isNumberString(const char *stringPointer)
{
    int index = 0;
    if (stringPointer[0] == '-' || stringPointer[0] == '+') index++;
    if (stringPointer[index] == '\0') return 0;
    while (stringPointer[index] != '\0') 
    {
        if (!isdigit((unsigned char)stringPointer[index])) return 0;
        index++;
    }
    return 1;
}

int startsWithKillKeyword(char *linePointer)
{
    while (*linePointer == ' ' || *linePointer == '\t') linePointer++;
    if (tolower((unsigned char)linePointer[0]) != 'k') return 0;
    if (tolower((unsigned char)linePointer[1]) != 'i') return 0;
    if (tolower((unsigned char)linePointer[2]) != 'l') return 0;
    if (tolower((unsigned char)linePointer[3]) != 'l') return 0;
    if (!isspace((unsigned char)linePointer[4])) return 0;
    return 1;
}

void initializeQueue(ProcessQueue *queuePointer)
{
    queuePointer->frontNodePointer = NULL; queuePointer->rearNodePointer = NULL;
}

void enqueueProcess(ProcessQueue *queuePointer, ProcessControlBlock *processControlBlockPointer)
{
    QueueNode *newNodePointer = (QueueNode *)malloc(sizeof(QueueNode));
    if (!newNodePointer) 
    { 
        printf("Memory allocation failed for QueueNode"); 
        exit(EXIT_FAILURE); 
    }
    newNodePointer->processControlBlockPointer = processControlBlockPointer;
    newNodePointer->nextNodePointer = NULL;
    if (!queuePointer->rearNodePointer) {
        queuePointer->frontNodePointer = queuePointer->rearNodePointer = newNodePointer;
    } else {
        queuePointer->rearNodePointer->nextNodePointer = newNodePointer;
        queuePointer->rearNodePointer = newNodePointer;
    }
}

ProcessControlBlock *dequeueProcess(ProcessQueue *queuePointer)
{
    if (!queuePointer->frontNodePointer) return NULL;
    QueueNode *temporaryNodePointer = queuePointer->frontNodePointer;
    ProcessControlBlock *processControlBlockPointer = temporaryNodePointer->processControlBlockPointer;
    queuePointer->frontNodePointer = temporaryNodePointer->nextNodePointer;
    if (!queuePointer->frontNodePointer) queuePointer->rearNodePointer = NULL;
    free(temporaryNodePointer);
    return processControlBlockPointer;
}

void removeProcessFromQueue(ProcessQueue *queuePointer, int pid)
{
    QueueNode *currentNodePointer = queuePointer->frontNodePointer;
    QueueNode *previousNodePointer = NULL;
    while (currentNodePointer) {
        if (currentNodePointer->processControlBlockPointer->pid == pid) {
            if (!previousNodePointer) queuePointer->frontNodePointer = currentNodePointer->nextNodePointer;
            else previousNodePointer->nextNodePointer = currentNodePointer->nextNodePointer;
            if (currentNodePointer == queuePointer->rearNodePointer) queuePointer->rearNodePointer = previousNodePointer;
            free(currentNodePointer);
            return;
        }
        previousNodePointer = currentNodePointer;
        currentNodePointer = currentNodePointer->nextNodePointer;
    }
}

int hashPid(int pid)
{
    if (pid < 0) pid = -pid;
    return pid % HASH_SIZE;
}

void initializeHashTable()
{
    for (int index = 0; index < HASH_SIZE; index++) hashTable[index].usedFlag = 0;
}

void insertIntoHashTable(int pid, ProcessControlBlock *processControlBlockPointer)
{
    int baseIndex = hashPid(pid);
    for (int offset = 0; offset < HASH_SIZE; offset++) {
        int position = (baseIndex + offset) % HASH_SIZE;
        if (!hashTable[position].usedFlag) 
        {
            hashTable[position].usedFlag = 1;
            hashTable[position].pidKey = pid;
            hashTable[position].processControlBlockPointer = processControlBlockPointer;
            return;
        }
    }
    fprintf(stderr, "Error: Hash table capacity exceeded for PID %d.\n", pid);
}

ProcessControlBlock *findProcessByPid(int pid)
{
    int baseIndex = hashPid(pid);
    for (int offset = 0; offset < HASH_SIZE; offset++) {
        int position = (baseIndex + offset) % HASH_SIZE;
        if (!hashTable[position].usedFlag) return NULL;
        if (hashTable[position].pidKey == pid) return hashTable[position].processControlBlockPointer;
    }
    return NULL;
}

int parseProcessLine(const char *linePointer, ProcessControlBlock *processControlBlockPointer)
{
    char nameBuffer[32]; char pidBuffer[32]; char cpuBuffer[32]; char ioStartBuffer[32]; char ioDurationBuffer[32];

    int fieldCount = sscanf(linePointer, "%31s %31s %31s %31s %31s", nameBuffer, pidBuffer, cpuBuffer, ioStartBuffer, ioDurationBuffer);
    
    if (fieldCount < 3) return 0;
    if (!isNumberString(pidBuffer)) return 0;
    if (!isNumberString(cpuBuffer)) return 0;

    int pid = atoi(pidBuffer);
    int cpuBurst = atoi(cpuBuffer);

    if (pid <= 0 || cpuBurst <= 0) { fprintf(stderr, "Error: PID and CPU Burst must be positive.\n"); return 0; }
    if (findProcessByPid(pid) != NULL) { fprintf(stderr, "Error: Duplicate PID %d detected.\n", pid); return 0; }

    int ioStart = -1;
    int ioDuration = 0;

    if (fieldCount >= 4 && compareString(ioStartBuffer, "-") != 0 && isNumberString(ioStartBuffer)) {
        ioStart = atoi(ioStartBuffer);
        if (ioStart < 0 || ioStart > cpuBurst) { fprintf(stderr, "Error: I/O Start (%d) must be between 0 and CPU Burst (%d).\n", ioStart, cpuBurst); return 0; }
    }

    if (fieldCount >= 5 && compareString(ioDurationBuffer, "-") != 0 && isNumberString(ioDurationBuffer)) {
        ioDuration = atoi(ioDurationBuffer);
        if (ioDuration < 0) { fprintf(stderr, "Error: I/O Duration must be non-negative.\n"); return 0; }
    }
    
    if (ioStart >= 0 && ioDuration == 0) ioStart = -1;
    
    copyStringSafe(processControlBlockPointer->name, nameBuffer, MAX_NAME_LENGTH);
    
    processControlBlockPointer->pid = pid;
    processControlBlockPointer->cpuBurst = cpuBurst;
    processControlBlockPointer->ioStart = ioStart;
    processControlBlockPointer->ioDuration = ioDuration;
    processControlBlockPointer->ioRemaining = 0;
    processControlBlockPointer->executedCpu = 0;
    processControlBlockPointer->completionTime = -1;
    processControlBlockPointer->killedFlag = 0;
    processControlBlockPointer->state = StateReady;

    return 1;
}

int parseKillLine(const char *linePointer, KillEvent *killEventArray, int *killEventCountPointer)
{
    char keywordBuffer[10]; char pidBuffer[32]; char timeBuffer[32];

    int fieldCount = sscanf(linePointer, "%9s %31s %31s", keywordBuffer, pidBuffer, timeBuffer);
    if (fieldCount != 3) { fprintf(stderr, "Error: KILL event must specify PID and Time.\n"); return 0; }
    if (!isNumberString(pidBuffer) || !isNumberString(timeBuffer)) { fprintf(stderr, "Error: PID or Time in KILL event is not numeric.\n"); return 0; }

    int pid = atoi(pidBuffer);
    int killTime = atoi(timeBuffer);
    
    if (pid <= 0 || killTime < 0) { fprintf(stderr, "Error: PID must be positive and Kill Time non-negative.\n"); return 0; }
    if (*killEventCountPointer >= MAX_KILL_EVENTS) { fprintf(stderr, "Error: Maximum number of KILL events reached.\n"); return 0; }

    killEventArray[*killEventCountPointer].pid = pid;
    killEventArray[*killEventCountPointer].killTime = killTime;
    (*killEventCountPointer)++;

    return 1;
}

int readSimulationInput(ProcessControlBlock *processArray, int *processCountPointer, KillEvent *killEventArray, int *killEventCountPointer)
{
    char inputLineBuffer[MAX_LINE_LENGTH];
    int readSuccess = 0;
    int tempProcessCount = 0;
    int tempKillCount = 0;
    
    *processCountPointer = 0; *killEventCountPointer = 0;
    initializeHashTable(); 

    while (1) {
        if (fgets(inputLineBuffer, sizeof(inputLineBuffer), stdin) == NULL) { readSuccess = 1; break; }
        if (isBlankLine(inputLineBuffer)) { readSuccess = 1; break; }

        if (startsWithKillKeyword(inputLineBuffer)) {
            if (!parseKillLine(inputLineBuffer, killEventArray, &tempKillCount)) return 0;
        } else {
            if (tempProcessCount >= MAX_PROCESSES) { fprintf(stderr, "Error: Max processes exceeded.\n"); return 0; }
            if (!parseProcessLine(inputLineBuffer, &processArray[tempProcessCount])) return 0;
            insertIntoHashTable(processArray[tempProcessCount].pid, &processArray[tempProcessCount]);
            tempProcessCount++;
        }
    }

    if (!readSuccess) return 0; 

    if (tempProcessCount == 0) { fprintf(stderr, "Error: No processes were entered.\n"); return 0; }

    for (int i = 0; i < tempKillCount; i++) {
        if (findProcessByPid(killEventArray[i].pid) == NULL) { fprintf(stderr, "Error: KILL targets non-existent PID %d.\n", killEventArray[i].pid); return 0; }
    }
    
    *processCountPointer = tempProcessCount; *killEventCountPointer = tempKillCount;

    return 1;
}

void updateWaitingQueue(ProcessQueue *waitingQueuePointer, ProcessQueue *readyQueuePointer)
{
    QueueNode *currentNodePointer = waitingQueuePointer->frontNodePointer;
    QueueNode *previousNodePointer = NULL;

    while (currentNodePointer) {
        ProcessControlBlock *processControlBlockPointer = currentNodePointer->processControlBlockPointer;
        if (processControlBlockPointer->ioRemaining > 0) processControlBlockPointer->ioRemaining--;
        QueueNode *nextNodePointer = currentNodePointer->nextNodePointer;

        if (processControlBlockPointer->ioRemaining == 0) {
            processControlBlockPointer->state = StateReady;
            if (!previousNodePointer) waitingQueuePointer->frontNodePointer = nextNodePointer;
            else previousNodePointer->nextNodePointer = nextNodePointer;
            if (currentNodePointer == waitingQueuePointer->rearNodePointer) waitingQueuePointer->rearNodePointer = previousNodePointer;
            enqueueProcess(readyQueuePointer, processControlBlockPointer);
            free(currentNodePointer);
        } else {
            previousNodePointer = currentNodePointer;
        }
        currentNodePointer = nextNodePointer;
    }
}

void applyKillEvent(ProcessQueue *readyQueuePointer, ProcessQueue *waitingQueuePointer, ProcessQueue *terminatedQueuePointer, ProcessControlBlock **runningProcessPointer, int pid, int currentTime, int *completedProcessCountPointer)
{
    ProcessControlBlock *targetProcessPointer = findProcessByPid(pid);

    if (!targetProcessPointer || targetProcessPointer->killedFlag || targetProcessPointer->state == StateTerminated) return;

    if (*runningProcessPointer && (*runningProcessPointer)->pid == pid) *runningProcessPointer = NULL;
    else {
        removeProcessFromQueue(readyQueuePointer, pid);
        removeProcessFromQueue(waitingQueuePointer, pid);
    }

    targetProcessPointer->killedFlag = 1;
    targetProcessPointer->state = StateTerminated;
    targetProcessPointer->completionTime = currentTime;

    enqueueProcess(terminatedQueuePointer, targetProcessPointer);
    (*completedProcessCountPointer)++;
}

void executeCpuTick(ProcessControlBlock **runningProcessPointer, ProcessQueue *waitingQueuePointer, ProcessQueue *terminatedQueuePointer, int *completedProcessCountPointer, int currentTime)
{
    if (!*runningProcessPointer) return;

    ProcessControlBlock *processControlBlockPointer = *runningProcessPointer;
    processControlBlockPointer->executedCpu++;

    if (processControlBlockPointer->executedCpu == processControlBlockPointer->cpuBurst) {
        processControlBlockPointer->state = StateTerminated;
        processControlBlockPointer->completionTime = currentTime + 1;
        enqueueProcess(terminatedQueuePointer, processControlBlockPointer);
        (*completedProcessCountPointer)++;
        *runningProcessPointer = NULL;
    } else if (processControlBlockPointer->ioDuration > 0 && processControlBlockPointer->ioStart >= 0 && processControlBlockPointer->executedCpu == processControlBlockPointer->ioStart) {
        processControlBlockPointer->state = StateWaiting;
        processControlBlockPointer->ioRemaining = processControlBlockPointer->ioDuration;
        enqueueProcess(waitingQueuePointer, processControlBlockPointer);
        *runningProcessPointer = NULL;
    }
}

void sortKillEvents(KillEvent *killEventArray, int count)
{
    for (int i = 1; i < count; i++) {
        KillEvent key = killEventArray[i];
        int j = i - 1;

        while (j >= 0 && 
               (killEventArray[j].killTime > key.killTime || 
               (killEventArray[j].killTime == key.killTime && killEventArray[j].pid > key.pid))) 
        {
            killEventArray[j + 1] = killEventArray[j];
            j = j - 1;
        }
        killEventArray[j + 1] = key;
    }
}

void sortResultsByPid(int *indexArray, int count, ProcessControlBlock *processArray)
{
    for (int i = 0; i < count - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < count; j++) {
            if (processArray[indexArray[j]].pid < processArray[indexArray[min_idx]].pid) {
                min_idx = j;
            }
        }
        if (min_idx != i) {
            int temp = indexArray[i];
            indexArray[i] = indexArray[min_idx];
            indexArray[min_idx] = temp;
        }
    }
}

void simulateScheduler(ProcessControlBlock *processArray, int processCount, KillEvent *killEventArray, int killEventCount)
{
    ProcessQueue readyQueue; ProcessQueue waitingQueue; ProcessQueue terminatedQueue;

    initializeQueue(&readyQueue); initializeQueue(&waitingQueue); initializeQueue(&terminatedQueue);

    for (int processIndex = 0; processIndex < processCount; processIndex++) enqueueProcess(&readyQueue, &processArray[processIndex]);
    
    if (killEventCount > 0) sortKillEvents(killEventArray, killEventCount);

    int currentTime = 0; int completedProcessCount = 0; int killEventIndex = 0;
    ProcessControlBlock *runningProcessPointer = NULL;

    while (completedProcessCount < processCount) {
        while (killEventIndex < killEventCount && killEventArray[killEventIndex].killTime == currentTime) {
            applyKillEvent(&readyQueue, &waitingQueue, &terminatedQueue, &runningProcessPointer, killEventArray[killEventIndex].pid, currentTime, &completedProcessCount);
            killEventIndex++;
        }

        if (!runningProcessPointer) {
            runningProcessPointer = dequeueProcess(&readyQueue);
            if (runningProcessPointer) runningProcessPointer->state = StateRunning;
        }

        updateWaitingQueue(&waitingQueue, &readyQueue);
        
        if (runningProcessPointer) {
            executeCpuTick(&runningProcessPointer, &waitingQueue, &terminatedQueue, &completedProcessCount, currentTime);
        }

        currentTime++;
        sleep(1);
    }
}

void printResultsWithKill(ProcessControlBlock *processArray, int *indexArray, int processCount)
{
    printf("\n%-8s %-15s %-8s %-8s %-15s %-12s %-12s\n",
           "PID", "Name", "CPU", "IO", "Status", "Turnaround", "Waiting");

    for (int i = 0; i < processCount; i++) {
        ProcessControlBlock *pcb = &processArray[indexArray[i]];

        if (pcb->killedFlag) {
            printf("%-8d %-15s %-8d %-8d KILLED at %-6d %-12s %-12s\n",
                   pcb->pid, pcb->name, pcb->cpuBurst, pcb->ioDuration,
                   pcb->completionTime, "-", "-");
        } else {
            int turnaround = pcb->completionTime;
            int waiting = turnaround - pcb->cpuBurst;

            printf("%-8d %-15s %-8d %-8d %-15s %-12d %-12d\n",
                   pcb->pid, pcb->name, pcb->cpuBurst, pcb->ioDuration,
                   "OK", turnaround, waiting);
        }
    }

    printf("\n");
}

void printResultsNoKill(ProcessControlBlock *processArray, int *indexArray, int processCount)
{
    printf("\n%-8s %-15s %-8s %-8s %-12s %-12s\n",
           "PID", "Name", "CPU", "IO", "Turnaround", "Waiting");

    for (int i = 0; i < processCount; i++) {
        ProcessControlBlock *pcb = &processArray[indexArray[i]];

        int turnaround = pcb->completionTime;
        int waiting = turnaround - pcb->cpuBurst;

        printf("%-8d %-15s %-8d %-8d %-12d %-12d\n",
               pcb->pid, pcb->name, pcb->cpuBurst, pcb->ioDuration,
               turnaround, waiting);
    }

    printf("\n");
}

void printResults(ProcessControlBlock *processArray, int processCount)
{
    int indexArray[MAX_PROCESSES];
    int killedProcessExists = 0;

    for (int i = 0; i < processCount; i++) {
        indexArray[i] = i;
        if (processArray[i].killedFlag)
            killedProcessExists = 1;
    }

    sortResultsByPid(indexArray, processCount, processArray);

    if (killedProcessExists)
        printResultsWithKill(processArray, indexArray, processCount);
    else
        printResultsNoKill(processArray, indexArray, processCount);
}

int askToContinue()
{
    char responseBuffer[16];
    int flushCharacter;

    printf("Run another simulation? (yes/no): ");

    if (scanf("%15s", responseBuffer) != 1) return 0;

    while ((flushCharacter = getchar()) != '\n' && flushCharacter != EOF) {}

    if (responseBuffer[0] == 'y' || responseBuffer[0] == 'Y') return 1;
    return 0;
}

int main()
{
    ProcessControlBlock processArray[MAX_PROCESSES];
    KillEvent killEventArray[MAX_KILL_EVENTS];

    do {
        int processCount = 0;
        int killEventCount = 0;

        if (!readSimulationInput(processArray, &processCount, killEventArray, &killEventCount)) break;

        simulateScheduler(processArray, processCount, killEventArray, killEventCount);

        printResults(processArray, processCount);

    } while (askToContinue());

    return 0;
}