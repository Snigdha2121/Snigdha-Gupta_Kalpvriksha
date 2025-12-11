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
    int pendingIOStart;
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

int compareString(const char *string1, const char *string2)
{
    while (*string1 && *string2)
    {
        if (tolower((unsigned char)*string1) != tolower((unsigned char)*string2))
            return (tolower((unsigned char)*string1) - tolower((unsigned char)*string2));
        string1++;
        string2++;
    }
    return tolower((unsigned char)*string1) - tolower((unsigned char)*string2);
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
        if (!isspace((unsigned char)linePointer[index]))
            return 0;
    }
    return 1;
}

int isNumberString(const char *stringPointer)
{
    int index = 0;
    if (stringPointer[0] == '-' || stringPointer[0] == '+')
        index++;
    if (stringPointer[index] == '\0')
        return 0;
    while (stringPointer[index] != '\0')
    {
        if (!isdigit((unsigned char)stringPointer[index]))
            return 0;
        index++;
    }
    return 1;
}

int startsWithKillKeyword(char *linePointer)
{
    while (*linePointer == ' ' || *linePointer == '\t')
        linePointer++;
    if (tolower((unsigned char)linePointer[0]) != 'k')
        return 0;
    if (tolower((unsigned char)linePointer[1]) != 'i')
        return 0;
    if (tolower((unsigned char)linePointer[2]) != 'l')
        return 0;
    if (tolower((unsigned char)linePointer[3]) != 'l')
        return 0;
    if (!isspace((unsigned char)linePointer[4]))
        return 0;
    return 1;
}

void initializeQueue(ProcessQueue *queuePointer)
{
    queuePointer->frontNodePointer = NULL;
    queuePointer->rearNodePointer = NULL;
}

void enqueueProcess(ProcessQueue *queuePointer, ProcessControlBlock *processControlBlockPointer)
{
    QueueNode *newNodePointer = (QueueNode *)malloc(sizeof(QueueNode));
    if (!newNodePointer)
    {
        printf("Memory allocation failed for QueueNode");
        return;
    }
    newNodePointer->processControlBlockPointer = processControlBlockPointer;
    newNodePointer->nextNodePointer = NULL;
    if (!queuePointer->rearNodePointer)
    {
        queuePointer->frontNodePointer = queuePointer->rearNodePointer = newNodePointer;
    }
    else
    {
        queuePointer->rearNodePointer->nextNodePointer = newNodePointer;
        queuePointer->rearNodePointer = newNodePointer;
    }
}

ProcessControlBlock *dequeueProcess(ProcessQueue *queuePointer)
{
    if (!queuePointer->frontNodePointer)
        return NULL;
    QueueNode *temporaryNodePointer = queuePointer->frontNodePointer;
    ProcessControlBlock *processControlBlockPointer = temporaryNodePointer->processControlBlockPointer;
    queuePointer->frontNodePointer = temporaryNodePointer->nextNodePointer;
    if (!queuePointer->frontNodePointer)
        queuePointer->rearNodePointer = NULL;
    free(temporaryNodePointer);
    return processControlBlockPointer;
}

void removeProcessFromQueue(ProcessQueue *queuePointer, int pid)
{
    QueueNode *currentNodePointer = queuePointer->frontNodePointer;
    QueueNode *previousNodePointer = NULL;
    while (currentNodePointer)
    {
        if (currentNodePointer->processControlBlockPointer->pid == pid)
        {
            if (!previousNodePointer)
                queuePointer->frontNodePointer = currentNodePointer->nextNodePointer;
            else
                previousNodePointer->nextNodePointer = currentNodePointer->nextNodePointer;
            if (currentNodePointer == queuePointer->rearNodePointer)
                queuePointer->rearNodePointer = previousNodePointer;
            free(currentNodePointer);
            return;
        }
        previousNodePointer = currentNodePointer;
        currentNodePointer = currentNodePointer->nextNodePointer;
    }
}

int hashPid(int pid)
{
    if (pid < 0)
        pid = -pid;
    return pid % HASH_SIZE;
}

void initializeHashTable()
{
    for (int index = 0; index < HASH_SIZE; index++)
        hashTable[index].usedFlag = 0;
}

