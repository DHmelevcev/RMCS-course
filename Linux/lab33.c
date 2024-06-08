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
    if (argc != 3) {
        fprintf(stderr, "Bad arguments in instruction\n");
        return 1;
    }

    struct p_result result0;
    int k1 = atoi(argv[0]), k2 = atoi(argv[1]), parent_pid = atoi(argv[2]);
    printf("Process 3 started: %d\n", result0.pid = getpid());
    memset(result0.values, '3', sizeof(result0.values));

    write(k1, &result0, sizeof(result0));
    printf("Process 3 wrote k1\n");

    kill(parent_pid, SIGCONT);
    printf("Process 3 sent signal to p1\n");

    printf("Process 3 stopped\n");
    raise(SIGSTOP); // or kill(result0.pid, SIGSTOP);
    printf("Process 3 resumed\n");

    read(k2, &result0, sizeof(result0));
    write(k1, &result0, sizeof(result0));
    read(k2, &result0, sizeof(result0));
    write(k1, &result0, sizeof(result0));
    printf("Process 3 readed k2 and wrote k1\n");

    printf("Process 3 ended\n");
    return 0;
}