# Escape Game (Unix Systems Programming)

## Terminal Escape Game

A terminal-based escape game written in C using raw Unix system calls instead of the standard C library. Built for a Unix Systems Programming course, this project focuses on low-level file I/O, multi-process architecture, and inter-process synchronization.

## Overview

The player navigates a 2D ASCII map to reach a goal while avoiding two independently-moving enemies, a Snake and a Wolf. The map and live game state live in a shared text file, and every process reads and writes to it only through system calls — no `fopen`, `printf`/`scanf`, or `string.h` functions anywhere in the codebase.

## Key technical features

- **Multi-process architecture** — `fork()` spawns a separate child process for the Snake and another for the Wolf, each moving independently of the player and of each other.
- **Process synchronization** — `flock()` gives exclusive access to a shared game-state file so concurrent reads/writes from the player, snake, and wolf processes never corrupt each other's view of the game.
- **System-call-only I/O** — all file and data handling goes through `open()`, `read()`, `write()`, `close()`. Integer parsing and string handling are hand-implemented since `<string.h>`, `<math.h>`, and the `printf`/`scanf` family are off-limits.
- **Raw terminal rendering** — ANSI escape codes clear and redraw the screen each frame; `tcgetattr()`/`tcsetattr()` put the terminal in raw mode so a single keypress registers instantly, with no need to press Enter.
- **Timed enemy behavior** — `nanosleep()` drives independent movement timers (Snake every 2s, Wolf every 1s). Movement is randomized via `rand()`/`srand()`, but switches to direct pursuit once an enemy is one tile from the player.
- **Clean process lifecycle** — the parent process calls `wait()` to reap both children on win/loss, with all memory (`malloc`'d map array) and file descriptors freed before exit.

## Technologies

- C
- GCC
- Linux
- POSIX system calls
- Makefile

## Concepts demonstrated

- Process creation (`fork()`)
- File descriptors
- File locking (`flock()`)
- Terminal control
- Low-level file I/O
- Memory management

## Running

```
make
./escape-game
```

<img width="544" height="479" alt="Screenshot 2026-07-05 at 7 24 21 PM" src="https://github.com/user-attachments/assets/b0c607c0-fa40-4f3c-a63c-06a00cc01edb" />

## What I learned

This project strengthened my understanding of Unix process management, synchronization, and low-level operating system concepts by implementing a multi-process terminal application.
