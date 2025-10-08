#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

struct node {
    char *text;
    struct node *next;
};

/* ---------- список ---------- */
static struct node* make_node(const char *input) {
    struct node *n = (struct node*)malloc(sizeof *n);
    if (!n) { perror("malloc node"); exit(1); }
    size_t len = strlen(input);
    n->text = (char*)malloc(len + 1);
    if (!n->text) { perror("malloc text"); exit(1); }
    memcpy(n->text, input, len + 1);
    n->next = NULL;
    return n;
}

/* ---------- отрисовка строки и позиционирование курсора ---------- */
static void redraw_line(const char *buf, size_t len, size_t cur, size_t *prev_print_len) {
    size_t i;
    /* в начало строки */
    write(STDOUT_FILENO, "\r", 1);

    /* напечатать содержимое */
    if (len > 0) write(STDOUT_FILENO, buf, len);

    /* стереть остаток, если раньше было длиннее */
    if (*prev_print_len > len) {
        size_t diff = *prev_print_len - len;
        for (i = 0; i < diff; ++i) write(STDOUT_FILENO, " ", 1);
        for (i = 0; i < diff; ++i) write(STDOUT_FILENO, "\b", 1);
    }
    *prev_print_len = len;

    /* вернуть курсор на позицию cur (сейчас он в конце) */
    if (len > cur) {
        size_t back = len - cur;
        for (i = 0; i < back; ++i) write(STDOUT_FILENO, "\b", 1);
    }
    /* на всякий случай сбросить буфер терминала */
    fsync(STDOUT_FILENO);
}

/* ---------- сырой ввод строки с режимом вставки ---------- */
/* Возвращает строку с '\n' в конце (как fgets). Память через malloc. */
static char* read_line_insert_mode(void) {
    struct termios orig, raw;
    if (tcgetattr(STDIN_FILENO, &orig) == -1) { perror("tcgetattr"); exit(1); }
    raw = orig;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) { perror("tcsetattr"); exit(1); }

    size_t cap = 64, len = 0, cur = 0, prev_print_len = 0;
    char *buf = (char*)malloc(cap);
    if (!buf) { perror("malloc buf"); exit(1); }

    for (;;) {
        unsigned char c;
        if (read(STDIN_FILENO, &c, 1) != 1) c = '\n';

        if (c == '\r' || c == '\n') {                 /* Enter */
            /* перенос курсора в конец и вывод строки + \n (как эхо) */
            if (len > cur) {
                size_t i, back = len - cur;
                for (i = 0; i < back; ++i) write(STDOUT_FILENO, "\b", 1);
            }
            write(STDOUT_FILENO, "\r", 1);
            if (len > 0) write(STDOUT_FILENO, buf, len);
            write(STDOUT_FILENO, "\n", 1);
            break;

        } else if (c == 127 || c == 8) {              /* Backspace */
            if (cur > 0) {
                memmove(&buf[cur-1], &buf[cur], len - cur);
                cur--; len--;
                redraw_line(buf, len, cur, &prev_print_len);
            }

        } else if (c == 27) {                          /* ESC-последовательности */
            unsigned char a, b, d;
            if (read(STDIN_FILENO, &a, 1) != 1) continue;
            if (a != '[') continue;

            if (read(STDIN_FILENO, &b, 1) != 1) continue;

            if (b == 'D') {                            /* ← */
                if (cur > 0) { cur--; redraw_line(buf, len, cur, &prev_print_len); }
            } else if (b == 'C') {                     /* → */
                if (cur < len) { cur++; redraw_line(buf, len, cur, &prev_print_len); }
            } else if (b == 'H') {                     /* Home */
                cur = 0; redraw_line(buf, len, cur, &prev_print_len);
            } else if (b == 'F') {                     /* End */
                cur = len; redraw_line(buf, len, cur, &prev_print_len);
            } else if (b >= '0' && b <= '9') {         /* возможные 1~, 3~, 4~ */
                if (read(STDIN_FILENO, &d, 1) != 1) continue;
                if (d == '~') {
                    if (b == '1') {                    /* Home: ESC [ 1 ~ */
                        cur = 0; redraw_line(buf, len, cur, &prev_print_len);
                    } else if (b == '4') {             /* End: ESC [ 4 ~ */
                        cur = len; redraw_line(buf, len, cur, &prev_print_len);
                    } else if (b == '3') {             /* Delete: ESC [ 3 ~ */
                        if (cur < len) {
                            memmove(&buf[cur], &buf[cur+1], len - cur - 1);
                            len--;
                            redraw_line(buf, len, cur, &prev_print_len);
                        }
                    }
                }
            }
        } else if (c >= 32 && c <= 126) {              /* печатаемые символы — ВСТАВКА */
            if (len + 1 >= cap) {
                cap *= 2;
                char *nb = (char*)realloc(buf, cap);
                if (!nb) { perror("realloc"); free(buf); buf=NULL; break; }
                buf = nb;
            }
            memmove(&buf[cur+1], &buf[cur], len - cur); /* сдвиг правой части */
            buf[cur] = (char)c;
            cur++; len++;
            redraw_line(buf, len, cur, &prev_print_len);
        } else {
            /* игнор прочих кодов */
        }
    }

    /* восстановить терминал */
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig) == -1) { perror("tcsetattr restore"); }

    /* добавить '\n' и '\0' */
    if (len + 2 > cap) {
        char *nb = (char*)realloc(buf, len + 2);
        if (!nb) { perror("realloc end"); free(buf); return NULL; }
        buf = nb;
    }
    buf[len++] = '\n';
    buf[len] = '\0';
    return buf;
}

/* ---------- main ---------- */
int main(void) {
    struct node *head = (struct node*)malloc(sizeof *head);
    if (!head) { perror("malloc head"); return 1; }
    head->text = NULL;
    head->next = NULL;
    struct node *current = head;

    for (;;) {
        char *line = read_line_insert_mode();   /* строка с \n и \0 */
        if (!line) break;
        if (line[0] == '.') {                   /* точка в начале — конец ввода */
            free(line);
            break;
        }
        current->next = make_node(line);
        current = current->next;
        free(line);
    }

    for (struct node *p = head->next; p; p = p->next)
        printf("%s", p->text);

    /* освобождение памяти */
    for (struct node *p = head; p; ) {
        struct node *n = p->next;
        free(p->text);
        free(p);
        p = n;
    }
    return 0;
}