void insertIntoHashTable(int pid, ProcessControlBlock *processControlBlockPointer)
{
    int baseIndex = hashPid(pid);
    for (int offset = 0; offset < HASH_SIZE; offset++)
    {
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
    for (int offset = 0; offset < HASH_SIZE; offset++)
    {
        int position = (baseIndex + offset) % HASH_SIZE;
        if (!hashTable[position].usedFlag)
            return NULL;
        if (hashTable[position].pidKey == pid)
            return hashTable[position].processControlBlockPointer;
    }
    return NULL;
}

int extractProcessFields(const char *line, char *name, char *pid, char *cpu, char *ioStart, char *ioDur, int *fieldCount)
{
    *fieldCount = sscanf(line, "%31s %31s %31s %31s %31s", name, pid, cpu, ioStart, ioDur);
    return (*fieldCount >= 3);
}

int validateBasicProcessFields(const char *pidStr, const char *cpuStr, int *pidOut, int *cpuOut)
{
    if (!isNumberString(pidStr) || !isNumberString(cpuStr))
        return 0;

    int pid = atoi(pidStr);
    int cpu = atoi(cpuStr);

    if (pid <= 0 || cpu <= 0)
    {
        fprintf(stderr, "Error: PID and CPU Burst must be positive.\n");
        return 0;
    }
    if (findProcessByPid(pid) != NULL)
    {
        fprintf(stderr, "Error: Duplicate PID %d detected.\n", pid);
        return 0;
    }

    *pidOut = pid;
    *cpuOut = cpu;
    return 1;
}

int validateIoFieldsProcess(int fieldCount, const char *ioStartStr, const char *ioDurStr, int cpuBurst, int *ioStart, int *ioDur)
{
    *ioStart = -1;
    *ioDur = 0;

    if (fieldCount >= 4 && compareString(ioStartStr, "-") != 0 && isNumberString(ioStartStr))
    {
        *ioStart = atoi(ioStartStr);
        if (*ioStart < 0 || *ioStart >= cpuBurst)
        {
            fprintf(stderr, "Error: I/O Start (%d) must be >= 0 and < CPU Burst (%d).\n", *ioStart, cpuBurst);
            return 0;
        }
    }

    if (fieldCount >= 5 && compareString(ioDurStr, "-") != 0 && isNumberString(ioDurStr))
    {
        *ioDur = atoi(ioDurStr);
        if (*ioDur < 0)
        {
            fprintf(stderr, "Error: I/O Duration must be non-negative.\n");
            return 0;
        }
    }

    if (*ioStart >= 0 && *ioDur == 0)
        *ioStart = -1;

    return 1;
}

void initializePCBFields(ProcessControlBlock *pcb, const char *name, int pid, int cpuBurst, int ioStart, int ioDur)
{
    copyStringSafe(pcb->name, name, MAX_NAME_LENGTH);
    pcb->pid = pid;
    pcb->cpuBurst = cpuBurst;
    pcb->ioStart = ioStart;
    pcb->ioDuration = ioDur;
    pcb->ioRemaining = 0;
    pcb->executedCpu = 0;
    pcb->completionTime = -1;
    pcb->killedFlag = 0;
    pcb->state = StateReady;
    pcb->pendingIOStart = 0;

}

int parseProcessLine(const char *linePointer, ProcessControlBlock *processControlBlockPointer)
{
    char nameBuffer[32], pidBuffer[32], cpuBuffer[32];
    char ioStartBuffer[32], ioDurationBuffer[32];
    int fieldCount, pid, cpuBurst, ioStart, ioDur;

    if (!extractProcessFields(linePointer, nameBuffer, pidBuffer, cpuBuffer, ioStartBuffer, ioDurationBuffer, &fieldCount))
        return 0;

    if (!validateBasicProcessFields(pidBuffer, cpuBuffer, &pid, &cpuBurst))
        return 0;

    if (!validateIoFieldsProcess(fieldCount, ioStartBuffer, ioDurationBuffer, cpuBurst, &ioStart, &ioDur))
        return 0;

    initializePCBFields(processControlBlockPointer, nameBuffer, pid, cpuBurst, ioStart, ioDur);
    return 1;
}

int parseKillLine(const char *linePointer, KillEvent *killEventArray, int *killEventCountPointer)
{
    char keywordBuffer[10];
    char pidBuffer[32];
    char timeBuffer[32];

    int fieldCount = sscanf(linePointer, "%9s %31s %31s", keywordBuffer, pidBuffer, timeBuffer);
    if (fieldCount != 3)
    {
        fprintf(stderr, "Error: KILL event must specify PID and Time.\n");
        return 0;
    }
    if (!isNumberString(pidBuffer) || !isNumberString(timeBuffer))
    {
        fprintf(stderr, "Error: PID or Time in KILL event is not numeric.\n");
        return 0;
    }

    int pid = atoi(pidBuffer);
    int killTime = atoi(timeBuffer);

    if (pid <= 0 || killTime < 0)
    {
        fprintf(stderr, "Error: PID must be positive and Kill Time non-negative.\n");
        return 0;
    }
    if (*killEventCountPointer >= MAX_KILL_EVENTS)
    {
        fprintf(stderr, "Error: Maximum number of KILL events reached.\n");
        return 0;
    }

    killEventArray[*killEventCountPointer].pid = pid;
    killEventArray[*killEventCountPointer].killTime = killTime;
    (*killEventCountPointer)++;

    return 1;
}

void initializeInputCounters(int *processCountPointer, int *killEventCountPointer, int *tempProcessCount, int *tempKillCount, int *readSuccess)
{
    *processCountPointer = 0;
    *killEventCountPointer = 0;
    *tempProcessCount = 0;
    *tempKillCount = 0;
    *readSuccess = 0;
    initializeHashTable();
}

int readOneInputLine(char *buffer)
{
    if (fgets(buffer, MAX_LINE_LENGTH, stdin) == NULL)
        return 0;
    if (isBlankLine(buffer))
        return 0;
    return 1;
}

int processKillOrProcessLine(char *line, ProcessControlBlock *processArray, KillEvent *killEventArray, int *tempProcessCount, int *tempKillCount)
{
    if (startsWithKillKeyword(line))
    {
        if (!parseKillLine(line, killEventArray, tempKillCount))
            return 0;
    }
    else
    {
        if (*tempProcessCount >= MAX_PROCESSES)
        {
            fprintf(stderr, "Error: Max processes exceeded.\n");
            return 0;
        }
        if (!parseProcessLine(line, &processArray[*tempProcessCount]))
            return 0;

        insertIntoHashTable(processArray[*tempProcessCount].pid, &processArray[*tempProcessCount]);
        (*tempProcessCount)++;
    }
    return 1;
}

int validateFinalInput(int tempProcessCount, int tempKillCount, KillEvent *killEventArray)
{
    if (tempProcessCount == 0)
    {
        fprintf(stderr, "Error: No processes were entered.\n");
        return 0;
    }

    for (int i = 0; i < tempKillCount; i++)
    {
        if (findProcessByPid(killEventArray[i].pid) == NULL)
        {
            fprintf(stderr, "Error: KILL targets non-existent PID %d.\n", killEventArray[i].pid);
            return 0;
        }
    }
    return 1;
}

int readSimulationInput(ProcessControlBlock *processArray, int *processCountPointer, KillEvent *killEventArray, int *killEventCountPointer)
{
    char inputLineBuffer[MAX_LINE_LENGTH];
    int tempProcessCount, tempKillCount, readSuccess;

    initializeInputCounters(processCountPointer, killEventCountPointer, &tempProcessCount, &tempKillCount, &readSuccess);

    while (1)
    {
        if (!readOneInputLine(inputLineBuffer))
        {
            readSuccess = 1;
            break;
        }
        if (!processKillOrProcessLine(inputLineBuffer, processArray, killEventArray, &tempProcessCount, &tempKillCount))
            return 0;
    }

    if (!readSuccess)
        return 0;

    if (!validateFinalInput(tempProcessCount, tempKillCount, killEventArray))
        return 0;

    *processCountPointer = tempProcessCount;
    *killEventCountPointer = tempKillCount;
    return 1;
}

void updateWaitingQueue(ProcessQueue *waitingQueuePointer, ProcessQueue *readyQueuePointer)
{
    QueueNode *currentNodePointer = waitingQueuePointer->frontNodePointer;
    QueueNode *previousNodePointer = NULL;

    while (currentNodePointer)
    {
        ProcessControlBlock *processControlBlockPointer = currentNodePointer->processControlBlockPointer;
        if (processControlBlockPointer->ioRemaining > 0)
            processControlBlockPointer->ioRemaining--;
        QueueNode *nextNodePointer = currentNodePointer->nextNodePointer;

        if (processControlBlockPointer->ioRemaining == 0)
        {
            processControlBlockPointer->state = StateReady;
            if (!previousNodePointer)
                waitingQueuePointer->frontNodePointer = nextNodePointer;
            else
                previousNodePointer->nextNodePointer = nextNodePointer;
            if (currentNodePointer == waitingQueuePointer->rearNodePointer)
                waitingQueuePointer->rearNodePointer = previousNodePointer;
            enqueueProcess(readyQueuePointer, processControlBlockPointer);
            free(currentNodePointer);
        }
        else
        {
            previousNodePointer = currentNodePointer;
        }
        currentNodePointer = nextNodePointer;
    }
}

void applyKillEvent(ProcessQueue *readyQueuePointer, ProcessQueue *waitingQueuePointer, ProcessQueue *terminatedQueuePointer, ProcessControlBlock **runningProcessPointer, int pid, int currentTime, int *completedProcessCountPointer)
{
    ProcessControlBlock *targetProcessPointer = findProcessByPid(pid);

    if (!targetProcessPointer || targetProcessPointer->killedFlag || targetProcessPointer->state == StateTerminated)
        return;

    if (*runningProcessPointer && (*runningProcessPointer)->pid == pid)
        *runningProcessPointer = NULL;
    else
    {
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
    if (!*runningProcessPointer)
        return;

    ProcessControlBlock *processControlBlockPointer = *runningProcessPointer;
    processControlBlockPointer->executedCpu++;

    if (processControlBlockPointer->executedCpu == processControlBlockPointer->cpuBurst)
    {
        processControlBlockPointer->state = StateTerminated;
        processControlBlockPointer->completionTime = currentTime + 1;
        enqueueProcess(terminatedQueuePointer, processControlBlockPointer);
        (*completedProcessCountPointer)++;
        *runningProcessPointer = NULL;
    }
    else if (processControlBlockPointer->ioDuration > 0 && processControlBlockPointer->ioStart >= 0 && processControlBlockPointer->executedCpu == processControlBlockPointer->ioStart)
        {
            processControlBlockPointer->pendingIOStart = 1;
        }
}

void sortKillEvents(KillEvent *killEventArray, int count)
{
    for (int i = 1; i < count; i++)
    {
        KillEvent key = killEventArray[i];
        int j = i - 1;

        while (j >= 0 && (killEventArray[j].killTime > key.killTime || (killEventArray[j].killTime == key.killTime && killEventArray[j].pid > key.pid)))
        {
            killEventArray[j + 1] = killEventArray[j];
            j = j - 1;
        }
        killEventArray[j + 1] = key;
    }
}

void sortResultsByPid(int *indexArray, int count, ProcessControlBlock *processArray)
{
    for (int i = 0; i < count - 1; i++)
    {
        int min_idx = i;
        for (int j = i + 1; j < count; j++)
        {
            if (processArray[indexArray[j]].pid < processArray[indexArray[min_idx]].pid)
            {
                min_idx = j;
            }
        }
        if (min_idx != i)
        {
            int temp = indexArray[i];
            indexArray[i] = indexArray[min_idx];
            indexArray[min_idx] = temp;
        }
    }
}

void handleKills(int currentTime, KillEvent *killEvents, int killCount, int *killIndex, ProcessQueue *ready, ProcessQueue *waiting, ProcessQueue *terminated,ProcessControlBlock **running, int *completed)
{
    while (*killIndex < killCount &&
           killEvents[*killIndex].killTime == currentTime)
    {
        applyKillEvent(ready, waiting, terminated, running,
                       killEvents[*killIndex].pid, currentTime, completed);
        (*killIndex)++;
    }
}

void scheduleNextProcess(ProcessQueue *ready, ProcessControlBlock **running)
{
    if (!*running)
    {
        *running = dequeueProcess(ready);
        if (*running)
            (*running)->state = StateRunning;
    }
}

void processCpuWork(ProcessControlBlock **running, ProcessQueue *terminated, int *completed, int currentTime)
{
    if (!*running)
        return;

    ProcessControlBlock *pcb = *running;
    pcb->executedCpu++;

    if (pcb->executedCpu == pcb->cpuBurst)
    {
        pcb->state = StateTerminated;
        pcb->completionTime = currentTime + 1;
        enqueueProcess(terminated, pcb);
        (*completed)++;
        *running = NULL;
        return;
    }

    if (pcb->ioDuration > 0 && pcb->ioStart >= 0 &&
        pcb->executedCpu == pcb->ioStart)
    {
        pcb->pendingIOStart = 1;
    }
}

void startPendingIO(ProcessControlBlock **running, ProcessQueue *waiting)
{
    if (*running == NULL)
        return;

    ProcessControlBlock *pcb = *running;

    if (pcb->pendingIOStart)
    {
        pcb->pendingIOStart = 0;
        pcb->state = StateWaiting;
        pcb->ioRemaining = pcb->ioDuration +1;

        enqueueProcess(waiting, pcb);
        *running = NULL;
    }
}

void processWaitingIO(ProcessQueue *waiting, ProcessQueue *ready)
{
    updateWaitingQueue(waiting, ready);
}

void printQueue(const char *label, ProcessQueue *q)
{
    printf("%s: [", label);
    QueueNode *node = q->frontNodePointer;
    while (node)
    {
        printf("%d", node->processControlBlockPointer->pid);
        node = node->nextNodePointer;
        if (node) printf(", ");
    }
    printf("]\n");
}

void printTickLog(int tick,
                  ProcessControlBlock *running,
                  ProcessQueue *ready,
                  ProcessQueue *waiting)
{
    printf("\n=== Tick %d ===\n", tick);

    if (running)
        printf("Running: %d (%s)\n", running->pid, running->name);
    else
        printf("Running: [idle]\n");

    printQueue("Ready", ready);
    printQueue("Waiting", waiting);
}

void simulateScheduler(ProcessControlBlock *processArray, int processCount,
                       KillEvent *killEvents, int killCount)
{
    ProcessQueue ready, waiting, terminated;
    initializeQueue(&ready);
    initializeQueue(&waiting);
    initializeQueue(&terminated);

    for (int i = 0; i < processCount; i++)
        enqueueProcess(&ready, &processArray[i]);

    if (killCount > 0)
        sortKillEvents(killEvents, killCount);

    int currentTime = 0;
    int completed = 0;
    int killIndex = 0;
    ProcessControlBlock *running = NULL;

    while (completed < processCount)
    {
        handleKills(currentTime, killEvents, killCount, &killIndex, &ready, &waiting, &terminated, &running, &completed);

        processWaitingIO(&waiting, &ready);

        scheduleNextProcess(&ready, &running);

        processCpuWork(&running, &terminated, &completed, currentTime);

        startPendingIO(&running, &waiting);

        printTickLog(currentTime, running, &ready, &waiting);

        currentTime++;
        sleep(1);
    }
}

void printResultsWithKill(ProcessControlBlock *processArray, int *indexArray, int processCount)
{
    printf("\n%-8s %-15s %-8s %-8s %-15s %-12s %-12s\n", "PID", "Name", "CPU", "IO", "Status", "Turnaround", "Waiting");

    for (int i = 0; i < processCount; i++)
    {
        ProcessControlBlock *pcb = &processArray[indexArray[i]];

        if (pcb->killedFlag)
        {
            printf("%-8d %-15s %-8d %-8d KILLED at %-6d %-12s %-12s\n", pcb->pid, pcb->name, pcb->cpuBurst, pcb->ioDuration, pcb->completionTime, "-", "-");
        }
        else
        {
            int turnaround = pcb->completionTime;
            int waiting = turnaround - pcb->cpuBurst;

            printf("%-8d %-15s %-8d %-8d %-15s %-12d %-12d\n", pcb->pid, pcb->name, pcb->cpuBurst, pcb->ioDuration, "OK", turnaround, waiting);
        }
    }

    printf("\n");
}

void printResultsNoKill(ProcessControlBlock *processArray, int *indexArray, int processCount)
{
    printf("\n%-8s %-15s %-8s %-8s %-12s %-12s\n", "PID", "Name", "CPU", "IO", "Turnaround", "Waiting");

    for (int i = 0; i < processCount; i++)
    {
        ProcessControlBlock *pcb = &processArray[indexArray[i]];

        int turnaround = pcb->completionTime;
        int waiting = turnaround - pcb->cpuBurst;

        printf("%-8d %-15s %-8d %-8d %-12d %-12d\n", pcb->pid, pcb->name, pcb->cpuBurst, pcb->ioDuration, turnaround, waiting);
    }

    printf("\n");
}

void printResults(ProcessControlBlock *processArray, int processCount)
{
    int indexArray[MAX_PROCESSES];
    int killedProcessExists = 0;

    for (int i = 0; i < processCount; i++)
    {
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

    if (scanf("%15s", responseBuffer) != 1)
        return 0;

    while ((flushCharacter = getchar()) != '\n' && flushCharacter != EOF)
        ;
    if (responseBuffer[0] == 'y' || responseBuffer[0] == 'Y')
        return 1;
    return 0;
}

int main()
{
    ProcessControlBlock processArray[MAX_PROCESSES];
    KillEvent killEventArray[MAX_KILL_EVENTS];

    do
    {
        int processCount = 0;
        int killEventCount = 0;

        if (!readSimulationInput(processArray, &processCount, killEventArray, &killEventCount))
            break;

        simulateScheduler(processArray, processCount, killEventArray, killEventCount);

        printResults(processArray, processCount);

    } while (askToContinue());

    return 0;
}