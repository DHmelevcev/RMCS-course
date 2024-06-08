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
    if (argc != 2) {
        fprintf(stderr, "Bad arguments in instruction\n");
        return 1;
    }

    int pid0;
    struct p_result result0;
    int k00 = atoi(argv[0]), k21 = atoi(argv[1]);
    printf("Process 2 started: %d\n", result0.pid = getpid());
    memset(result0.values, '2', sizeof(result0.values));

    read(k00, &pid0, sizeof(pid0));
    printf("Process 2 readed k0\n");

    write(k21, &result0, sizeof(result0));
    printf("Process 2 wrote k2\n");

    kill(pid0, SIGCONT);
    printf("Process 2 sent signal to p3\n");

    printf("Process 2 ended\n");
    return 0;
}