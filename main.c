#include "escape.h"

int main(int argc, char *argv[])
{
    const char usage[]       = "Usage: ./escape <filename>\n";
    const char openError[]   = "Error: Cannot open file\n";
    const char readError[]   = "Error: Cannot read file\n";
    const char memoryError[] = "Error: Memory allocation failed\n";
    const char stateError[]  = "Error: Cannot open state file\n";
    const char stateReadError[] = "Error: Cannot read state file\n";

    char buffer[4096];
    char command;

    int bytesRead, fd, index = 0;
    int gameState, rows, cols;
    int i, j;
    int playerRow = 0, playerCol = 0;
    int result;
    int lockFd;

    int **map;
    struct termios oldSettings;
    pid_t wolf_pid, snake_pid;

    srand((unsigned int)time(NULL));

    //too many or too few command line arguments
    if (argc != 2) {
        write(1, usage, sizeof(usage) - 1);
        return 1;
    }

    //initial open error handling when opening the map file
    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        write(1, openError, sizeof(openError) - 1);
        return 1;
    }

    //read file into buffer
    bytesRead = read(fd, buffer, sizeof(buffer));
    if (bytesRead < 0) {
        write(1, readError, sizeof(readError) - 1);
        close(fd);
        return 1;
    }

    close(fd);

    //parsing the header
    gameState = read_next_int(buffer, bytesRead, &index);
    rows = read_next_int(buffer, bytesRead, &index);
    cols = read_next_int(buffer, bytesRead, &index);

    //max map size 20x20
    if (rows > 20 || cols > 20 || rows <= 0 || cols <= 0) {
        write(1, "Error: Invalid map size\n", 24);
        return 1;
    }

    //allocating 2d array
    map = malloc(rows * sizeof(int *));
    if (!map) {
        write(1, memoryError, sizeof(memoryError) - 1);
        return 1;
    }

    //null chekc on malloc
    for (i = 0; i < rows; i++) {
        map[i] = malloc(cols * sizeof(int));
        if (!map[i]) {
            write(1, memoryError, sizeof(memoryError) - 1);
            for (j = 0; j < i; j++) free(map[j]);
            free(map);
            return 1;
        }
    }

    //fill the array
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            map[i][j] = read_next_int(buffer, bytesRead, &index);
        }
    }

    find_player_position(map, rows, cols, &playerRow, &playerCol);

    lockFd = open("state.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (lockFd < 0) {
        write(1, stateError, sizeof(stateError) - 1);
        for (i = 0; i < rows; i++) free(map[i]);
        free(map);
        return 1;
    }

    write_state_to_file(lockFd, gameState, map, rows, cols);
    close(lockFd);

    //child process using fork
    wolf_pid = fork();

    if (wolf_pid < 0) {
        write(1, "Error: fork failed\n", 19);
        for (i = 0; i < rows; i++) free(map[i]);
        free(map);
        return 1;
    }

    if (wolf_pid == 0) {
        srand((unsigned int)(time(NULL) ^ getpid()));

        //wolf move every 1 sec
        struct timespec delay;
        delay.tv_sec  = 1;
        delay.tv_nsec = 0;

        while (1) {
            nanosleep(&delay, NULL);

            //open state.txt for reading and writing
            lockFd = open("state.txt", O_RDWR);
            if (lockFd < 0) {
                break;
            }

            //file locked, only wolf can use
            flock(lockFd, LOCK_EX);

            //read latest game state/ if fail
            if (read_state_from_file(lockFd, &gameState, map, rows, cols) < 0) {
                flock(lockFd, LOCK_UN);
                close(lockFd);
                break;
            }

            //child process stops when running, win, lose, quit
            if (gameState != 0) {
                //unlock file
                flock(lockFd, LOCK_UN);
                close(lockFd);
                break;
            }

            //lose- wolf
            result = move_wolf_once(map, rows, cols);
            if (result == -1) {
                gameState = 2;
            }

            write_state_to_file(lockFd, gameState, map, rows, cols);
            //unlock file
            flock(lockFd, LOCK_UN);
            close(lockFd);//close file
        }

        for (i = 0; i < rows; i++) free(map[i]);
        free(map);
        return 0;
    }

    //child process using fork
    snake_pid = fork();

    if (snake_pid < 0) {
        write(1, "Error: fork failed\n", 19);
        for (i = 0; i < rows; i++) free(map[i]);
        free(map);
        return 1;
    }

    if (snake_pid == 0) {
        srand((unsigned int)(time(NULL) ^ getpid()));

        //snake moves every 2 sec
        struct timespec delay;
        delay.tv_sec  = 2;
        delay.tv_nsec = 0;

        while (1) {
            nanosleep(&delay, NULL);

            lockFd = open("state.txt", O_RDWR);
            if (lockFd < 0) {
                break;
            }

            flock(lockFd, LOCK_EX);

            if (read_state_from_file(lockFd, &gameState, map, rows, cols) < 0) {
                flock(lockFd, LOCK_UN);
                close(lockFd);
                break;
            }
            ///child process stops when running, win, lose, quit
            if (gameState != 0) {
                flock(lockFd, LOCK_UN);
                close(lockFd);
                break;
            }

            //lose -snake
            result = move_snake_once(map, rows, cols);
            if (result == -1) {
                gameState = 3;
            }

            write_state_to_file(lockFd, gameState, map, rows, cols);
            flock(lockFd, LOCK_UN);
            close(lockFd);
        }

        for (i = 0; i < rows; i++) free(map[i]);
        free(map);
        return 0;
    }

    set_terminal_raw(&oldSettings);

    struct timespec delay;
    delay.tv_sec  = 0;
    delay.tv_nsec = 150000000;

    while (1) {
        lockFd = open("state.txt", O_RDWR);
        //error handling
        if (lockFd < 0) {
            write(1, stateError, sizeof(stateError) - 1);
            break;
        }

        //lock file
        flock(lockFd, LOCK_EX);

        if (read_state_from_file(lockFd, &gameState, map, rows, cols) < 0) {
            write(1, stateReadError, sizeof(stateReadError) - 1);
            flock(lockFd, LOCK_UN);
            close(lockFd);
            break;
        }

        //player won
        if (gameState == 1) {
            flock(lockFd, LOCK_UN);//unlock
            close(lockFd);//close
            clear_screen();
            print_map_with_border(map, rows, cols);
            write(1, "Press w to move UP\n",    19);
            write(1, "Press s to move DOWN\n",  21);
            write(1, "Press a to move LEFT\n",  21);
            write(1, "Press d to move RIGHT\n", 22);
            write(1, "\nYou win!\n", 10);
            //wait for child processes to finish
            wait(NULL);
            wait(NULL);
            break;
        }

        //player lost to wolf
        if (gameState == 2) {
            flock(lockFd, LOCK_UN);
            close(lockFd);
            clear_screen();
            print_map_with_border(map, rows, cols);
            write(1, "Press w to move UP\n",    19);
            write(1, "Press s to move DOWN\n",  21);
            write(1, "Press a to move LEFT\n",  21);
            write(1, "Press d to move RIGHT\n", 22);
            write(1, "\nYou lose! The wolf ate you!\n", 29);
            wait(NULL);
            wait(NULL);
            break;
        }

        //player lost to snake
        if (gameState == 3) {
            flock(lockFd, LOCK_UN);
            close(lockFd);
            clear_screen();
            print_map_with_border(map, rows, cols);
            write(1, "Press w to move UP\n",    19);
            write(1, "Press s to move DOWN\n",  21);
            write(1, "Press a to move LEFT\n",  21);
            write(1, "Press d to move RIGHT\n", 22);
            write(1, "\nYou lose! The snake bit you!\n", 30);
            wait(NULL);
            wait(NULL);
            break;
        }

        find_player_position(map, rows, cols, &playerRow, &playerCol);

        //redraw the map on every loop
        clear_screen();
        print_map_with_border(map, rows, cols);

        write(1, "Press w to move UP\n",    19);
        write(1, "Press s to move DOWN\n",  21);
        write(1, "Press a to move LEFT\n",  21);
        write(1, "Press d to move RIGHT\n", 22);

        //read one char from keyboard & store char in command variable
        if (read(0, &command, 1) > 0) {
            //ignore enter keys
            if (command == '\n' || command == '\r') {
                flock(lockFd, LOCK_UN);
                close(lockFd);
                continue;
            }

            if (command == 'q') {
                gameState = 4;

                write_state_to_file(lockFd, gameState, map, rows, cols);

                flock(lockFd, LOCK_UN);
                close(lockFd);

                wait(NULL);
                wait(NULL);
                break;
            }

            //return the value in game.c, into result
            result = move_player(map, rows, cols, &playerRow, &playerCol, command);

            //win
            if (result == 1) {
                gameState = 1;
                write_state_to_file(lockFd, gameState, map, rows, cols);//save win to state.txt
                flock(lockFd, LOCK_UN);
                close(lockFd);

                //redraw the map on every loop iteration
                clear_screen();
                print_map_with_border(map, rows, cols);

                write(1, "Press w to move UP\n",    19);
                write(1, "Press s to move DOWN\n",  21);
                write(1, "Press a to move LEFT\n",  21);
                write(1, "Press d to move RIGHT\n", 22);

                write(1, "\nYou win!\n", 10);

                //both child processes
                wait(NULL);
                wait(NULL);
                break;
            }
        }

        write_state_to_file(lockFd, gameState, map, rows, cols);
        flock(lockFd, LOCK_UN);
        close(lockFd);

        nanosleep(&delay, NULL);
    }

    handle_exit(&oldSettings);

    //free map memory to prevent leaks
    for (i = 0; i < rows; i++) {
        free(map[i]);
    }
    free(map);

    return 0;
}