#include <stdio.h>
#include <stdlib.h>
#include "Players_data.h"

typedef struct
{
    int PlayerId;
    char Name[51];
    char TeamName[51];
    char Role[20];
    int TotalRuns;
    float BattingAverage;
    float StrikeRate;
    int Wickets;
    float EconomyRate;
    float PerformanceIndex;
} PlayerInfo;

typedef struct PlayerNode
{
    PlayerInfo Data;
    struct PlayerNode *Next;
} PlayerNode;

typedef struct
{
    int TeamId;
    char Name[51];
    int TotalPlayers;
    float AverageBattingStrikeRate;
    PlayerNode *BatsmanHead;
    PlayerNode *BowlerHead;
    PlayerNode *AllRounderHead;
} TeamInfo;

static TeamInfo teamList[10];
static int totalTeams = 10;
static PlayerNode *allBatsmenHead = NULL;
static PlayerNode *allBowlersHead = NULL;
static PlayerNode *allAllRoundersHead = NULL;

void copyString(char *destination, char *source)
{
    int indexValue = 0;
    while (source && source[indexValue])
    {
        destination[indexValue] = source[indexValue];
        indexValue++;
    }
    destination[indexValue] = '\0';
}

int compareString(char *first, char *second)
{
    int indexValue = 0;
    while (first[indexValue] && second[indexValue])
    {
        if (first[indexValue] != second[indexValue])
            return first[indexValue] - second[indexValue];
        indexValue++;
    }
    return first[indexValue] - second[indexValue];
}

float calculatePerformanceIndex(PlayerInfo *playerInfo)
{
    if (compareString(playerInfo->Role, "Batsman") == 0)
        return (playerInfo->BattingAverage * playerInfo->StrikeRate) / 100.0f;

    if (compareString(playerInfo->Role, "Bowler") == 0)
        return (playerInfo->Wickets * 2.0f) + (100.0f - playerInfo->EconomyRate);

    if (compareString(playerInfo->Role, "All-rounder") == 0)
    {
        float battingScore = (playerInfo->BattingAverage * playerInfo->StrikeRate) / 100.0f;
        float bowlingScore = playerInfo->Wickets * 2.0f;
        return battingScore + bowlingScore;
    }
    return 0.0f;
}

PlayerNode *createPlayerNode(PlayerInfo *playerInfo)
{
    PlayerNode *newNode = (PlayerNode *)malloc(sizeof(PlayerNode));
    newNode->Data = *playerInfo;
    newNode->Next = NULL;
    return newNode;
}

PlayerNode *insertSorted(PlayerNode *headNode, PlayerNode *newNode)
{
    if (headNode == NULL || newNode->Data.PerformanceIndex > headNode->Data.PerformanceIndex)
    {
        newNode->Next = headNode;
        return newNode;
    }
    PlayerNode *currentNode = headNode;
    while (currentNode->Next && currentNode->Next->Data.PerformanceIndex >= newNode->Data.PerformanceIndex)
        currentNode = currentNode->Next;

    newNode->Next = currentNode->Next;
    currentNode->Next = newNode;
    return headNode;
}

void addPlayerToTeamList(TeamInfo *teamInfo, PlayerNode *newNode)
{
    if (compareString(newNode->Data.Role, "Batsman") == 0)
        teamInfo->BatsmanHead = insertSorted(teamInfo->BatsmanHead, newNode);
    else if (compareString(newNode->Data.Role, "Bowler") == 0)
        teamInfo->BowlerHead = insertSorted(teamInfo->BowlerHead, newNode);
    else if (compareString(newNode->Data.Role, "All-rounder") == 0)
        teamInfo->AllRounderHead = insertSorted(teamInfo->AllRounderHead, newNode);
}

void addPlayerToGlobalList(PlayerInfo *playerInfo)
{
    PlayerNode *newNode = createPlayerNode(playerInfo);

    if (compareString(playerInfo->Role, "Batsman") == 0)
    {
        newNode->Next = allBatsmenHead;
        allBatsmenHead = newNode;
    }
    else if (compareString(playerInfo->Role, "Bowler") == 0)
    {
        newNode->Next = allBowlersHead;
        allBowlersHead = newNode;
    }
    else if (compareString(playerInfo->Role, "All-rounder") == 0)
    {
        newNode->Next = allAllRoundersHead;
        allAllRoundersHead = newNode;
    }
}

