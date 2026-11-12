#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

void clrscr() {
    write(1, "\033[H\033[2J", 7);
}

void gotoxy(int x, int y) {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "\033[%d;%dH", y, x);
    write(1, buf, len);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        write(2, "Usage: ./lab4 <num_strings> <string>\n", 37);
        exit(1);
    }
    
    int PROCNUM = atoi(argv[1]);
    char* str = argv[2];
    int len = strlen(str);
    
    int* pid = sbrk((PROCNUM + 1) * sizeof(int));
    int j;
    
    clrscr();
    
    for (j = 0; j < PROCNUM; j++) {
        gotoxy(1, j + 1);
        write(1, str, len);
    }
    
    usleep(500000);
    
    for (j = 0; j < PROCNUM; j++) {
        if ((pid[j] = fork()) == 0) {
            usleep(PROCNUM - j);
            for (int x = 1; x <= len; x++) {
                gotoxy(x, j + 1);
                write(1, " ", 1);
                
                srand(getpid());
                int delay = rand() % 50000000 + 10000000;
                for (volatile int k = 0; k < delay; k++);
            }
            exit('A' + j);
        }
    }
    
    int status;
    int p;
    write(2, "\n--- Exit order ---\n", 21);
    while ((p = wait(&status)) != -1) {
        if (WIFEXITED(status)) {
            char msg[32];
            int l = snprintf(msg, sizeof(msg), "Process %c finished\n", WEXITSTATUS(status));
            write(2, msg, l);
        }
    }
    
    gotoxy(1, PROCNUM + 2);
    return 0;
}