#include "escape.h"

int is_valid_player_move(int **map, int rows, int cols, int newRow, int newCol)
{
    if (newRow < 0 || newRow >= rows) return 0;
    if (newCol < 0 || newCol >= cols) return 0;
    if (map[newRow][newCol] == 4) return 0;//snake
    if (map[newRow][newCol] == 5) return 0;//wolf
    if (map[newRow][newCol] == 1) return 0;//wall

    //if can move
    return 1;
}


int is_valid_enemy_move(int **map, int rows, int cols, int newRow, int newCol)
{
    if (newRow < 0 || newRow >= rows) return 0;
    if (newCol < 0 || newCol >= cols) return 0;

    if (map[newRow][newCol] == 1) return 0;//wall
    if (map[newRow][newCol] == 3) return 0;//goal
    if (map[newRow][newCol] == 4) return 0;//snake(running into each other)
    if (map[newRow][newCol] == 5) return 0;//wolf

    return 1;
}

int move_player(int **map, int rows, int cols, int *playerRow, int *playerCol, char command)
{
    int newRow = *playerRow;
    int newCol = *playerCol;

    //directions
    if (command == 'w') newRow--;
    else if (command == 's') newRow++;
    else if (command == 'a') newCol--;
    else if (command == 'd') newCol++;
    else return 0;

    //win
    if (is_valid_player_move(map, rows, cols, newRow, newCol)) {
        if (map[newRow][newCol] == 3) {
            map[*playerRow][*playerCol] = 0;//old player cell
            map[newRow][newCol] = 2;//player is in the goal
            *playerRow = newRow;
            *playerCol = newCol;
            return 1;
        }

        map[*playerRow][*playerCol] = 0;
        map[newRow][newCol] = 2;
        *playerRow = newRow;
        *playerCol = newCol;
    }

    return 0;
}

int move_wolf_once(int **map, int rows, int cols)
{
    int wolfRow, wolfCol;
    int newRow, newCol;
    int direction;
    int attempts = 0;
    int playerRow, playerCol;

    find_object_position(map, rows, cols, 5, &wolfRow, &wolfCol);

    if (wolfRow == -1 || wolfCol == -1) {
        return 0;
    }

    find_object_position(map, rows, cols, 2, &playerRow, &playerCol);

    if (playerRow != -1) {//in map.c if the player is not found
        //check how far
        int rowDiff = playerRow - wolfRow;
        int colDiff = playerCol - wolfCol;
        //negative into positive
        int absRow = rowDiff < 0 ? -rowDiff : rowDiff;
        int absCol = colDiff < 0 ? -colDiff : colDiff;

        //attack when player is 1 move away, stop diagonal attack
        if (absRow <= 1 && absCol <= 1 && (absRow + absCol) <= 1) {
            map[wolfRow][wolfCol] = 0;
            map[playerRow][playerCol] = 5;
            return -1;
        }
    }

    while (attempts < 10) {
        newRow = wolfRow;
        newCol = wolfCol;

        direction = rand() % 4; //0-3 numbers

        //move wolf 4 directions
        if (direction == 0) newRow--;
        else if (direction == 1) newRow++;
        else if (direction == 2) newCol--;
        else if (direction == 3) newCol++;

        if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols) {
            if (map[newRow][newCol] == 2) {
                map[wolfRow][wolfCol] = 0;
                map[newRow][newCol] = 5;
                return -1;
            }

            if (is_valid_enemy_move(map, rows, cols, newRow, newCol)) {
                map[wolfRow][wolfCol] = 0;
                map[newRow][newCol] = 5;
                return 0;
            }
        }

        attempts++;
    }

    return 0;
}

int move_snake_once(int **map, int rows, int cols)
{
    int snakeRow, snakeCol;
    int newRow, newCol;
    int direction;
    int attempts = 0;
    int playerRow, playerCol;

    //-1 up, 0 same row, 1 down
    int rowMoves[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    //-1 left, 0 same col, 1 right
    int colMoves[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

    find_object_position(map, rows, cols, 4, &snakeRow, &snakeCol);

    if (snakeRow == -1 || snakeCol == -1) {
        return 0;
    }

    find_object_position(map, rows, cols, 2, &playerRow, &playerCol);

    //in map.c if the player is not found
    if (playerRow != -1) {
        int rowDiff = playerRow - snakeRow;
        int colDiff = playerCol - snakeCol;
        int absRow = rowDiff < 0 ? -rowDiff : rowDiff;
        int absCol = colDiff < 0 ? -colDiff : colDiff;

        //diagonal, up, down, sides
        if (absRow <= 1 && absCol <= 1 && (absRow + absCol) >= 1) {
            map[snakeRow][snakeCol] = 0;
            map[playerRow][playerCol] = 4;
            return -1;
        }
    }

    while (attempts < 20) {
        direction = rand() % 8;//0-7 numbers

        newRow = snakeRow + rowMoves[direction];
        newCol = snakeCol + colMoves[direction];

        if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols) {
            if (map[newRow][newCol] == 2) {
                map[snakeRow][snakeCol] = 0;
                map[newRow][newCol] = 4;
                return -1;
            }

            if (is_valid_enemy_move(map, rows, cols, newRow, newCol)) {
                map[snakeRow][snakeCol] = 0;
                map[newRow][newCol] = 4;
                return 0;
            }
        }

        attempts++;
    }

    return 0;
}