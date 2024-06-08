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
    if (argc != 4) {
        fprintf(stderr, "Bad arguments in instruction\n");
        return 1;
    }

    int pid0;
    struct p_result result0;
    int k01 = atoi(argv[0]), k11 = atoi(argv[1]), k20 = atoi(argv[2]), k21 = atoi(argv[3]);
    printf("Process 1 started: %d\n", result0.pid = getpid());
    memset(result0.values, '1', sizeof(result0.values));

    // p3
    if (!(pid0 = fork())) {
        // пишем в k1
        // читаем из k2
        close(k01);
        close(k21);

        // вызываем исполнительный файл lab33 и передаем ему дескрипторы и id родительского процесса
        char arg_k1[10];
        sprintf(arg_k1, "%d", k11);
        char arg_k2[10];
        sprintf(arg_k2, "%d", k20);
        char arg_pid[10];
        sprintf(arg_pid, "%d", result0.pid);
        execl("lab33", arg_k1, arg_k2, arg_pid, NULL);

        exit(0);
    }

    // пишем в k0
    // пишем в k2
    close(k11);
    close(k20);

    printf("Process 1 stopped\n");
    raise(SIGSTOP); // or kill(result0.pid, SIGSTOP);
    printf("Process 1 resumed\n");

    write(k21, &result0, sizeof(result0));
    printf("Process 1 wrote k2\n");

    write(k01, &pid0, sizeof(pid0));
    printf("Process 1 wrote k0\n");

    pid0 = wait(NULL);
    printf("Process 1 waited for process: %d\n", pid0);

    printf("Process 1 ended\n");
    return 0;
}