void computeBattingStats(PlayerNode *headNode, float *totalStrikeRate, int *totalCount)
{
    PlayerNode *currentNode = headNode;
    while (currentNode)
    {
        *totalStrikeRate += currentNode->Data.StrikeRate;
        *totalCount = *totalCount + 1;
        currentNode = currentNode->Next;
    }
}

void computeTeamStats(TeamInfo *teamInfo)
{
    float strikeRateSum = 0.0f;
    int battingCount = 0;
    int bowlingCount = 0;

    computeBattingStats(teamInfo->BatsmanHead, &strikeRateSum, &battingCount);
    computeBattingStats(teamInfo->AllRounderHead, &strikeRateSum, &battingCount);

    PlayerNode *currentNode = teamInfo->BowlerHead;
    while (currentNode)
    {
        bowlingCount++;
        currentNode = currentNode->Next;
    }

    teamInfo->TotalPlayers = battingCount + bowlingCount;

    if (battingCount > 0)
        teamInfo->AverageBattingStrikeRate = strikeRateSum / battingCount;
    else
        teamInfo->AverageBattingStrikeRate = 0.0f;
}

int findTeamIndexById(int teamId)
{
    int lowIndex = 0;
    int highIndex = totalTeams - 1;

    while (lowIndex <= highIndex)
    {
        int midIndex = lowIndex + (highIndex - lowIndex) / 2;

        if (teamList[midIndex].TeamId == teamId)
            return midIndex;

        if (teamList[midIndex].TeamId < teamId)
            lowIndex = midIndex + 1;
        else
            highIndex = midIndex - 1;
    }
    return -1;
}

int findTeamIndexByName(char *teamName)
{
    for (int indexValue = 0; indexValue < totalTeams; indexValue++)
        if (compareString(teamList[indexValue].Name, teamName) == 0)
            return indexValue;
    return -1;
}

PlayerNode *getMiddleNode(PlayerNode *headNode)
{
    if (!headNode)
        return headNode;

    PlayerNode *slowNode = headNode;
    PlayerNode *fastNode = headNode->Next;

    while (fastNode)
    {
        fastNode = fastNode->Next;
        if (fastNode)
        {
            slowNode = slowNode->Next;
            fastNode = fastNode->Next;
        }
    }
    return slowNode;
}

PlayerNode *mergeSortedLists(PlayerNode *firstList, PlayerNode *secondList)
{
    if (!firstList)
        return secondList;
    if (!secondList)
        return firstList;

    PlayerNode *resultNode;

    if (firstList->Data.PerformanceIndex >= secondList->Data.PerformanceIndex)
    {
        resultNode = firstList;
        resultNode->Next = mergeSortedLists(firstList->Next, secondList);
    }
    else
    {
        resultNode = secondList;
        resultNode->Next = mergeSortedLists(firstList, secondList->Next);
    }

    return resultNode;
}

PlayerNode *mergeSort(PlayerNode *headNode)
{
    if (!headNode || !headNode->Next)
        return headNode;

    PlayerNode *middleNode = getMiddleNode(headNode);
    PlayerNode *rightSide = middleNode->Next;
    middleNode->Next = NULL;

    PlayerNode *leftSorted = mergeSort(headNode);
    PlayerNode *rightSorted = mergeSort(rightSide);

    return mergeSortedLists(leftSorted, rightSorted);
}

void sortTeamsByStrikeRate(TeamInfo *arrayList, int count)
{
    for (int indexValue = 1; indexValue < count; indexValue++)
    {
        TeamInfo keyTeam = arrayList[indexValue];
        int position = indexValue - 1;

        while (position >= 0 &&
               arrayList[position].AverageBattingStrikeRate < keyTeam.AverageBattingStrikeRate)
        {
            arrayList[position + 1] = arrayList[position];
            position--;
        }
        arrayList[position + 1] = keyTeam;
    }
}

