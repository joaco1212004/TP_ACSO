#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h> 

#define MAX_COMMANDS 256
#define MAX_ARGS 64

// Verifica si la línea contiene errores en el uso de pipes (|)
int es_sintaxis_invalida(const char *s) {
    while (*s && isspace((unsigned char)*s)) s++;

    if (*s == '|') return 1;

    const char *end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end)) end--;

    if (*end == '|') return 1;

    for (const char *p = s; *p; p++) {
        if (p[0] == '|' && p[1] == '|') return 1;

        if (p[0] == '|' && isspace((unsigned char)p[1])) {
            const char *q = p + 1;
            while (*q && isspace((unsigned char)*q)) q++;

            if (*q == '|') return 1;
        }
    }

    return 0;
}

// Separar una línea de comando en sus argumentos por comillas
int separar_args(char *cmd, char *args[]) {
    int count = 0;
    char *p = cmd;

    while (*p) {
        while (*p && isspace((unsigned char)*p)) *p++ = '\0';

        if (!*p) break;

        if (count >= MAX_ARGS - 1) return -2;

        if (*p == '"' || *p == '\'') {
            char q = *p++;
            char *inicio = p;
            while (*p && *p != q) p++;

            if (*p != q) return -1;

            *p++ = '\0';
            args[count++] = inicio;
        } else {
            char *inicio = p;
            while (*p && !isspace((unsigned char)*p)) p++;

            if (*p) *p++ = '\0';

            args[count++] = inicio;
        }
    }

    args[count] = NULL;
    return count;
}

int main() {
    char buffer[4096];
    char *cmds[MAX_COMMANDS];

    while (1) {
        // Mostrar el prompt de la shell
        printf("Shell> ");
        fflush(stdout);

        // Leer la línea de entrada
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        
        buffer[strcspn(buffer, "\n")] = '\0';

        // Verificar errores de sintaxis con pipes
        if (es_sintaxis_invalida(buffer)) {
            fprintf(stderr, "Error: sintaxis de pipe inválida\n");
            continue;
        }

        // Separar comandos por pipes por comillas
        int cantidad = 0;
        char *inicio = buffer;
        int dentro = 0;
        char tipo = 0;

        for (char *p = buffer; *p; p++) {
            if (dentro) {if (*p == tipo) dentro = 0;}
            else {
                if (*p == '"' || *p == '\'') {
                    dentro = 1;
                    tipo = *p;
                } else if (*p == '|') {
                    *p = '\0';
                    while (*inicio && isspace((unsigned char)*inicio)) inicio++;

                    char *fin = p - 1;
                    while (fin > inicio && isspace((unsigned char)*fin)) *fin-- = '\0';

                    cmds[cantidad++] = inicio;
                    inicio = p + 1;
                }
            }
        }

        // Agregar el último comando post último pipe
        while (*inicio && isspace((unsigned char)*inicio)) inicio++;

        char *fin = buffer + strlen(buffer) - 1;
        while (fin > inicio && isspace((unsigned char)*fin)) *fin-- = '\0';

        cmds[cantidad++] = inicio;
        if (cantidad == 0) continue;

        // Salir si el comando es "exit"
        if (cantidad == 1 && strcmp(cmds[0], "exit") == 0) break;

        if (cantidad > MAX_COMMANDS) {
            fprintf(stderr, "Error: demasiados comandos\n");
            continue;
        }

        // Crear los pipes necesarios entre procesos
        int pipes[MAX_COMMANDS - 1][2];
        for (int i = 0; i < cantidad - 1; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("pipe");
                exit(1);
            }
        }

        // Crear un hijo por comando
        for (int i = 0; i < cantidad; i++) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                exit(1);
            }

            if (pid == 0) {
                if (i > 0) dup2(pipes[i-1][0], STDIN_FILENO); // Redireccionar entrada si no es el primer comando

                if (i < cantidad - 1) dup2(pipes[i][1], STDOUT_FILENO);// Redireccionar salida si no es el último comando

                // Cerrar todos los pipes en el hijo
                for (int j = 0; j < cantidad - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                // Separar los argumentos del comando
                char *args[MAX_ARGS];
                int ac = separar_args(cmds[i], args);
                if (ac == -1) {
                    fprintf(stderr, "Error: comillas sin cerrar\n");
                    _exit(1);
                } else if (ac == -2) {
                    fprintf(stderr, "Error: demasiados argumentos\n");
                    _exit(1);
                } else if (ac == 0) {
                    fprintf(stderr, "Error: comando vacío\n");
                    _exit(1);
                }

                // Ejecutar el comando
                execvp(args[0], args);
                perror("execvp");
                _exit(1);
            }
        }

        // Cerrar los pipes en el padre
        for (int i = 0; i < cantidad - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        for (int i = 0; i < cantidad; i++) wait(NULL); // Esperar a todos los hijos
    }

    return 0;
}
