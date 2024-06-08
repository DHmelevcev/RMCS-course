#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define MAX_STRINGS 1024
#define MAX_IN_STRING 256

#define MSG_type 1

#define K 20

struct String
{
    int id;
    char chars[MAX_IN_STRING];
};

struct DB
{
    struct String* strings[MAX_STRINGS];
};

struct MSG
{
    long mtype;
    char mtext[sizeof(struct String)];
};

int main(const int argc, const char* argv[])
{
    if (argc > 1) {
        fprintf(stderr, "Too many arguments in instruction\n");
        return 1;
    }

    // создаём новую очередь сообщений с правами доступа 600
    int msgqid = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    if (msgqid == -1) {
        perror("msgget");
        return 2;
    }

    // порождаем K процессов
    for (int strid = 0; strid < K; ++strid) {
        if (!(fork())) {
            const char text[] = "New message ";
            const int text_size = sizeof(text);

            // инициализируем новое сообщение
            struct MSG msg;
            msg.mtype = MSG_type;

            struct String str;
            str.id = strid + 10;
            strcpy(str.chars, text);
            sprintf(&str.chars[text_size - 1], "%d", strid + 10);
            memcpy(msg.mtext, &str, sizeof(int) + sizeof(str.chars));

            // отправляем его
            if (msgsnd(msgqid, (void*)&msg, sizeof(long) + sizeof(int) + text_size, 0) == -1) {
                perror("msgsnd");
            }

            exit(0);
        }
    }

    // создаем и инициализируем новую СУБД
    struct DB db;
    for (int i = 0; i < MAX_STRINGS; ++i) {
        const char text[] = "Old message ";
        const int text_size = sizeof(text);

        struct String* str = NULL;
        while ((str = malloc(sizeof(struct String))) == NULL) {}
        str->id = i;
        strcpy(str->chars, text);
        sprintf(&str->chars[text_size - 1], "%d", i);

        db.strings[i] = str;
    }

    // принимаем K сообщений
    struct MSG msg;
    for (int i = 0; i < K; ++i) {
        if (msgrcv(msgqid, &msg, sizeof(struct MSG), MSG_type, 0) == -1) {
            perror("msgrcv");
        }

        // копируем старую строку в буффер и заменем её на полученную
        char buffer[MAX_IN_STRING];
        strcpy(buffer, db.strings[msg.mtext[0]]->chars);
        strcpy(db.strings[msg.mtext[0]]->chars, &msg.mtext[sizeof(int)]);

        printf("%s -> ", buffer);
        printf("%s\n", db.strings[msg.mtext[0]]->chars);
    }

    // удаляем очередь
    if (msgctl(msgqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        return 3;
    }

    return 0;
}