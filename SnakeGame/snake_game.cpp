#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>

#define WIDTH 20
#define HEIGHT 20
#define MAX_SNAKE_SIZE WIDTH *HEIGHT

typedef struct
{
    int x, y;
} point;

point snake_[MAX_SNAKE_SIZE];
point fruit_;
int snake_length_ = 0;
bool game_over_ = false;
struct termios org_term_;
struct termios new_term_;
volatile sig_atomic_t flag_ = 0;

void HideEcho(bool value)
{
  if(value == true)
  {
    tcgetattr(STDIN_FILENO, &org_term_);

    new_term_ = org_term_;

    new_term_.c_lflag &= ~(ECHO | ICANON);

    new_term_.c_cc[VMIN] = 0;
    new_term_.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &new_term_);
  }
  else tcsetattr(STDIN_FILENO, TCSANOW, &org_term_);

  return;
}

int ReturnInputKey()
{
    struct termios org_term;

    char input_key = 0;

    tcgetattr(STDIN_FILENO, &org_term);

    struct termios new_term = org_term;

    new_term.c_lflag &= ~(ECHO | ICANON);

    new_term.c_cc[VMIN] = 0;
    new_term.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

    read(STDIN_FILENO, &input_key, 1);

    tcsetattr(STDIN_FILENO, TCSANOW, &org_term);

    tcflush(STDIN_FILENO, TCIFLUSH);

    return input_key;
}

void HandleSignal(int signal)
{
    flag_ = 1;
    HideEcho(false);
    printf("\033[?25h");
    printf("\033[2J\033[1;1H");
}

void PlaceFruit()
{
    while (1)
    {
        fruit_.x = rand() % (WIDTH - 2) + 1;
        fruit_.y = rand() % (HEIGHT - 2) + 1;

        int overlap = 0;
        for (int i = 0; i < snake_length_; i++)
        {
            if (snake_[i].x == fruit_.x && snake_[i].y == fruit_.y)
            {
                overlap = 1;
                break;
            }
        }
        if (!overlap) break;
    }
}

void init_snake()
{
    snake_length_ = 5;

    for (int i = 0; i < snake_length_; i++)
    {
        snake_[i].x = WIDTH / 2 - i;
        snake_[i].y = HEIGHT / 2;
    }

    PlaceFruit();
}

void DrawBoard()
{
    printf("\033[2J\033[1;1H");
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            int snake_part = 0;
            for (int i = 0; i < snake_length_; i++)
            {
                if (snake_[i].x == x && snake_[i].y == y)
                {
                    snake_part = 1;
                    break;
                }
            }

            if (x == 0 || y == 0 || x == WIDTH - 1 || y == HEIGHT - 1) printf("■ ");
            else if (snake_part)
            {
	        if(game_over_ == true) printf("\33[31m▣ \33[0m");
	        else printf("▣ ");
            }
            else if (fruit_.x == x && fruit_.y == y) printf("▨ ");
            else printf("□ ");
        }
        printf("\n");
    }
}

void UpdateSnake(int dx, int dy)
{
    point next = { snake_[0].x + dx, snake_[0].y + dy };

    if (next.x <= 0 || next.x >= WIDTH - 1 || next.y <= 0 || next.y >= HEIGHT - 1)
    {
	game_over_ = true;
        return;
    }

    for (int i = 1; i < snake_length_; i++)
    {
        if (next.x == snake_[i].x && next.y == snake_[i].y)
        {
	    game_over_ = true;
            return;
        }
    }

    for (int i = snake_length_ - 1; i > 0; i--)
    {
        snake_[i] = snake_[i - 1];
    }

    snake_[0] = next;

    if (snake_[0].x == fruit_.x && snake_[0].y == fruit_.y)
    {
        if (snake_length_ < MAX_SNAKE_SIZE)
        {
            snake_[snake_length_].x = snake_[snake_length_ - 1].x - dx;
            snake_[snake_length_].y = snake_[snake_length_ - 1].y - dy;
            snake_length_++;
        }

        PlaceFruit();
    }
}

int main()
{
    srand(time(NULL));
    init_snake();

    int dx = 1;
    int dy = 0;

    int prev_dx = dx;
    int prev_dy = dy;

    int delay = 200000;

    HideEcho(true);
    printf("\033[?25l");
    signal(SIGINT, HandleSignal);

    while (game_over_ != true && !flag_)
    {
        DrawBoard();
        char input = ReturnInputKey();
        usleep(delay);

        switch (input)
        {
            case 'w':
	    case 'W':
                if (prev_dy != 1)
                {
                    dx = 0;
                    dy = -1;
                }
                break;
            case 'a':
	    case 'A':
                if (prev_dx != 1)
                {
                    dx = -1;
                    dy = 0;
                }
                break;
            case 's':
	    case 'S':
                if (prev_dy != -1)
                {
                    dx = 0;
                    dy = 1;
                }
                break;
            case 'd':
	    case 'D':
                if (prev_dx != -1)
                {
                    dx = 1;
                    dy = 0;
                }
                break;
            default:
                break;
        }

        prev_dx = dx;
        prev_dy = dy;
        UpdateSnake(dx, dy);

        delay = delay - (snake_length_ - 5) * 200;
        if (delay < 100000) delay = 100000;
	if (flag_) break;
    }
    DrawBoard();
    HideEcho(false);
    printf("\033[?25h\n");

    return 0;
}
