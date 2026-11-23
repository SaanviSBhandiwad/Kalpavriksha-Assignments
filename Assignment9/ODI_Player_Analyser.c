#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Players_data.h"

#define MAX_TEAMS            10
#define MAX_PLAYERS_PER_TEAM 50
#define MAX_NAME_LENGTH      50
#define MIN_ID               1
#define MAX_ID               1000

#define ROLE_BATSMAN      1
#define ROLE_BOWLER       2
#define ROLE_ALL_ROUNDER  3

typedef struct PlayerRecord
{
    int   playerId;
    char  name[MAX_NAME_LENGTH + 1];
    char  teamName[MAX_NAME_LENGTH + 1];
    int   role;
    int   totalRuns;
    float battingAverage;
    float strikeRate;
    int   wickets;
    float economyRate;
    float performanceIndex;
    struct PlayerRecord *nextInTeam;
    struct PlayerRecord *nextInRole;
} PlayerRecord;

typedef struct
{
    int   teamId;
    char  name[MAX_NAME_LENGTH + 1];
    int   totalPlayers;
    float averageBattingStrikeRate;
    float sumBattingStrikeRate;
    int   battingPlayersCount;
    PlayerRecord *allPlayersHead;
    PlayerRecord *allPlayersTail;
    PlayerRecord *roleHead[4];
} Team;

Team teamList[MAX_TEAMS];
int  numberOfTeams = 0;

PlayerRecord *heap[MAX_TEAMS];
int heapSize = 0;

// Removes leftover characters of the current input line
void clearInputBuffer()
{
    int ch;

    while ((ch = getchar()) != '\n' && ch != EOF)
    {
    }
}

// Reads and validates a player name (1 to MAX_NAME_LENGTH characters).
// Handles blank lines and overlong input without leaving characters for the next prompt.
int readPlayerName(char name[], size_t size)
{
    size_t length;
    int ch;

    while (1)
    {
        if (fgets(name, (int)size, stdin) == NULL)
        {
            return 0;
        }

        length = strlen(name);

        // If the input line did not fit, discard the rest of that line.
        if (length > 0 && name[length - 1] != '\n' && !feof(stdin))
        {
            while ((ch = getchar()) != '\n' && ch != EOF)
            {
            }
            printf("Name must not exceed %d characters. Enter again: ", MAX_NAME_LENGTH);
            continue;
        }

        name[strcspn(name, "\r\n")] = '\0';
        length = strlen(name);

        if (length == 0 || strspn(name, " \t") == length)
        {
            printf("Name cannot be empty. Enter again: ");
            continue;
        }

        if (length > MAX_NAME_LENGTH)
        {
            printf("Name must not exceed %d characters. Enter again: ", MAX_NAME_LENGTH);
            continue;
        }

        return 1;
    }
}

// Converts a role number (1, 2, 3) to its name
const char *roleToString(int role)
{
    if (role == ROLE_BATSMAN)
    {
        return "Batsman";
    }
    if (role == ROLE_BOWLER)
    {
        return "Bowler";
    }
    return "All-rounder";
}

// Converts a role name from the header file to its number
int roleFromString(const char *text)
{
    if (strcmp(text, "Batsman") == 0)
    {
        return ROLE_BATSMAN;
    }
    if (strcmp(text, "Bowler") == 0)
    {
        return ROLE_BOWLER;
    }
    return ROLE_ALL_ROUNDER;
}

// Calculates the performance index using the formula of the player's role
float calculatePerformanceIndex(int role, float battingAverage, float strikeRate, int wickets, float economyRate)
{
    float battingPart = (battingAverage * strikeRate) / 100.0f;

    if (role == ROLE_BATSMAN)
    {
        return battingPart;
    }
    if (role == ROLE_BOWLER)
    {
        return (wickets * 2) + (100.0f - economyRate);
    }
    return battingPart + (wickets * 2);
}

