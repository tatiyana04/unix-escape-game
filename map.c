#include "escape.h"

int string_length(const char *str)
{
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

void int_to_string(int num, char *str)
{
    int i = 0, j;
    char temp;

    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    while (num > 0) {
        str[i++] = (num % 10) + '0';
        num /= 10;
    }

    str[i] = '\0';

    for (j = 0; j < i / 2; j++) {
        temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
}

void skip_spaces_and_newlines(char *buffer, int bytesRead, int *index)
{
    while (*index < bytesRead &&
           (buffer[*index] == ' ' || buffer[*index] == '\n')) {
        (*index)++;
    }
}

int read_next_int(char *buffer, int bytesRead, int *index)
{
    int value = 0;

    skip_spaces_and_newlines(buffer, bytesRead, index);

    while (*index < bytesRead &&
           buffer[*index] >= '0' &&
           buffer[*index] <= '9') {
        value = value * 10 + (buffer[*index] - '0');
        (*index)++;
    }

    return value;
}

void print_int(int value)
{
    char numberString[20];
    int_to_string(value, numberString);
    write(1, numberString, string_length(numberString));
}

char map_value_to_char(int value)
{
    if (value == 0) return ' ';
    if (value == 1) return ' ';
    if (value == 2) return 'P';
    if (value == 3) return 'G';
    if (value == 4) return '~';
    if (value == 5) return 'W';
    return '?';
}

void print_cell(int value)
{
    char ch;

    if (value == 1) {
        //[47m set background color to white
        //[0m reset all formatting back to normal
        write(1, "\033[47m \033[0m", 10);
    } else {
        ch = map_value_to_char(value);
        write(1, &ch, 1);
    }
}

void clear_screen(void)
{
    //\033[H moves the cursor to the top left corner of the terminal
    //\033[J clears everything from the cursor to the end of the screen
    write(1, "\033[H\033[J", 6);
}

void print_map_with_border(int **map, int rows, int cols)
{
    int i, j;
    char star = '*';

    //print top row of stars
    for (j = 0; j < cols + 2; j++) write(1, &star, 1);
    write(1, "\n", 1);

    for (i = 0; i < rows; i++) {
        write(1, &star, 1); //left star

        for (j = 0; j < cols; j++) {
            print_cell(map[i][j]);//print map cells
        }

        write(1, &star, 1);//right star
        write(1, "\n", 1);
    }

    //bottom row of stars
    for (j = 0; j < cols + 2; j++) write(1, &star, 1);
    write(1, "\n", 1);
}

void find_player_position(int **map, int rows, int cols, int *playerRow, int *playerCol)
{
    int i, j;

    //change playerrow and col to -1 through pointer
    *playerRow = -1;
    *playerCol = -1;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            if (map[i][j] == 2) {
                *playerRow = i;
                *playerCol = j;
                return;
            }
        }
    }
}

void find_object_position(int **map, int rows, int cols, int objectValue, int *objectRow, int *objectCol)
{
    int i, j;

    *objectRow = -1;
    *objectCol = -1;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            if (map[i][j] == objectValue) {
                *objectRow = i;
                *objectCol = j;
                return;
            }
        }
    }
}

void set_terminal_raw(struct termios *oldSettings)
{
    struct termios newSettings;

    tcgetattr(0, oldSettings);//Save the current terminal settings
    newSettings = *oldSettings;

    newSettings.c_lflag &= ~ICANON;//no need to press enter
    newSettings.c_lflag &= ~ECHO;//dont show on terminal

    //dont wait forever for an input & dont keep running fast
    newSettings.c_cc[VMIN] = 0;
    newSettings.c_cc[VTIME] = 1;//wait 0.1 sec then continue

    //apply terminal settings to stdin rn
    tcsetattr(0, TCSANOW, &newSettings);
}

void restore_terminal(struct termios *oldSettings)
{
    tcsetattr(0, TCSANOW, oldSettings);
}

void handle_exit(struct termios *oldSettings)
{
    restore_terminal(oldSettings);
}