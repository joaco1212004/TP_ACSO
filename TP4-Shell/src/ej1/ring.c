#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <errno.h>

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Error: anillo <n> <c> <s>\n");
        return 1;
    }

    char *endptr;
    errno = 0;

    long tmp_n = strtol(argv[1], &endptr, 10);
    if (errno != 0 || *endptr != '\0' || tmp_n < 1 || tmp_n > INT_MAX) {
        fprintf(stderr, "Error: <n> debe ser un entero positivo válido (entre 1 y %d).\n", INT_MAX);
        return 1;
    }

    int n = (int) tmp_n;

    errno = 0;
    long tmp_val = strtol(argv[2], &endptr, 10);
    if (errno != 0 || *endptr != '\0' || tmp_val < INT_MIN || tmp_val > INT_MAX) {
        fprintf(stderr, "Error: <c> debe ser un entero válido entre %d y %d.\n", INT_MIN, INT_MAX);
        return 1;
    }

    int val = (int) tmp_val;

    errno = 0;
    long tmp_start = strtol(argv[3], &endptr, 10);
    if (errno != 0 || *endptr != '\0' || tmp_start < 1 || tmp_start > n) {
        fprintf(stderr, "Error: <s> debe estar entre 1 y %d (inclusive).\n", n);
        return 1;
    }

    int start = (int) tmp_start - 1;

    int ring[n][2];
    for (int i = 0; i < n; i++) {
        if (pipe(ring[i]) == -1) {
            perror("pipe");
            return 1;
        }
    }

    int p2c[2], c2p[2];
    if (pipe(p2c) == -1 || pipe(c2p) == -1) {
        perror("pipe");
        return 1;
    }

    // Procesos de los Hijos
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Hij

            for (int j = 0; j < n; j++) {
                if (j != i) close(ring[j][1]);
                if (j != (i - 1 + n) % n) close(ring[j][0]);
            }

            if (i != start) {
                close(p2c[0]); close(p2c[1]);
                close(c2p[0]); close(c2p[1]);
            }

            int buffer;

            if (i == start) {
                // Hijo que inicia la comunicación
                close(p2c[1]); close(c2p[0]);
                if (read(p2c[0], &buffer, sizeof(int)) != sizeof(int)) {
                    perror("read (p2c)");
                    exit(1);
                }

                buffer++;
                write(ring[i][1], &buffer, sizeof(int));
                if (read(ring[(i - 1 + n) % n][0], &buffer, sizeof(int)) != sizeof(int)) {
                    perror("read (ring)");
                    exit(1);
                }

                write(c2p[1], &buffer, sizeof(int));
                close(p2c[0]); close(c2p[1]);
            } else {
                // Hijo intermedio
                if (read(ring[(i - 1 + n) % n][0], &buffer, sizeof(int)) != sizeof(int)) {
                    perror("read (ring intermedio)");
                    exit(1);
                }

                buffer++;
                write(ring[i][1], &buffer, sizeof(int));
            }
            exit(0);
        }
    }

    // Padre
    close(p2c[0]); close(c2p[1]);
    write(p2c[1], &val, sizeof(int));
    close(p2c[1]);

    int result;
    if (read(c2p[0], &result, sizeof(int)) != sizeof(int)) {
        perror("read (c2p)");
        return 1;
    }

    close(c2p[0]);

    printf("Resultado: %d\n", result);

    for (int i = 0; i < n; i++) wait(NULL);

    return 0;
}
