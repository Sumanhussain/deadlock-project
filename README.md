# Multiplayer Resource Management and Deadlock Recovery System

![CI](https://github.com/Sumanhussain/deadlock-project/actions/workflows/ci.yml/badge.svg)

A C-based multiplayer resource management simulation demonstrating Operating System concepts and DevOps practices.

## OS Concepts Demonstrated
- **Process creation**: `fork()` creates one child process per game
- **Multithreading**: one `pthread` per player inside each game
- **IPC**: `pipe` sends game results from child to parent
- **Synchronization**: mutex + condition variable for turn-based access
- **Resource allocation**: players request shared resources each round
- **Deadlock detection and recovery**: if resources are insufficient, the player is penalized and all resources are reset (simulation)

## Project Structure
    src/game.c            main C program
    scripts/run_game.sh   runs the game with monitoring and logging
    scripts/monitor.sh    process/thread/CPU/memory monitoring
    scripts/collect_logs.sh  log summary
    tests/test.sh         automated test
    Makefile              build automation
    Dockerfile            containerized run
    .github/workflows/ci.yml  GitHub Actions CI

## Build and Run
    make            # build
    make run        # run interactively
    make test       # automated test
    make monitor    # run with monitoring and log collection
    make clean      # remove build files and logs

## DevOps Tools
- **Bash**: system/process monitoring and log collection
- **Makefile**: automated compilation
- **Git and GitHub**: version control
- **GitHub Actions**: builds and tests on every push
- **Docker**: consistent runtime environment

## Docker
    docker build -t deadlock-game .
    docker run -it deadlock-game
