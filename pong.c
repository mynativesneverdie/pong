#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define FIELD_HEIGHT 27
#define FIELD_WIDTH 82
#define PERSON_SIZE 3
#define PLAYER1_START_Y 13
#define PLAYER2_START_Y 13
#define PLAYER1_DEFAULT_X 2
#define PLAYER2_DEFAULT_X (FIELD_WIDTH - 3)
#define BALL_START_X (FIELD_WIDTH / 2)
#define BALL_START_Y (FIELD_HEIGHT / 2)
#define POS_SCORE_CHARACTERS 29
#define MAX_SCORE_TO_WIN 21
#define TRUE 1
#define FALSE 0

void screenRendering(const int *player_1_Y, const int *player_2_Y,
                     const int *ball1_x, const int *ball1_y,
                     int *scPlayer1, int *scPlayer2);
void playerMove(int *player_1_Y, int *player_2_Y);
void moveBall(int *ball1_x, int *ball1_y,
              int *vector_X, int *vector_Y,
              const int *player_1_Y, const int *player_2_Y);
int scoreDisplay(int *scPlayer1, int *scPlayer2,
                  int *ball1_x, int *ball1_y);
void clearDisplay(void);

struct termios	*g_saved_term;

void prepareTerminal(struct termios	*term) {
    g_saved_term = (struct termios *)malloc(sizeof(struct termios));
    term = (struct termios *)malloc(sizeof(struct termios));
    tcgetattr(0, g_saved_term);
    memcpy(term, g_saved_term, sizeof(struct termios));
	term->c_lflag &= ~(ICANON | ECHO);
	term->c_cc[VMIN] = 1;
	term->c_cc[VTIME] = 0;
    tcsetattr(0, TCSAFLUSH, term);
}

void	sigint_handler(int	sig)
{
	if (sig > 0)
	{
		tcsetattr(0, TCSAFLUSH, g_saved_term);
        exit(0);
	}
}


int main(void) {
    int player_1_Y = PLAYER1_START_Y;  // players position
    int player_2_Y = PLAYER2_START_Y;

    int ball_X = BALL_START_X;  // ball position and direction
    int ball_Y = BALL_START_Y;
    int vector_X = -1;
    int vector_Y = 1;

    int score_Player_1 = 0;  // SCORE
    int score_Player_2 = 0;

    
	struct termios	*term;

    prepareTerminal(term);
    signal(SIGINT, sigint_handler);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    while (TRUE) {
            screenRendering(&player_1_Y, &player_2_Y,
                            &ball_X, &ball_Y,
                            &score_Player_1, &score_Player_2);
            playerMove(&player_1_Y, &player_2_Y);
            moveBall(&ball_X, &ball_Y,
                     &vector_X, &vector_Y,
                     &player_1_Y, &player_2_Y);
            clearDisplay();

            if (scoreDisplay(&score_Player_1, &score_Player_2,
                             &ball_X, &ball_Y)) {
                break;
            }
    }
    tcsetattr(0, TCSAFLUSH, g_saved_term);
    return 0;
}

