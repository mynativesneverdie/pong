#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#define USLEEP_TIME 100000
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

typedef struct  s_state
{
	int player_1_Y; // players position
    int player_2_Y;

    int ball_X;     // ball position and direction
    int ball_Y;
    int vector_X;
    int vector_Y;

    int score_Player_1;  // SCORE
    int score_Player_2;
}               t_state;

void * screenRendering(void * state);
void playerMove(int *player_1_Y, int *player_2_Y);
void * moveBall(void * state);
int scoreDisplay(t_state * state);
void clearDisplay(void);
void initState(t_state *state);
void prepareTerminal(struct termios	*term, struct termios *saved_term);
void createBallThread(t_state * state, pthread_t *id);
void createRenderingThread(t_state * state, pthread_t *id);

int g_run_program = 1;

int main(void) {
    struct termios	term;
    struct termios	saved_term;
    t_state         state;
    pthread_t       id_moveBall_thread;
    pthread_t       id_rendering_thread;

    initState(&state);
    prepareTerminal(&term, &saved_term);
    createRenderingThread(&state, &id_rendering_thread);
    createBallThread(&state, &id_moveBall_thread);

    while (g_run_program) {
        playerMove(&state.player_1_Y, &state.player_2_Y);
        if (scoreDisplay(&state))
            break;
    }
    pthread_join(id_moveBall_thread, NULL);
    pthread_join(id_rendering_thread, NULL);
    tcsetattr(0, TCSAFLUSH, &saved_term);
    return 0;
}

void initState(t_state *state) {
    state->player_1_Y = PLAYER1_START_Y;
    state->player_2_Y = PLAYER2_START_Y;
    state->ball_X = BALL_START_X;
    state->ball_Y = BALL_START_Y;
    state->vector_X = -1;
    state->vector_Y = 1;
    state->score_Player_1 = 0;
    state->score_Player_2 = 0;
}

void fatalThreadCreating(void) {
    printf("Can't create thread\n");
    exit(1);
}

void createBallThread(t_state * state, pthread_t *id) {
    int ret = pthread_create(id, NULL, moveBall, state);
    if (ret != 0)
        fatalThreadCreating();
}

void createRenderingThread(t_state * state, pthread_t *id) {
    int ret = pthread_create(id, NULL, screenRendering, state);
    if (ret != 0)
        fatalThreadCreating();
}

void * screenRendering(void * s) {

    const t_state * state = s;

    while (g_run_program) {

        usleep(USLEEP_TIME);
        clearDisplay();

        // rendering score-bar
        for (int y = 0; y < PERSON_SIZE; ++y) {
            for (int x = 0; x < FIELD_WIDTH; ++x) {
                if ((y == 0 && x != 0 && x != FIELD_WIDTH - 1) ||
                    (y == PERSON_SIZE - 1 && x != 0 && x != FIELD_WIDTH - 1)) {
                    printf("#");
                } else if (((x == 0 && y != 0 && y != 2) ||
                            (x == FIELD_WIDTH - 1 && y != 0 && y != 2))) {
                    printf("|");
                } else if (x == POS_SCORE_CHARACTERS) {
                    printf("Player 1: %d | Player 2: %d", state->score_Player_1, state->score_Player_2);
                }  else if (y == 1 && x < FIELD_WIDTH - POS_SCORE_CHARACTERS) {
                    printf(" ");
                } else if (y != 1) {
                    printf(" ");
                }
            }
            printf("\n");
        }

        // rendering field + ball + players
        for (int y = 0; y < FIELD_HEIGHT; ++y) {
            for (int x = 0; x < FIELD_WIDTH; ++x) {
                if ((x == PLAYER1_DEFAULT_X &&
                    (y == state->player_1_Y - 1 || y == state->player_1_Y || y == state->player_1_Y + 1)) ||
                    (x == PLAYER2_DEFAULT_X &&
                    (y == state->player_2_Y - 1 || y == state->player_2_Y|| y == state->player_2_Y + 1))) {
                    printf("|");
                } else if (x == state->ball_X && y == state->ball_Y) {
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
    }
    return (NULL);
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

void * moveBall(void * s) {  // constants function - for all
    t_state * state = s;

    while (g_run_program) {
        usleep(USLEEP_TIME);

        if (state->ball_Y == 1 || state->ball_Y == FIELD_HEIGHT - 2) {  // if move_ball 1 or move_ball 80
            state->vector_Y *= -1;
        }
        if (state->ball_X == PLAYER1_DEFAULT_X + 1 &&  // move from player1
            (state->ball_Y == state->player_1_Y - 1 || state->ball_Y == state->player_1_Y  || state->ball_Y == state->player_1_Y + 1)) {
            state->vector_X *= -1;
        }
        if (state->ball_X == PLAYER2_DEFAULT_X - 1 &&  // move from player2
            (state->ball_Y == state->player_2_Y - 1 || state->ball_Y == state->player_2_Y  || state->ball_Y == state->player_2_Y + 1)) {
            state->vector_X *= -1;
        }

        state->ball_Y += state->vector_Y;
        state->ball_X += state->vector_X;
    }
    return (NULL);
}

int scoreDisplay(t_state * state) {
    if (state->ball_X < 2) {
        state->score_Player_2 += 1;
        state->ball_X = BALL_START_X;
        state->ball_Y = BALL_START_Y;
    }
    if (state->ball_X > FIELD_WIDTH - 3) {
        state->score_Player_1 += 1;
        state->ball_X = BALL_START_X;
        state->ball_Y = BALL_START_Y;
    }
    if (state->score_Player_1 == MAX_SCORE_TO_WIN) {
        printf("\nGoodGame_GLHF!!! PLAYER 1 WINS.\n%d - %d\n", state->score_Player_1, state->score_Player_2);
        return TRUE;
    } else if (state->score_Player_2 == MAX_SCORE_TO_WIN) {
        printf("\nGoodGame_GLHF!!! PLAYER 2 WINS.\n%d - %d\n", state->score_Player_2, state->score_Player_1);
        return TRUE;
    }
    return FALSE;
}

void clearDisplay(void) {
    system("clear");
}

void	sigint_handler(int	sig)
{
	if (sig > 0)
	{
		g_run_program = 0;
	}
}

void prepareTerminal(struct termios	*term, struct termios *saved_term) {
    tcgetattr(0, saved_term);
    memcpy(term, saved_term, sizeof(struct termios));
	term->c_lflag &= ~(ICANON | ECHO);
	term->c_cc[VMIN] = 1;
	term->c_cc[VTIME] = 0;
    tcsetattr(0, TCSAFLUSH, term);
    signal(SIGINT, sigint_handler);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}