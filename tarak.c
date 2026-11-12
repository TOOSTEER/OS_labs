#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

void clrscr() {
    write(1, "\033[H\033[2J", 7);
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
    
    // Переключаемся на альтернативный экран (чтобы не засорять терминал)
    write(1, "\033[?1049h", 8);
    clrscr();
    // Скрываем курсор
    write(1, "\033[?25l", 6);
    
    // Рисуем исходные строки
    for (j = 0; j < PROCNUM; j++) {
        char cmd[128];
        int clen = snprintf(cmd, sizeof(cmd), "\033[%d;1H%s", j + 1, str);
        write(1, cmd, clen);
    }
    
    usleep(500000);
    
    for (j = 0; j < PROCNUM; j++) {
        if ((pid[j] = fork()) == 0) {
            usleep(PROCNUM - j);
            
            // Инициализируем генератор случайных чисел один раз
            srand(getpid() + j);
            
            for (int x = 1; x <= len; x++) {
                char cmd[64];
                // Объединяем перемещение курсора и печать пробела в ОДНУ строку
                int clen = snprintf(cmd, sizeof(cmd), "\033[%d;%dH ", j + 1, x);
                write(1, cmd, clen);
                
                // Фиктивный цикл для создания задержки
                int delay = rand() % 5000000 + 1000000; 
                for (volatile int k = 0; k < delay; k++);
            }
            
            // Небольшая задержка, чтобы терминал успел отрисовать последний пробел
            usleep(50000); 
            exit('A' + j);
        }
    }
    
    int status;
    int p;
    int row = PROCNUM + 2; 
    
    // Ожидание завершения всех потомков
    while ((p = wait(&status)) != -1) {
        if (WIFEXITED(status)) {
            char msg[64];
            // Принудительно ставим курсор на новую строку для вывода статуса
            int l = snprintf(msg, sizeof(msg), "\033[%d;1HProcess %c finished", row++, WEXITSTATUS(status));
            write(2, msg, l);
        }
    }
    
    // Возвращаем курсор
    write(1, "\033[?25h", 6); 
    
    // Ждем нажатия Enter, чтобы пользователь увидел результат
    write(2, "\nPress Enter to exit...", 23);
    char buf[1];
    read(0, buf, 1); 
    
    // Возвращаемся в основной экран терминала
    write(1, "\033[?1049l", 8); 
    return 0;
}