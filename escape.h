#ifndef ESCAPE_H
#define ESCAPE_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include <termios.h>
#include <stdlib.h>

int string_length(const char *str);
void int_to_string(int num, char *str);
void skip_spaces_and_newlines(char *buffer, int bytesRead, int *index);
int read_next_int(char *buffer, int bytesRead, int *index);
void print_int(int value);
char map_value_to_char(int value);
void print_cell(int value);
void clear_screen(void);
void print_map_with_border(int **map, int rows, int cols);
void find_player_position(int **map, int rows, int cols, int *playerRow, int *playerCol);
void find_object_position(int **map, int rows, int cols, int objectValue, int *objectRow, int *objectCol);
int is_valid_player_move(int **map, int rows, int cols, int newRow, int newCol);
int is_valid_enemy_move(int **map, int rows, int cols, int newRow, int newCol);
int move_player(int **map, int rows, int cols, int *playerRow, int *playerCol, char command);
int move_wolf_once(int **map, int rows, int cols);
int move_snake_once(int **map, int rows, int cols);
void set_terminal_raw(struct termios *oldSettings);
void restore_terminal(struct termios *oldSettings);
void handle_exit(struct termios *oldSettings);
void write_state_to_file(int fd, int gameState, int **map, int rows, int cols);
int read_state_from_file(int fd, int *gameState, int **map, int rows, int cols);

#endif