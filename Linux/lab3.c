#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <memory.h>

struct p_result
{
    int pid;
    char values[40];
};

int main(const int argc, const char* argv[])
{
    if (argc > 1) {
        fprintf(stderr, "Too many arguments in instruction\n");
        return 1;
    }

    printf("Process 0 started\n");

    int pid0;

    int k0[2];
    int k1[2];
    int k2[2];
    
    // открываем каналы
    pipe(k0);
    pipe(k1);
    pipe(k2);

    struct p_result result0;

    // p1
    if (!(fork())) {
        // закрываем ненужные дескрипторы
        close(k0[0]);
        close(k1[0]);

        // вызываем исполнительный файл lab31 и передаем ему дескрипторы
        char arg_k01[10];
        sprintf(arg_k01, "%d", k0[1]);
        char arg_k11[10];
        sprintf(arg_k11, "%d", k1[1]);
        char arg_k20[10];
        sprintf(arg_k20, "%d", k2[0]);
        char arg_k21[10];
        sprintf(arg_k21, "%d", k2[1]);
        execl("lab31", arg_k01, arg_k11, arg_k20, arg_k21, NULL);
        exit(0);
    }
    // p2
    if (!(fork())) {
        // читаем из k0
        // пишем в k2
        close(k0[1]);
        close(k1[0]);
        close(k1[1]);
        close(k2[0]);

        // вызываем исполнительный файл lab32 и передаем ему дескрипторы
        char arg_k00[10];
        sprintf(arg_k00, "%d", k0[0]);
        char arg_k21[10];
        sprintf(arg_k21, "%d", k2[1]);
        execl("lab32", arg_k00, arg_k21, NULL);
        exit(0);
    }

    // читаем из k1
    read(k1[0], &result0, sizeof(result0));
    printf("Values 1:");
    for (int i = 0; i < sizeof(result0.values); ++i) printf(" %c", result0.values[i]);

    read(k1[0], &result0, sizeof(result0));
    printf("\nValues 2:");
    for (int i = 0; i < sizeof(result0.values); ++i) printf(" %c", result0.values[i]);

    read(k1[0], &result0, sizeof(result0));
    printf("\nValues 3:");
    for (int i = 0; i < sizeof(result0.values); ++i) printf(" %c", result0.values[i]);

    printf("\nProcess 0 readed k1\n");

    pid0 = wait(NULL);
    printf("Process 0 waited for process: %d\n", pid0);
    pid0 = wait(NULL);
    printf("Process 0 waited for process: %d\n", pid0);

    printf("Process 0 ended\n");
    return 0;
}