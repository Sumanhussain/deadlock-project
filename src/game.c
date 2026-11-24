#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <errno.h>

/* ---------- CONSTANT DEFINITIONS ---------- */
#define MAX_PLAYERS_PER_GAME 5      // Maximum players allowed in one game
#define MAX_RESOURCES 5             // Maximum shared resources
#define MAX_MARKS 20                // Maximum score a player can have
#define MAX_UNIQUE_PLAYERS 20       // Maximum players in tournament
#define MAX_PLAYER_NAME 20          // Maximum player name length

/* ---------- COLOR CODES FOR OUTPUT ---------- */
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define CYAN    "\033[1;36m"
#define PURPLE  "\033[1;35m"
#define RESET   "\033[0m"

/* ---------- INPUT CLEANUP (handles EOF safely) ---------- */
void clearInput() {
    int c;
    while((c = getchar()) != '\n' && c != EOF);
    if(c == EOF) {
        printf(RED "\n Input ended unexpectedly. Exiting.\n" RESET);
        exit(EXIT_FAILURE);
    }
}

/* ---------- STRUCT DEFINITIONS ---------- */
typedef struct {
    int id;                     
    int players;                
    int rounds;                 
    int *turn;                  
    pthread_mutex_t *lock;     
    pthread_cond_t *cond;      
    int *resources;             
    int resCount;               
    int *score;                 
    int *resUsed;               
    int *deadlockCount;         
    char names[MAX_PLAYERS_PER_GAME][MAX_PLAYER_NAME]; 
} Player;

typedef struct {
    char name[MAX_PLAYER_NAME]; 
    int score;                  
    int deadlocks;              
} TournamentPlayer;

typedef struct {
    int score[MAX_PLAYERS_PER_GAME];
    int resUsed[MAX_PLAYERS_PER_GAME];
    int deadlockCount[MAX_PLAYERS_PER_GAME];
} GameResult;

/* ---------- FUNCTION TO PRINT CURRENT TIME ---------- */
void logTime() {
    time_t now = time(NULL);
    if (now == (time_t)-1) return;
    
    struct tm *t = localtime(&now);
    if (t == NULL) return;
    
    printf(CYAN "[%02d:%02d:%02d] " RESET,
           t->tm_hour, t->tm_min, t->tm_sec);
}

/* ---------- THREAD FUNCTION ---------- */
void* play(void* arg) {
    Player* p = (Player*)arg;
    
    for(int r = 0; r < p->rounds; r++) {
        pthread_mutex_lock(p->lock);
        while(*(p->turn) != p->id)
            pthread_cond_wait(p->cond, p->lock);
        
        if(p->id == 0) {
            for(int i = 0; i < p->resCount; i++) {
                p->resources[i] = 1;
            }
        }
        
        int need = (rand() % p->resCount) + 1;
        logTime();
        printf(BLUE " %-10s | Round %-2d | Needs %-2d resources\n" RESET,
               p->names[p->id], r + 1, need);
        
        int available = 0;
        for(int i = 0; i < p->resCount; i++)
            if(p->resources[i]) available++;
        
        if(available < need) {
            logTime();
            printf(RED " DEADLOCK  | %s moved to waiting queue\n" RESET,
                   p->names[p->id]);
            
            p->deadlockCount[p->id]++;
            if(p->score[p->id] > 2) p->score[p->id] -= 2;
            else p->score[p->id] = 0;
            
            logTime();
            printf(YELLOW " Recovery: Resetting all resources\n" RESET);
            
            for(int i = 0; i < p->resCount; i++) p->resources[i] = 1;
            
            sleep(1);
            logTime();
            printf(GREEN " Deadlock recovered successfully\n\n" RESET);
        } else {
            int used = 0;
            for(int i = 0; i < p->resCount && used < need; i++) {
                if(p->resources[i]) {
                    p->resources[i] = 0;
                    used++;
                }
            }
            
            logTime();
            printf(GREEN " %s acquired resources successfully\n\n" RESET,
                   p->names[p->id]);
            
            p->score[p->id] += need * 5;
            if(p->score[p->id] > MAX_MARKS) p->score[p->id] = MAX_MARKS;
            
            p->resUsed[p->id] += need;
        }
        
        *(p->turn) = (*(p->turn) + 1) % p->players;
        pthread_cond_broadcast(p->cond);
        pthread_mutex_unlock(p->lock);
        sleep(1);
    }
    
    pthread_exit(NULL);
}