void initializeTeams()
{
    for (int indexValue = 0; indexValue < totalTeams; indexValue++)
    {
        teamList[indexValue].TeamId = indexValue + 1;
        copyString(teamList[indexValue].Name, (char *)teams[indexValue]);
        teamList[indexValue].TotalPlayers = 0;
        teamList[indexValue].AverageBattingStrikeRate = 0.0f;
        teamList[indexValue].BatsmanHead = NULL;
        teamList[indexValue].BowlerHead = NULL;
        teamList[indexValue].AllRounderHead = NULL;
    }
}

PlayerInfo createPlayerInfoFromHeader(const Player *sourcePlayer)
{
    PlayerInfo playerInfo;

    playerInfo.PlayerId = sourcePlayer->id;
    copyString(playerInfo.Name, (char *)sourcePlayer->name);
    copyString(playerInfo.TeamName, (char *)sourcePlayer->team);
    copyString(playerInfo.Role, (char *)sourcePlayer->role);

    playerInfo.TotalRuns = sourcePlayer->totalRuns;
    playerInfo.BattingAverage = sourcePlayer->battingAverage;
    playerInfo.StrikeRate = sourcePlayer->strikeRate;
    playerInfo.Wickets = sourcePlayer->wickets;
    playerInfo.EconomyRate = sourcePlayer->economyRate;

    playerInfo.PerformanceIndex = calculatePerformanceIndex(&playerInfo);
    return playerInfo;
}

void populateTeams()
{
    for (int indexValue = 0; indexValue < playerCount; indexValue++)
    {
        PlayerInfo playerInfo = createPlayerInfoFromHeader(&players[indexValue]);
        int teamIndex = findTeamIndexByName(playerInfo.TeamName);
        if (teamIndex < 0)
            continue;

        PlayerNode *node = createPlayerNode(&playerInfo);
        addPlayerToTeamList(&teamList[teamIndex], node);
        addPlayerToGlobalList(&playerInfo);
    }
}

void calculateAllTeamStats()
{
    for (int indexValue = 0; indexValue < totalTeams; indexValue++)
        computeTeamStats(&teamList[indexValue]);
}

void initializeSystem()
{
    initializeTeams();
    populateTeams();
    calculateAllTeamStats();
}

void freePlayerNodes(PlayerNode *headNode)
{
    PlayerNode *currentNode = headNode;
    while (currentNode)
    {
        PlayerNode *nextNode = currentNode->Next;
        free(currentNode);
        currentNode = nextNode;
    }
}

void cleanup()
{
    for (int indexValue = 0; indexValue < totalTeams; indexValue++)
    {
        freePlayerNodes(teamList[indexValue].BatsmanHead);
        freePlayerNodes(teamList[indexValue].BowlerHead);
        freePlayerNodes(teamList[indexValue].AllRounderHead);
    }
    freePlayerNodes(allBatsmenHead);
    freePlayerNodes(allBowlersHead);
    freePlayerNodes(allAllRoundersHead);
}

void printLine()
{
    printf("==========================================================================================\n");
}

