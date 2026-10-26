#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <time.h>

int textmode(int);
int getch();

int textmode(int mode) {
    static struct termios con[2];
    if (mode > 0)
        return tcsetattr(0, TCSAFLUSH, &con[1]);
    tcgetattr(0, &con[0]);
    tcgetattr(0, &con[1]);
    con[0].c_lflag &= ~(ICANON | ECHO | ISIG);
    con[0].c_iflag &= ~(ISTRIP | IXOFF | IXANY | IXON);
    con[0].c_oflag |= CS8;
    con[0].c_cc[VMIN] = 1;
    con[0].c_cc[VTIME] = 0;
    return tcsetattr(0, TCSAFLUSH, &con[0]);
}

int getch() {
    unsigned char c = 0;
    if (read(0, &c, 1) != 1)
        return -1;
    return c;
}

int main() {
    int a, b;
    char a_buf[32];
    char *ans_str = (char *)sbrk(32);
    int err_count = 0;

    write(1, "Enter first operand: ", 21);
    int n = read(0, a_buf, 31);
    if (n <= 0) exit(1);
    a_buf[n - 1] = '\0';
    a = atoi(a_buf);

    srand(time(NULL));
    b = rand() % 100;

    int correct_ans = a - b;
    sprintf(ans_str, "%d", correct_ans);

    char problem[64];
    int p_len = sprintf(problem, "%d - %d = ", a, b);
    write(1, problem, p_len);

    textmode(0);
    time_t start_time = time(NULL);

    int len = strlen(ans_str);
    int i = 0;
    int c;

    while (i < len) {
        c = getch();
        if (c == -1) break;

        if (c == 27) {
            write(1, "\n[ESC pressed. Exiting.]\n", 25);
            break;
        }

        if (c == ans_str[i]) {
            write(1, &c, 1);
            i++;
        } else {
            err_count++;
        }
    }

    time_t end_time = time(NULL);
    textmode(1);
    write(1, "\n", 1);

    char diag[256];
    int d_len = sprintf(diag, "\n--- Diagnostics ---\nTime spent: %ld sec\nErrors: %d\n", 
                        (long)(end_time - start_time), err_count);
    write(2, diag, d_len);

    exit(0);
}