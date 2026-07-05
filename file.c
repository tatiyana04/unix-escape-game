#include "escape.h"

void write_state_to_file(int fd, int gameState, int **map, int rows, int cols)
{
    int i, j;
    char buffer[32];

    lseek(fd, 0, SEEK_SET);//file pointer to the start of the file
    ftruncate(fd, 0);//delete everything in current state

    //game state(0,1,2,3,4)
    int_to_string(gameState, buffer);
    write(fd, buffer, string_length(buffer));
    write(fd, "\n", 1);

    //no of rows - line 2
    int_to_string(rows, buffer);
    write(fd, buffer, string_length(buffer));
    write(fd, " ", 1);

    //no of col - line 2
    int_to_string(cols, buffer);
    write(fd, buffer, string_length(buffer));
    write(fd, "\n", 1);

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            //convert map value and write
            int_to_string(map[i][j], buffer);
            write(fd, buffer, string_length(buffer));
            //space between numbers in each col
            if (j < cols - 1) {
                write(fd, " ", 1);
            }
        }
        write(fd, "\n", 1);
    }
}

int read_state_from_file(int fd, int *gameState, int **map, int rows, int cols)
{
    int bytesRead;
    int index = 0;
    int fileRows;
    int fileCols;
    int i, j;
    char buffer[4096];

    lseek(fd, 0, SEEK_SET);

    bytesRead = read(fd, buffer, sizeof(buffer));
    if (bytesRead < 0) {
        return -1;
    }

    *gameState = read_next_int(buffer, bytesRead, &index);
    fileRows = read_next_int(buffer, bytesRead, &index);
    fileCols = read_next_int(buffer, bytesRead, &index);

    if (fileRows != rows || fileCols != cols) {
        return -1;
    }

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            map[i][j] = read_next_int(buffer, bytesRead, &index);
        }
    }

    return 0;
}