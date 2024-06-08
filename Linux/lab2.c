#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define EPS 1e-10
#define TWOPI 6.28318530718

struct p_result
{
        double value;
        int pid;
};

int main(const int argc, const char* argv[])
{
        if (argc < 2) {
                fprintf(stderr, "Missing argument 'x'\n");
                return 1;
        }
        if (argc > 2) {
                fprintf(stderr,"Too many arguments in instruction\n");
                return 2;
        }
        if (mkfifo("fifo", 0666)) { // создаем именнованный поток
                perror(NULL);
                return 3;
        }

        double x = atof(argv[1]); // кастим массив чаров к даблу
        double res1 = 0.;
        double res2 = 1.;
        struct p_result res; // структура, в которую будем читать результат

        int fifo; // дескриптор именованного потока
        int pid1; // id первого процесса
        int pid2; // id второго процесса

        // форк первого и второго процесса
        if (!(pid1 = fork())) {
                if ((fifo = open("fifo", O_WRONLY)) == -1) { // открываем файл для записи
                        fprintf(stderr,"fifo stolen\n");
                        exit(1);
                }

                while (fabs(res1 - res2) > EPS) {
                        res2 = res1;
                        res1 += 8. / ((4. * pid1 + 1) * (4. * pid1 + 3));
                        ++pid1;
                }

                res.value = res1; res.pid = getpid();
                if (write(fifo, &res, sizeof(res)) == -1) { // пишем в файл структуру
                        fprintf(stderr,"fifo stolen\n");
                        exit(1);
                }
                close(fifo);
                exit(0);
        }
        if (!(pid2 = fork())) {
                if ((fifo = open("fifo", O_WRONLY)) == -1) { // открываем файл для записи
                        fprintf(stderr,"fifo stolen\n");
                        exit(1);
                }

                double x2 = x - TWOPI * (long long)(x / TWOPI);
                x2 *= x2;
                res1 = x2;
                unsigned long long del = 2;
                signed char sign = -1;

                while (pid2 < 20) {
                        pid2 += 2;
                        res2 += (res1 / del) * sign;
                        sign = -sign;
                        res1 *= x2;
                        del *= (pid2 + 1) * (pid2 + 2);
                }

                res.value = res2; res.pid = getpid();
                if (write(fifo, &res, sizeof(res)) == -1) {
                        fprintf(stderr,"fifo stolen\n");
                        exit(1);
                }
                close(fifo);
                exit(0);
        }

        if ((fifo = open("fifo", O_RDONLY)) == -1) { // открываем файл для чтения
                fprintf(stderr,"fifo stolen\n");
                return(4);
        }

        for(int pcnt = 2; pcnt > 0;) { // хотим прочитать только 2 структуры
                if (x = read(fifo, &res, sizeof(res))) { // читаем из файла в структуру
                        if (x == -1) {
                                fprintf(stderr,"fifo stolen\n");
                                return(4);
                        }
                        if (res.pid == pid1) {
                                res1 = res.value;
                                printf("Pi = %lf\n", res.value);
                        } else {
                                res2 = res.value;
                                printf("cos(x) = %lf\n", res.value);
                        }
                        --pcnt;
                }
        }

        close(fifo);
        remove("fifo"); // удаляем именованный поток
        printf("f(x) = %lf\n", res1 * res2);

        return 0;
}