#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 4096

/* 
 * Единая таблица транслитерации для UTF-8.
 * Индексы 0-31:  Заглавные А-Я (байты 0xD0 0x90 - 0xD0 0xAF)
 * Индексы 32-47: Строчные а-п (байты 0xD0 0xB0 - 0xD0 0xBF)
 * Индексы 48-63: Строчные р-я (байты 0xD1 0x80 - 0xD1 0x8F)
 */
const char* map[64] = {
    // Верхний регистр (А-Я)
    "A", "B", "V", "G", "D", "E", "Zh", "Z", "I", "Y", "K", "L", "M", "N", "O", "P",
    "R", "S", "T", "U", "F", "Kh", "Ts", "Ch", "Sh", "Shch", "", "Y", "", "E", "Yu", "Ya",
    // Нижний регистр (а-п)
    "a", "b", "v", "g", "d", "e", "zh", "z", "i", "y", "k", "l", "m", "n", "o", "p",
    // Нижний регистр (р-я)
    "r", "s", "t", "u", "f", "kh", "ts", "ch", "sh", "shch", "", "y", "", "e", "yu", "ya"
};

int main() {
    unsigned char in_buf[BUF_SIZE];
    // Буфер вывода с запасом (x4), так как "Щ" -> "Shch"
    char out_buf[BUF_SIZE * 4]; 
    ssize_t bytes_read;

    // Чтение с клавиатуры (stdin, fd=0)
    while ((bytes_read = read(0, in_buf, sizeof(in_buf))) > 0) {
        size_t out_idx = 0;
        int i = 0;
        
        while (i < bytes_read) {
            unsigned char c = in_buf[i];
            
            // Если ASCII (латиница, цифры, знаки препинания) — копируем как есть
            if (c < 128) {
                out_buf[out_idx++] = c;
                i++;
            } 
            // Если начало 2-байтового символа UTF-8 кириллицы
            else if (c == 0xD0 || c == 0xD1) {
                if (i + 1 < bytes_read) {
                    unsigned char c2 = in_buf[i + 1];
                    const char* replacement = NULL;
                    
                    // 1. Обработка букв Ё и ё (они выбиваются из общего диапазона)
                    if (c == 0xD0 && c2 == 0x81) {
                        replacement = "Yo";
                    } else if (c == 0xD1 && c2 == 0x91) {
                        replacement = "yo";
                    }
                    // 2. Заглавные А-Я (0xD0 0x90 - 0xD0 0xAF)
                    else if (c == 0xD0 && c2 >= 0x90 && c2 <= 0xAF) { 
                        replacement = map[c2 - 0x90]; 
                    }
                    // 3. Строчные а-п (0xD0 0xB0 - 0xD0 0xBF)
                    else if (c == 0xD0 && c2 >= 0xB0 && c2 <= 0xBF) { 
                        replacement = map[32 + (c2 - 0xB0)]; 
                    }
                    // 4. Строчные р-я (0xD1 0x80 - 0xD1 0x8F)
                    else if (c == 0xD1 && c2 >= 0x80 && c2 <= 0x8F) { 
                        replacement = map[48 + (c2 - 0x80)]; 
                    }
                    
                    // Если символ распознан, подставляем транслитерацию
                    if (replacement != NULL) {
                        size_t len = strlen(replacement);
                        if (out_idx + len < sizeof(out_buf)) {
                            memcpy(&out_buf[out_idx], replacement, len);
                            out_idx += len;
                        }
                        i += 2; // Пропускаем 2 байта UTF-8
                        continue;
                    }
                }
                // Если не распознали (например, это украинская или сербская буква), просто копируем байт
                out_buf[out_idx++] = c;
                i++;
            } 
            else {
                // Прочие байты (например, знаки препинания UTF-8) копируем как есть
                out_buf[out_idx++] = c;
                i++;
            }
        }
        
        // Вывод на экран (stdout, fd=1)
        if (out_idx > 0) {
            write(1, out_buf, out_idx);
        }
    }
    
    return 0;
}