void printPlayerHeader(int includeTeam)
{
    if (includeTeam)
    {
        printLine();
        printf("%-6s %-20s %-20s %-14s %-8s %-8s %-8s %-8s %-8s %-12s\n", "ID", "Name", "Team", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf. Index");
        printLine();
    }
    else
    {
        printLine();
        printf("%-6s %-20s %-14s %-8s %-8s %-8s %-8s %-8s %-12s\n",
               "ID", "Name", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf. Index");
        printLine();
    }
}

void printPlayer(PlayerInfo *playerInfo, int showTeam)
{
    if (showTeam)
    {
        printf("%-6d %-20s %-20s %-14s %-8d %-8.1f %-8.1f %-8d %-8.1f %-12.2f\n",
               playerInfo->PlayerId, playerInfo->Name, playerInfo->TeamName, playerInfo->Role,
               playerInfo->TotalRuns, playerInfo->BattingAverage,
               playerInfo->StrikeRate, playerInfo->Wickets,
               playerInfo->EconomyRate, playerInfo->PerformanceIndex);
    }
    else
    {
        printf("%-6d %-20s %-14s %-8d %-8.1f %-8.1f %-8d %-8.1f %-12.2f\n",
               playerInfo->PlayerId, playerInfo->Name, playerInfo->Role,
               playerInfo->TotalRuns, playerInfo->BattingAverage,
               playerInfo->StrikeRate, playerInfo->Wickets,
               playerInfo->EconomyRate, playerInfo->PerformanceIndex);
    }
}

void displayPlayerList(PlayerNode *headNode, int showTeam)
{
    PlayerNode *currentNode = headNode;
    while (currentNode)
    {
        printPlayer(&currentNode->Data, showTeam);
        currentNode = currentNode->Next;
    }
}

void getRoleName(int roleChoice, char *roleText)
{
    if (roleChoice == 1)
        copyString(roleText, "Batsman");
    else if (roleChoice == 2)
        copyString(roleText, "Bowler");
    else if (roleChoice == 3)
        copyString(roleText, "All-rounder");
    else
        copyString(roleText, "Unknown");
}

PlayerInfo readPlayerFromUser(char *teamName)
{
    printf("Enter Player Details:\n");
    PlayerInfo playerInfo;
    int roleChoice;

    copyString(playerInfo.TeamName, teamName);

    printf("Player ID: ");
    scanf("%d", &playerInfo.PlayerId);

    printf("Player Name: ");
    scanf(" %[^\n]s", playerInfo.Name);

    printf("Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    scanf("%d", &roleChoice);

    getRoleName(roleChoice, playerInfo.Role);

    printf("Total Runs: ");
    scanf("%d", &playerInfo.TotalRuns);

    printf("Batting Average: ");
    scanf("%f", &playerInfo.BattingAverage);

    printf("Strike Rate: ");
    scanf("%f", &playerInfo.StrikeRate);

    printf("Wickets: ");
    scanf("%d", &playerInfo.Wickets);

    printf("Economy Rate: ");
    scanf("%f", &playerInfo.EconomyRate);

    playerInfo.PerformanceIndex = calculatePerformanceIndex(&playerInfo);

    return playerInfo;
}

void addPlayer()
{
    int teamId;
    printf("Enter Team ID to add player: ");
    scanf("%d", &teamId);

    int teamIndex = findTeamIndexById(teamId);
    if (teamIndex < 0)
    {
        printf("Team not found\n");
        return;
    }

    PlayerInfo playerInfo = readPlayerFromUser(teamList[teamIndex].Name);
    PlayerNode *newNode = createPlayerNode(&playerInfo);

    addPlayerToTeamList(&teamList[teamIndex], newNode);
    addPlayerToGlobalList(&playerInfo);

    computeTeamStats(&teamList[teamIndex]);

    printf("Player added successfully to Team %s\n", teamList[teamIndex].Name);
}

void DisplayTeam()
{
    int teamId;
    printf("Enter Team ID: ");
    scanf("%d", &teamId);

    int teamIndex = findTeamIndexById(teamId);
    if (teamIndex < 0)
    {
        printf("Team not found\n");
        return;
    }

    TeamInfo *t = &teamList[teamIndex];

    printf("Players of %s:\n", t->Name);
    printPlayerHeader(0);

    displayPlayerList(t->BatsmanHead, 0);
    displayPlayerList(t->BowlerHead, 0);
    displayPlayerList(t->AllRounderHead, 0);

    printLine();

    printf("Total Players: %d\n", t->TotalPlayers);
    printf("Average Batting Strike Rate: %.2f\n", t->AverageBattingStrikeRate);
}

void DisplayTeamsByStrikeRate()
{
    TeamInfo sortedTeams[10];

    for (int indexValue = 0; indexValue < totalTeams; indexValue++)
        sortedTeams[indexValue] = teamList[indexValue];

    sortTeamsByStrikeRate(sortedTeams, totalTeams);

    printf("Teams sorted by Batting Strike Rate\n");
    printLine();
    printf("%-6s %-20s %-12s %-12s\n", "ID", "Name", "Avg SR", "Players");
    printLine();

    for (int indexValue = 0; indexValue < totalTeams; indexValue++)
    {
        printf("%-6d %-20s %-12.2f %-12d\n",
               sortedTeams[indexValue].TeamId,
               sortedTeams[indexValue].Name,
               sortedTeams[indexValue].AverageBattingStrikeRate,
               sortedTeams[indexValue].TotalPlayers);
    }
    printLine();
}

PlayerNode* getTopKSelection(int* teamIndexOut, int* countLimitOut)
{
    int teamId, roleChoice, countLimit;

    printf("Enter Team ID: ");
    scanf("%d", &teamId);

    int teamIndex = findTeamIndexById(teamId);
    if (teamIndex < 0)
    {
        printf("Team not found\n");
        return NULL;
    }

    printf("Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    scanf("%d", &roleChoice);

    printf("Enter count: ");
    scanf("%d", &countLimit);

    PlayerNode* list = NULL;

    if (roleChoice == 1)
        list = teamList[teamIndex].BatsmanHead;
    else if (roleChoice == 2)
        list = teamList[teamIndex].BowlerHead;
    else if (roleChoice == 3)
        list = teamList[teamIndex].AllRounderHead;
    else
    {
        printf("Invalid role\n");
        return NULL;
    }

    *teamIndexOut = teamIndex;
    *countLimitOut = countLimit;

    return list;
}

void showTopK(PlayerNode* list, int teamIndex, int countLimit)
{
    printf("Top %d players of team %s\n",
           countLimit, teamList[teamIndex].Name);

    printPlayerHeader(0);

    int printed = 0;
    PlayerNode* current = list;

    while (current && printed < countLimit)
    {
        printPlayer(&current->Data, 0);
        current = current->Next;
        printed++;
    }

    printLine();
}

void displayTopK()
{
    int teamIndex, countLimit;

    PlayerNode* list = getTopKSelection(&teamIndex, &countLimit);
    if (!list) return;

    showTopK(list, teamIndex, countLimit);
}

void handleDisplayAllByRole()
{
    int roleChoice;
    printf("Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    scanf("%d", &roleChoice);

    PlayerNode *listHead = NULL;

    if (roleChoice == 1)
        listHead = allBatsmenHead;
    else if (roleChoice == 2)
        listHead = allBowlersHead;
    else if (roleChoice == 3)
        listHead = allAllRoundersHead;
    else
    {
        printf("Invalid role\n");
        return;
    }

    PlayerNode *sortedList = mergeSort(listHead);

    if (roleChoice == 1)
        allBatsmenHead = sortedList;
    else if (roleChoice == 2)
        allBowlersHead = sortedList;
    else if (roleChoice == 3)
        allAllRoundersHead = sortedList;

    printPlayerHeader(1);

    PlayerNode *currentNode = sortedList;
    while (currentNode)
    {
        printPlayer(&currentNode->Data, 1);
        currentNode = currentNode->Next;
    }
}

void takeValidInput(int *userChoice)
{
    int choice;
    char bufferChar;

    while (1)
    {
        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid. Re-enter: ");
            while ((bufferChar = getchar()) != '\n')
                ;
            continue;
        }

        bufferChar = getchar();
        if (bufferChar != '\n')
        {
            printf("Kindly enter only numeric values (1-6). Re-enter: ");
            while ((bufferChar = getchar()) != '\n')
                ;
            continue;
        }

        *userChoice = choice;
        return;
    }
}

void showMenu(int *userChoice)
{
    printf("\n====================================================================================\n");
    printf("ICC ODI Player Performance Analyzer\n");
    printf("====================================================================================\n");
    printf("1. Add Player to Team\n");
    printf("2. Display Players of a Specific Team\n");
    printf("3. Display Teams by Average Batting Strike Rate\n");
    printf("4. Display Top K Players of a Specific Team by Role\n");
    printf("5. Display All Players of a Role Across All Teams by Performance Index\n");
    printf("6. Exit\n");
    printf("====================================================================================\n");
    printf("Enter your choice: ");
    takeValidInput(userChoice);
}

int main()
{
    initializeSystem();

    int choice = 0;

    do
    {
        showMenu(&choice);

        switch(choice)
        {
            case 1: addPlayer(); break;
            case 2: DisplayTeam(); break;
            case 3: DisplayTeamsByStrikeRate(); break;
            case 4: displayTopK(); break;
            case 5: handleDisplayAllByRole(); break;
            case 6: printf("Exiting\n"); break;
            default: printf("Invalid choice\n");
        }
    } while (choice != 6);

    cleanup();
    return 0;
}