void screenRendering(const int *player_1_Y, const int *player_2_Y,
                    const int *ball1_x, const int *ball1_y,
                    int *scPlayer1, int *scPlayer2) {
    for (int y = 0; y < PERSON_SIZE; ++y) {  // rendering score-bar
        for (int x = 0; x < FIELD_WIDTH; ++x) {
            if ((y == 0 && x != 0 && x != FIELD_WIDTH - 1) ||
                (y == PERSON_SIZE - 1 && x != 0 && x != FIELD_WIDTH - 1)) {
                printf("#");
            } else if (((x == 0 && y != 0 && y != 2) ||
                        (x == FIELD_WIDTH - 1 && y != 0 && y != 2))) {
                printf("|");
            } else if (x == POS_SCORE_CHARACTERS) {
                printf("Player 1: %d | Player 2: %d", *scPlayer1, *scPlayer2);
            }  else if (y == 1 && x < FIELD_WIDTH - POS_SCORE_CHARACTERS) {
                printf(" ");
            } else if (y != 1) {
                printf(" ");
            }
        }
        printf("\n");
    }

    for (int y = 0; y < FIELD_HEIGHT; ++y) {  // rendering field + ball + players
        for (int x = 0; x < FIELD_WIDTH; ++x) {
            if ((x == PLAYER1_DEFAULT_X &&
                (y == *player_1_Y - 1 || y == *player_1_Y || y == *player_1_Y + 1)) ||
                (x == PLAYER2_DEFAULT_X &&
                (y == *player_2_Y - 1 || y == *player_2_Y || y == *player_2_Y + 1))) {
                printf("|");
            } else if (x == *ball1_x && y == *ball1_y) {
                printf("o");
            } else if ((y == 0 && x != 0 && x != FIELD_WIDTH - 1) ||
                        (y == FIELD_HEIGHT - 1 && x != 0 && x != FIELD_WIDTH - 1)) {
                printf("-");
            } else if ((x == 0 && y != 0 && y != FIELD_HEIGHT - 1) ||
                        (x == FIELD_WIDTH - 1 && y != 0 && y != FIELD_HEIGHT - 1) ||
                        (x == FIELD_WIDTH / 2)) {
                printf("|");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
    usleep(100000);
}

void playerMove(int *player_1_Y, int *player_2_Y) {
    char temp;
    switch (temp = getchar()) {
        case 'a' : ((*player_1_Y - 2) != 0) ? (*player_1_Y -= 1) : *player_1_Y; break;
        case 'z' : ((*player_1_Y + 2) != FIELD_HEIGHT - 1) ? (*player_1_Y += 1) : *player_1_Y; break;
        case 'k' : ((*player_2_Y - 2) != 0) ? (*player_2_Y -= 1) : *player_2_Y; break;
        case 'm' : ((*player_2_Y + 2) != FIELD_HEIGHT - 1) ? (*player_2_Y += 1) : *player_2_Y; break;
    }
}

void moveBall(int *ball1_x, int *ball1_y,
              int *vector_X, int *vector_Y,
                const int *player_1_Y, const int *player_2_Y) {  // constants function - for all.
    if (*ball1_y == 1 || *ball1_y == FIELD_HEIGHT - 2) {  // if move_ball 1 or move_ball 80
        *vector_Y = -(*vector_Y);
    }
    if (*ball1_x == PLAYER1_DEFAULT_X + 1 &&  // move from player1
        (*ball1_y == *player_1_Y - 1 || *ball1_y == *player_1_Y  || *ball1_y == *player_1_Y + 1)) {
        *vector_X = -(*vector_X);
    }
    if (*ball1_x == PLAYER2_DEFAULT_X - 1 &&  // move from player2
        (*ball1_y == *player_2_Y - 1 || *ball1_y == *player_2_Y  || *ball1_y == *player_2_Y + 1)) {
        *vector_X = -(*vector_X);
    }

    *ball1_y += *vector_Y;
    *ball1_x += *vector_X;
}

int scoreDisplay(int *scPlayer1, int *scPlayer2,
                 int *ball1_x, int *ball1_y) {
    if (*ball1_x < 2) {
        *scPlayer2 += 1;
        *ball1_x = BALL_START_X;
        *ball1_y = BALL_START_Y;
    }
    if (*ball1_x > FIELD_WIDTH - 3) {
        *scPlayer1 += 1;
        *ball1_x = BALL_START_X;
        *ball1_y = BALL_START_Y;
    }
    if (*scPlayer1 == MAX_SCORE_TO_WIN) {
        printf("\nGoodGame_GLHF!!! PLAYER 1 WINS.\n%d - %d\n", *scPlayer1, *scPlayer2);
        return TRUE;
    } else if (*scPlayer2 == MAX_SCORE_TO_WIN) {
        printf("\nGoodGame_GLHF!!! PLAYER 2 WINS.\n%d - %d\n", *scPlayer2, *scPlayer1);
        return TRUE;
    }
    return FALSE;
}

void clearDisplay(void) {
    system("clear");
}