/* ---------- MAIN FUNCTION ---------- */
int main() {
    srand(time(NULL));
    
    TournamentPlayer tPlayers[MAX_UNIQUE_PLAYERS];
    int tCount = 0;
    int totalGames = 0;
    int grandTotalDeadlocks = 0;
    
    while(1) {
        printf(YELLOW " Enter total number of games: " RESET);
        if(scanf("%d", &totalGames) != 1) {
            printf(RED " Invalid input. Please enter a number.\n" RESET);
            clearInput();
            continue;
        }
        if(totalGames >= 1) break;
        printf(RED " Total games must be at least 1. Try again.\n" RESET);
    }
    
    for(int g = 1; g <= totalGames; g++) {
        int players = 0, rounds = 0, resources = 0;
        char playerNames[MAX_PLAYERS_PER_GAME][MAX_PLAYER_NAME];
        
        printf(PURPLE "\n═══════════════════════════════\n");
        printf("            GAME %d            \n", g);
        printf("═══════════════════════════════\n" RESET);
        
        while(1) {
            printf(" Enter number of players (1-%d): ", MAX_PLAYERS_PER_GAME);
            if(scanf("%d", &players) != 1) {
                printf(RED " Invalid input.\n" RESET);
                clearInput();
                continue;
            }
            if(players >= 1 && players <= MAX_PLAYERS_PER_GAME) break;
            printf(RED " Players must be between 1 and %d.\n" RESET,
                   MAX_PLAYERS_PER_GAME);
        }
        
        for(int i = 0; i < players; i++) {
            printf(" Enter name of player %d: ", i + 1);
            if(scanf("%19s", playerNames[i]) != 1) {
                printf(RED " Failed to read name. Using default.\n" RESET);
                snprintf(playerNames[i], MAX_PLAYER_NAME, "Player%d", i+1);
            }
        }
        
        while(1) {
            printf(" Enter rounds: ");
            if(scanf("%d", &rounds) != 1) {
                printf(RED " Invalid input.\n" RESET);
                clearInput();
                continue;
            }
            if(rounds >= 1) break;
            printf(RED " Rounds must be at least 1.\n" RESET);
        }
        
        while(1) {
            printf(" Enter resources (1-%d): ", MAX_RESOURCES);
            if(scanf("%d", &resources) != 1) {
                printf(RED " Invalid input.\n" RESET);
                clearInput();
                continue;
            }
            if(resources >= 1 && resources <= MAX_RESOURCES) break;
            printf(RED " Resources must be between 1 and %d.\n" RESET,
                   MAX_RESOURCES);
        }
        
        int fd[2];
        if(pipe(fd) == -1) {
            perror(RED " Pipe creation failed" RESET);
            exit(EXIT_FAILURE);
        }
        
        fflush(stdout);
        pid_t pid = fork();
        if(pid == -1) {
            perror(RED " Fork failed" RESET);
            close(fd[0]);
            close(fd[1]);
            exit(EXIT_FAILURE);
        }
        
        if(pid == 0) { // CHILD
            close(fd[0]);
            pthread_t threads[MAX_PLAYERS_PER_GAME];
            Player p[MAX_PLAYERS_PER_GAME];
            int res[MAX_RESOURCES] = {0};
            int score[MAX_PLAYERS_PER_GAME] = {0};
            int resUsed[MAX_PLAYERS_PER_GAME] = {0};
            int deadlockCount[MAX_PLAYERS_PER_GAME] = {0};
            int turn = 0;
            pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
            pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
            
            for(int i = 0; i < resources; i++) res[i] = 1;
            
            printf(GREEN "\n GAME %d STARTED \n\n" RESET, g);
            
            for(int i = 0; i < players; i++) {
                for(int j = 0; j < players; j++) {
                    strncpy(p[i].names[j], playerNames[j], MAX_PLAYER_NAME-1);
                    p[i].names[j][MAX_PLAYER_NAME-1] = '\0';
                }
                
                p[i].id = i;
                p[i].players = players;
                p[i].rounds = rounds;
                p[i].turn = &turn;
                p[i].lock = &lock;
                p[i].cond = &cond;
                p[i].resources = res;
                p[i].resCount = resources;
                p[i].score = score;
                p[i].resUsed = resUsed;
                p[i].deadlockCount = deadlockCount;
                
                if(pthread_create(&threads[i], NULL, play, &p[i]) != 0) {
                    perror(RED " Thread creation failed" RESET);
                    exit(EXIT_FAILURE);
                }
            }
            
            for(int i = 0; i < players; i++)
                pthread_join(threads[i], NULL);
            
            GameResult result;
            for(int i = 0; i < players; i++) {
                result.score[i] = score[i];
                result.resUsed[i] = resUsed[i];
                result.deadlockCount[i] = deadlockCount[i];
            }
            
            write(fd[1], &result, sizeof(GameResult));
            close(fd[1]);
            exit(EXIT_SUCCESS);
        } else { // PARENT
            GameResult result;
            memset(&result, 0, sizeof(GameResult));
            close(fd[1]);
            wait(NULL);
            read(fd[0], &result, sizeof(GameResult));
            close(fd[0]);
            
            printf(BLUE "\n GAME %d RESULT \n" RESET, g);
            printf("┌────────────┬───────┬───────────┬──────────────┐\n");
            printf("│ Player     │ Score │ Deadlocks │ ResourcesUsed│\n");
            printf("├────────────┼───────┼───────────┼──────────────┤\n");
            for(int i = 0; i < players; i++) {
                printf("│ %-10s │ %-5d │ %-9d │ %-12d │\n",
                       playerNames[i], result.score[i],
                       result.deadlockCount[i], result.resUsed[i]);
                grandTotalDeadlocks += result.deadlockCount[i];
            }
            printf("└────────────┴───────┴───────────┴──────────────┘\n");
            
            // Determine winner
            int maxScore = result.score[0];
            int winners[MAX_PLAYERS_PER_GAME], wc = 0;
            winners[wc++] = 0;
            for(int i = 1; i < players; i++) {
                if(result.score[i] > maxScore) { maxScore = result.score[i]; wc=0; winners[wc++]=i; }
                else if(result.score[i] == maxScore) winners[wc++] = i;
            }
            
            int finalWinner = winners[0];
            if(wc > 1) {
                int minDead = result.deadlockCount[winners[0]];
                for(int i=1;i<wc;i++)
                    if(result.deadlockCount[winners[i]]<minDead) { minDead=result.deadlockCount[winners[i]]; finalWinner=winners[i]; }
            }
            
            printf(GREEN " GAME %d WINNER: %s \n\n" RESET,
                   g, playerNames[finalWinner]);
            
            for(int i = 0; i < players; i++) {
                int found = -1;
                for(int j = 0; j < tCount; j++)
                    if(strcmp(tPlayers[j].name, playerNames[i]) == 0) { found=j; break; }
                
                if(found != -1) { tPlayers[found].score += result.score[i]; tPlayers[found].deadlocks += result.deadlockCount[i]; }
                else if(tCount < MAX_UNIQUE_PLAYERS) {
                    strncpy(tPlayers[tCount].name, playerNames[i], MAX_PLAYER_NAME-1);
                    tPlayers[tCount].name[MAX_PLAYER_NAME-1]='\0';
                    tPlayers[tCount].score = result.score[i];
                    tPlayers[tCount].deadlocks = result.deadlockCount[i];
                    tCount++;
                }
            }
        }
    }
    
    // Tournament summary
    printf(PURPLE "\n═══════════════════════════════\n");
    printf("        TOURNAMENT RESULT       \n");
    printf("═══════════════════════════════\n" RESET);
    for(int i=0;i<tCount;i++)
        printf(" %-12s | Score: %-4d | Deadlocks: %-4d\n",
               tPlayers[i].name, tPlayers[i].score, tPlayers[i].deadlocks);
    
    // Server report
    printf(YELLOW "\n SERVER REPORT\n" RESET);
    printf("• Total Games Played : %d\n", totalGames);
    printf("• Total Players      : %d\n", tCount);
    printf("• Total Deadlocks    : %d\n", grandTotalDeadlocks);
    printf(GREEN "✔ Server shutting down safely...\n\n" RESET);
    
    return 0;
}