// Binary search on teamList (sorted by teamId), returns index or -1
int findTeamIndexById(int teamId)
{
    int low = 0;
    int high = numberOfTeams - 1;
    int mid;

    while (low <= high)
    {
        mid = (low + high) / 2;
        if (teamList[mid].teamId == teamId)
        {
            return mid;
        }
        else if (teamList[mid].teamId < teamId)
        {
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }
    return -1;
}

// Finds a team index by its name (used only while loading header data)
int findTeamIndexByName(const char *teamName)
{
    int i;

    for (i = 0; i < numberOfTeams; i++)
    {
        if (strcmp(teamList[i].name, teamName) == 0)
        {
            return i;
        }
    }
    return -1;
}

// Checks whether a player id is already used by any team
int playerIdExists(int playerId)
{
    int i;
    PlayerRecord *current;

    for (i = 0; i < numberOfTeams; i++)
    {
        current = teamList[i].allPlayersHead;
        while (current != NULL)
        {
            if (current->playerId == playerId)
            {
                return 1;
            }
            current = current->nextInTeam;
        }
    }
    return 0;
}

// Creates a new player node using malloc
PlayerRecord *createPlayer(int playerId, const char *name, const char *teamName, int role, int totalRuns, float battingAverage, float strikeRate, int wickets, float economyRate)
{
    PlayerRecord *newPlayer = (PlayerRecord *)malloc(sizeof(PlayerRecord));

    if (newPlayer == NULL)
    {
        printf("Memory allocation failed!\n");
        exit(1);
    }

    newPlayer->playerId = playerId;
    strncpy(newPlayer->name, name, MAX_NAME_LENGTH);
    newPlayer->name[MAX_NAME_LENGTH] = '\0';
    strncpy(newPlayer->teamName, teamName, MAX_NAME_LENGTH);
    newPlayer->teamName[MAX_NAME_LENGTH] = '\0';
    newPlayer->role = role;
    newPlayer->totalRuns = totalRuns;
    newPlayer->battingAverage = battingAverage;
    newPlayer->strikeRate = strikeRate;
    newPlayer->wickets = wickets;
    newPlayer->economyRate = economyRate;
    newPlayer->performanceIndex = calculatePerformanceIndex(role, battingAverage, strikeRate, wickets, economyRate);
    newPlayer->nextInTeam = NULL;
    newPlayer->nextInRole = NULL;
    return newPlayer;
}

// Adds a player to the team list, to the sorted role list, and updates team statistics
void insertPlayerIntoTeam(Team *team, PlayerRecord *newPlayer)
{
    PlayerRecord *current;
    PlayerRecord *previous = NULL;
    int role = newPlayer->role;

    newPlayer->nextInTeam = NULL;
    if (team->allPlayersHead == NULL)
    {
        team->allPlayersHead = newPlayer;
    }
    else
    {
        team->allPlayersTail->nextInTeam = newPlayer;
    }
    team->allPlayersTail = newPlayer;

    current = team->roleHead[role];
    while (current != NULL && current->performanceIndex >= newPlayer->performanceIndex)
    {
        previous = current;
        current = current->nextInRole;
    }
    newPlayer->nextInRole = current;
    if (previous == NULL)
    {
        team->roleHead[role] = newPlayer;
    }
    else
    {
        previous->nextInRole = newPlayer;
    }

    team->totalPlayers++;
    if (role == ROLE_BATSMAN || role == ROLE_ALL_ROUNDER)
    {
        team->sumBattingStrikeRate += newPlayer->strikeRate;
        team->battingPlayersCount++;
        team->averageBattingStrikeRate = team->sumBattingStrikeRate / team->battingPlayersCount;
    }
}

// Loads all teams and players from Players_data.h
void initializeTeamsAndPlayers()
{
    int i;
    int teamIndex;
    PlayerRecord *newPlayer;

    numberOfTeams = teamCount;
    for (i = 0; i < numberOfTeams; i++)
    {
        teamList[i].teamId = i + 1;
        strncpy(teamList[i].name, teams[i], MAX_NAME_LENGTH);
        teamList[i].name[MAX_NAME_LENGTH] = '\0';
        teamList[i].totalPlayers = 0;
        teamList[i].averageBattingStrikeRate = 0.0f;
        teamList[i].sumBattingStrikeRate = 0.0f;
        teamList[i].battingPlayersCount = 0;
        teamList[i].allPlayersHead = NULL;
        teamList[i].allPlayersTail = NULL;
        teamList[i].roleHead[1] = NULL;
        teamList[i].roleHead[2] = NULL;
        teamList[i].roleHead[3] = NULL;
    }

    for (i = 0; i < playerCount; i++)
    {
        teamIndex = findTeamIndexByName(players[i].team);
        if (teamIndex == -1)
        {
            printf("Team %s not found, skipping player %d\n", players[i].team, players[i].id);
            continue;
        }
        newPlayer = createPlayer(players[i].id, players[i].name, players[i].team, roleFromString(players[i].role), players[i].totalRuns, players[i].battingAverage, players[i].strikeRate, players[i].wickets, players[i].economyRate);
        insertPlayerIntoTeam(&teamList[teamIndex], newPlayer);
    }
}

// Prints the table heading used for player lists of one team
void printTeamPlayerHeader()
{
    printf("====================================================================================================\n");
    printf("%-5s %-24s %-12s %6s %6s %6s %5s %5s %10s\n", "ID", "Name", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf.Index");
    printf("====================================================================================================\n");
}

// Prints one player as a table row
void printPlayerRow(const PlayerRecord *player)
{
    printf("%-5d %-24s %-12s %6d %6.1f %6.1f %5d %5.1f %10.2f\n", player->playerId, player->name, roleToString(player->role), player->totalRuns, player->battingAverage, player->strikeRate, player->wickets, player->economyRate, player->performanceIndex);
}

// Menu option 1: reads player details and adds the player to a team
void addPlayerToTeam()
{
    int teamId, teamIndex;
    int playerId, role, totalRuns, wickets;
    float battingAverage, strikeRate, economyRate;
    char name[256];
    PlayerRecord *newPlayer;

    printf("Enter Team ID to add player: ");
    while (scanf("%d", &teamId) != 1 || teamId < MIN_ID || teamId > MAX_ID)
    {
        clearInputBuffer();
        printf("Invalid input. Enter a Team ID from %d to %d: ", MIN_ID, MAX_ID);
    }

    teamIndex = findTeamIndexById(teamId);
    if (teamIndex == -1)
    {
        printf("Team with ID %d does not exist.\n", teamId);
        return;
    }
    if (teamList[teamIndex].totalPlayers >= MAX_PLAYERS_PER_TEAM)
    {
        printf("Team %s already has the maximum of %d players.\n", teamList[teamIndex].name, MAX_PLAYERS_PER_TEAM);
        return;
    }

    printf("Enter Player Details:\n");

    printf("Player ID: ");
    while (scanf("%d", &playerId) != 1 || playerId < MIN_ID || playerId > MAX_ID)
    {
        clearInputBuffer();
        printf("Invalid input. Enter a Player ID from %d to %d: ", MIN_ID, MAX_ID);
    }
    if (playerIdExists(playerId))
    {
        printf("A player with ID %d already exists.\n", playerId);
        return;
    }

    printf("Name: ");
    clearInputBuffer();  // Consume the newline left by the previous scanf().
    if (!readPlayerName(name, sizeof(name)))
    {
        printf("Unable to read player name. Player was not added.\n");
        return;
    }

    printf("Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    while (scanf("%d", &role) != 1 || role < 1 || role > 3)
    {
        clearInputBuffer();
        printf("Invalid input. Enter 1, 2 or 3: ");
    }

    printf("Total Runs: ");
    while (scanf("%d", &totalRuns) != 1 || totalRuns < 0)
    {
        clearInputBuffer();
        printf("Invalid input. Enter a number >= 0: ");
    }

    printf("Batting Average: ");
    while (scanf("%f", &battingAverage) != 1 || battingAverage < 0)
    {
        clearInputBuffer();
        printf("Invalid input. Enter a number >= 0: ");
    }

    printf("Strike Rate: ");
    while (scanf("%f", &strikeRate) != 1 || strikeRate < 0)
    {
        clearInputBuffer();
        printf("Invalid input. Enter a number >= 0: ");
    }

    printf("Wickets: ");
    while (scanf("%d", &wickets) != 1 || wickets < 0)
    {
        clearInputBuffer();
        printf("Invalid input. Enter a number >= 0: ");
    }

    printf("Economy Rate: ");
    while (scanf("%f", &economyRate) != 1 || economyRate < 0)
    {
        clearInputBuffer();
        printf("Invalid input. Enter a number >= 0: ");
    }

    newPlayer = createPlayer(playerId, name, teamList[teamIndex].name, role, totalRuns, battingAverage, strikeRate, wickets, economyRate);
    insertPlayerIntoTeam(&teamList[teamIndex], newPlayer);
    printf("Player added successfully to Team %s!\n", teamList[teamIndex].name);
}

// Menu option 2: displays all players of a team with total and average strike rate
void displayPlayersOfTeam()
{
    int teamId, teamIndex;
    Team *team;
    PlayerRecord *current;

    printf("Enter Team ID: ");
    while (scanf("%d", &teamId) != 1 || teamId < MIN_ID || teamId > MAX_ID)
    {
        clearInputBuffer();
        printf("Invalid input. Enter a Team ID from %d to %d: ", MIN_ID, MAX_ID);
    }

    teamIndex = findTeamIndexById(teamId);
    if (teamIndex == -1)
    {
        printf("Team with ID %d does not exist.\n", teamId);
        return;
    }
    team = &teamList[teamIndex];

    printf("Players of Team %s:\n", team->name);
    printTeamPlayerHeader();
    current = team->allPlayersHead;
    while (current != NULL)
    {
        printPlayerRow(current);
        current = current->nextInTeam;
    }
    printf("====================================================================================================\n");
    printf("Total Players: %d\n", team->totalPlayers);
    printf("Average Batting Strike Rate: %.2f\n", team->averageBattingStrikeRate);
}

// Merges two sorted halves of the team pointer array (highest average first)
void mergeTeams(Team *arr[], Team *temp[], int left, int mid, int right)
{
    int i = left;
    int j = mid + 1;
    int k = left;

    while (i <= mid && j <= right)
    {
        if (arr[i]->averageBattingStrikeRate >= arr[j]->averageBattingStrikeRate)
        {
            temp[k++] = arr[i++];
        }
        else
        {
            temp[k++] = arr[j++];
        }
    }
    while (i <= mid)
    {
        temp[k++] = arr[i++];
    }
    while (j <= right)
    {
        temp[k++] = arr[j++];
    }
    for (k = left; k <= right; k++)
    {
        arr[k] = temp[k];
    }
}

// Merge sort on team pointers by average batting strike rate
void mergeSortTeams(Team *arr[], Team *temp[], int left, int right)
{
    int mid;

    if (left >= right)
    {
        return;
    }
    mid = (left + right) / 2;
    mergeSortTeams(arr, temp, left, mid);
    mergeSortTeams(arr, temp, mid + 1, right);
    mergeTeams(arr, temp, left, mid, right);
}

// Menu option 3: displays teams sorted by average batting strike rate (descending)
void displayTeamsByAverageStrikeRate()
{
    Team *sortedTeams[MAX_TEAMS];
    Team *tempArray[MAX_TEAMS];
    int i;

    for (i = 0; i < numberOfTeams; i++)
    {
        sortedTeams[i] = &teamList[i];
    }
    mergeSortTeams(sortedTeams, tempArray, 0, numberOfTeams - 1);

    printf("Teams Sorted by Average Batting Strike Rate\n");
    printf("=========================================================\n");
    printf("%-4s %-18s %-14s %s\n", "ID", "Team Name", "Avg Bat SR", "Total Players");
    printf("=========================================================\n");
    for (i = 0; i < numberOfTeams; i++)
    {
        printf("%-4d %-18s %-14.2f %d\n", sortedTeams[i]->teamId, sortedTeams[i]->name, sortedTeams[i]->averageBattingStrikeRate, sortedTeams[i]->totalPlayers);
    }
    printf("=========================================================\n");
}

// Menu option 4: displays top K players of a team for a role in O(K)
void displayTopKPlayersOfTeam()
{
    int teamId, teamIndex, role, k, shown;
    PlayerRecord *current;

    printf("Enter Team ID: ");
    while (scanf("%d", &teamId) != 1 || teamId < MIN_ID || teamId > MAX_ID)
    {
        clearInputBuffer();
        printf("Invalid input. Enter a Team ID from %d to %d: ", MIN_ID, MAX_ID);
    }

    teamIndex = findTeamIndexById(teamId);
    if (teamIndex == -1)
    {
        printf("Team with ID %d does not exist.\n", teamId);
        return;
    }

    printf("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    while (scanf("%d", &role) != 1 || role < 1 || role > 3)
    {
        clearInputBuffer();
        printf("Invalid input. Enter 1, 2 or 3: ");
    }

    printf("Enter number of players: ");
    while (scanf("%d", &k) != 1 || k < 1)
    {
        clearInputBuffer();
        printf("Invalid input. Enter a number >= 1: ");
    }

    printf("Top %d %s players of Team %s:\n", k, roleToString(role), teamList[teamIndex].name);
    printTeamPlayerHeader();

    shown = 0;
    current = teamList[teamIndex].roleHead[role];
    while (current != NULL && shown < k)
    {
        printPlayerRow(current);
        current = current->nextInRole;
        shown++;
    }
    printf("====================================================================================================\n");
    if (shown < k)
    {
        printf("Only %d %s player(s) available in this team.\n", shown, roleToString(role));
    }
}

// Moves the element at the given index down the heap until the max-heap order is restored
void heapifyDown(int index)
{
    int largest, leftChild, rightChild;
    PlayerRecord *temp;

    while (1)
    {
        largest = index;
        leftChild = 2 * index + 1;
        rightChild = 2 * index + 2;

        if (leftChild < heapSize && heap[leftChild]->performanceIndex > heap[largest]->performanceIndex)
        {
            largest = leftChild;
        }
        if (rightChild < heapSize && heap[rightChild]->performanceIndex > heap[largest]->performanceIndex)
        {
            largest = rightChild;
        }
        if (largest == index)
        {
            break;
        }

        temp = heap[index];
        heap[index] = heap[largest];
        heap[largest] = temp;
        index = largest;
    }
}

// Menu option 5: displays all players of a role from all teams in O(N log t) using a heap
void displayAllPlayersOfRole()
{
    int role, i, printedCount = 0;
    char title[20];
    PlayerRecord *best;

    printf("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    while (scanf("%d", &role) != 1 || role < 1 || role > 3)
    {
        clearInputBuffer();
        printf("Invalid input. Enter 1, 2 or 3: ");
    }

    heapSize = 0;
    for (i = 0; i < numberOfTeams; i++)
    {
        if (teamList[i].roleHead[role] != NULL)
        {
            heap[heapSize] = teamList[i].roleHead[role];
            heapSize++;
        }
    }
    for (i = heapSize / 2 - 1; i >= 0; i--)
    {
        heapifyDown(i);
    }

    if (role == ROLE_BATSMAN)
    {
        strcpy(title, "Batsmen");
    }
    else if (role == ROLE_BOWLER)
    {
        strcpy(title, "Bowlers");
    }
    else
    {
        strcpy(title, "All-rounders");
    }

    printf("%s of all teams:\n", title);
    printf("==========================================================================================================\n");
    printf("%-5s %-24s %-14s %-12s %6s %6s %6s %5s %5s %10s\n", "ID", "Name", "Team", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf.Index");
    printf("==========================================================================================================\n");

    while (heapSize > 0)
    {
        best = heap[0];
        printf("%-5d %-24s %-14s %-12s %6d %6.1f %6.1f %5d %5.1f %10.2f\n", best->playerId, best->name, best->teamName, roleToString(best->role), best->totalRuns, best->battingAverage, best->strikeRate, best->wickets, best->economyRate, best->performanceIndex);
        printedCount++;

        if (best->nextInRole != NULL)
        {
            heap[0] = best->nextInRole;
        }
        else
        {
            heap[0] = heap[heapSize - 1];
            heapSize--;
        }
        heapifyDown(0);
    }
    printf("==========================================================================================================\n");
    printf("Total %s: %d\n", title, printedCount);
}

// Frees all player nodes before the program ends
void freeAllPlayers()
{
    int i;
    PlayerRecord *current;
    PlayerRecord *next;

    for (i = 0; i < numberOfTeams; i++)
    {
        current = teamList[i].allPlayersHead;
        while (current != NULL)
        {
            next = current->nextInTeam;
            free(current);
            current = next;
        }
        teamList[i].allPlayersHead = NULL;
    }
}

// Prints the main menu
void printMenu()
{
    printf("\n==============================================================================\n");
    printf("ICC ODI Player Performance Analyzer\n");
    printf("==============================================================================\n");
    printf("1. Add Player to Team\n");
    printf("2. Display Players of a Specific Team\n");
    printf("3. Display Teams by Average Batting Strike Rate\n");
    printf("4. Display Top K Players of a Specific Team by Role\n");
    printf("5. Display all Players of specific role Across All Teams by performance index\n");
    printf("6. Exit\n");
    printf("==============================================================================\n");
}

// Loads the data and runs the menu until the user exits
int main()
{
    int choice;

    initializeTeamsAndPlayers();

    do
    {
        printMenu();
        printf("Enter your choice: ");
        while (scanf("%d", &choice) != 1 || choice < 1 || choice > 6)
        {
            clearInputBuffer();
            printf("Invalid input. Enter a choice from 1 to 6: ");
        }
        printf("\n");

        switch (choice)
        {
            case 1:
                addPlayerToTeam();
                break;
            case 2:
                displayPlayersOfTeam();
                break;
            case 3:
                displayTeamsByAverageStrikeRate();
                break;
            case 4:
                displayTopKPlayersOfTeam();
                break;
            case 5:
                displayAllPlayersOfRole();
                break;
            case 6:
                printf("Exiting program. Goodbye!\n");
                break;
        }
    } while (choice != 6);

    freeAllPlayers();
    return 0;
}