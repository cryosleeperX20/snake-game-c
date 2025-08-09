#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#define WIDTH 40
#define HEIGHT 20
#define MAX_SNAKE_LENGTH 100
#define HIGHSCORE_FILE "highscores.txt"

typedef struct {
    int x, y;
} Point;

Point snake[MAX_SNAKE_LENGTH];
int snake_length;
Point food;
int score;
int game_over;

void init_game();
void draw_game();
void move_snake(int dx, int dy);
void spawn_food();
void menu();
void high_scores();
void controls();
void save_score(int score);
void load_scores(int scores[], int *count);

int main() {
    initscr();
    noecho();
    cbreak();
    curs_set(FALSE);
    keypad(stdscr, TRUE);

    menu();

    endwin();
    return 0;
}

void menu() {
    char *choices[] = {"New Game", "High Scores", "Controls", "Exit"};
    int choice_count = 4;
    int highlight = 0;
    int ch;

    while (1) {
        clear();
        mvprintw(2, 2, "Snake Game");
        for (int i = 0; i < choice_count; i++) {
            if (i == highlight)
                mvprintw(4 + i, 4, "> %s", choices[i]);
            else
                mvprintw(4 + i, 6, "%s", choices[i]);
        }

        ch = getch();
        if (ch == KEY_UP) {
            highlight = (highlight - 1 + choice_count) % choice_count;
        } else if (ch == KEY_DOWN) {
            highlight = (highlight + 1) % choice_count;
        } else if (ch == '\n') {
            if (highlight == 0) { // New Game
                init_game();
            } else if (highlight == 1) {
                high_scores();
            } else if (highlight == 2) {
                controls();
            } else {
                break;
            }
        }
    }
}

void init_game() {
    snake_length = 3;
    snake[0] = (Point){WIDTH/2, HEIGHT/2};
    snake[1] = (Point){WIDTH/2 - 1, HEIGHT/2};
    snake[2] = (Point){WIDTH/2 - 2, HEIGHT/2};
    score = 0;
    game_over = 0;

    srand(time(NULL));
    spawn_food();

    int dx = 1, dy = 0;
    nodelay(stdscr, TRUE);

    while (!game_over) {
        int ch = getch();
        if (ch == KEY_UP && dy == 0) { dx = 0; dy = -1; }
        else if (ch == KEY_DOWN && dy == 0) { dx = 0; dy = 1; }
        else if (ch == KEY_LEFT && dx == 0) { dx = -1; dy = 0; }
        else if (ch == KEY_RIGHT && dx == 0) { dx = 1; dy = 0; }
        else if (ch == 'q') game_over = 1;

        move_snake(dx, dy);
        draw_game();
        napms(100);
    }

    nodelay(stdscr, FALSE);
    mvprintw(HEIGHT+2, 2, "Game Over! Final Score: %d", score);

    // Save score to file
    save_score(score);

    getch();
}

void move_snake(int dx, int dy) {
    Point new_head = {snake[0].x + dx, snake[0].y + dy};

    // Collision with walls
    if (new_head.x < 0 || new_head.x >= WIDTH || new_head.y < 0 || new_head.y >= HEIGHT) {
        game_over = 1;
        return;
    }
    // Collision with self
    for (int i = 0; i < snake_length; i++) {
        if (snake[i].x == new_head.x && snake[i].y == new_head.y) {
            game_over = 1;
            return;
        }
    }

    // Move snake
    for (int i = snake_length; i > 0; i--) {
        snake[i] = snake[i - 1];
    }
    snake[0] = new_head;

    // Eat food
    if (new_head.x == food.x && new_head.y == food.y) {
        snake_length++;
        if (snake_length > MAX_SNAKE_LENGTH) snake_length = MAX_SNAKE_LENGTH;
        score += 10;
        spawn_food();
    }
}

void draw_game() {
    clear();
    // Draw border
    for (int x = 0; x < WIDTH; x++) {
        mvprintw(0, x, "#");
        mvprintw(HEIGHT-1, x, "#");
    }
    for (int y = 0; y < HEIGHT; y++) {
        mvprintw(y, 0, "#");
        mvprintw(y, WIDTH-1, "#");
    }

    // Draw snake
    for (int i = 0; i < snake_length; i++) {
        mvprintw(snake[i].y, snake[i].x, i == 0 ? "O" : "o");
    }

    // Draw food
    mvprintw(food.y, food.x, "*");

    // Score
    mvprintw(HEIGHT, 2, "Score: %d", score);

    refresh();
}

void spawn_food() {
    int valid = 0;
    while (!valid) {
        valid = 1;
        food.x = rand() % (WIDTH-2) + 1;
        food.y = rand() % (HEIGHT-2) + 1;
        for (int i = 0; i < snake_length; i++) {
            if (snake[i].x == food.x && snake[i].y == food.y) {
                valid = 0;
                break;
            }
        }
    }
}

void high_scores() {
    clear();
    mvprintw(2, 2, "High Scores:");

    int scores[20], count = 0;
    load_scores(scores, &count);

    for (int i = 0; i < count && i < 10; i++) {
        mvprintw(4 + i, 4, "%d. %d", i + 1, scores[i]);
    }

    if (count == 0) {
        mvprintw(4, 4, "No scores yet.");
    }

    mvprintw(HEIGHT, 2, "Press any key to return.");
    getch();
}

void controls() {
    clear();
    mvprintw(2, 2, "Controls:");
    mvprintw(4, 2, "Arrow Keys - Move");
    mvprintw(5, 2, "Q - Quit Game");
    mvprintw(7, 2, "Press any key to return.");
    getch();
}

void save_score(int score) {
    FILE *f = fopen(HIGHSCORE_FILE, "a");
    if (f) {
        fprintf(f, "%d\n", score);
        fclose(f);
    }
}

void load_scores(int scores[], int *count) {
    FILE *f = fopen(HIGHSCORE_FILE, "r");
    *count = 0;
    if (f) {
        while (fscanf(f, "%d", &scores[*count]) == 1 && *count < 20) {
            (*count)++;
        }
        fclose(f);

        // Sort scores in descending order
        for (int i = 0; i < *count - 1; i++) {
            for (int j = i + 1; j < *count; j++) {
                if (scores[j] > scores[i]) {
                    int tmp = scores[i];
                    scores[i] = scores[j];
                    scores[j] = tmp;
                }
            }
        }
    }
}
