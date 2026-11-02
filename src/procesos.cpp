#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include "procesos.h"

using namespace std;

// Muestra como termino un proceso hijo
void mostrarEstado(pid_t pid, int estado)
{
    cout << "El hijo " << pid << " finalizo: ";
    if (WIFEXITED(estado))
        cout << "salida normal, codigo = " << WEXITSTATUS(estado) << endl;
    else if (WIFSIGNALED(estado))
        cout << "terminado por la senal " << WTERMSIG(estado) << endl;
}

// Crea dos hijos que ejecutan los comandos que ingrese el user
void ejecutarDosComandos()
{
    string comando1, comando2;
    int creados = 0;

    cout << "Comando para el hijo 1 (ej: ls, date, pwd): ";
    cin >> comando1;
    cout << "Comando para el hijo 2: ";
    cin >> comando2;

    cout << "Padre - PID = " << getpid() << ", PPID = " << getppid() << endl;

    // Hijo 1
    pid_t pid1 = fork();
    if (pid1 < 0) {
        cout << "Error en fork() del hijo 1." << endl;
    }
    else if (pid1 == 0) {
        cout << "Hijo 1 - PID = " << getpid() << ", PPID = " << getppid() << endl;
        execl((string("/bin/") + comando1).c_str(), comando1.c_str(), (char *)NULL);

        // Si exec falla
        cout << "Error en exec() del hijo 1." << endl;
        _exit(1);
    }
    else {
        creados++;
    }

    // Hijo 2
    pid_t pid2 = fork();
    if (pid2 < 0) {
        cout << "Error en fork() del hijo 2." << endl;
    }
    else if (pid2 == 0) {
        cout << "Hijo 2 - PID = " << getpid() << ", PPID = " << getppid() << endl;
        execl((string("/bin/") + comando2).c_str(), comando2.c_str(), (char *)NULL);

        // Si exec falla
        cout << "Error en exec() del hijo 2." << endl;
        _exit(1);
    }
    else {
        creados++;
    }

    // El padre espera a los hijos que logro crear
    int estado;
    for (int i = 0; i < creados; i++) {
        pid_t fin = wait(&estado);
        mostrarEstado(fin, estado);
    }
}

// Muestra los procesos hijos de MiniSO usando "ps"
void verPs()
{
    string yo = to_string(getpid());

    pid_t pid = fork();
    if (pid < 0) {
        cout << "Error en fork()." << endl;
    }
    else if (pid == 0) {
        execl("/bin/ps", "ps", "-o", "pid,ppid,stat,cmd", "--ppid", yo.c_str(), "-p", yo.c_str(), (char *)NULL);
        cout << "Error en exec()." << endl;
        _exit(1);
    }
    else {
        waitpid(pid, NULL, 0);
    }
}

// Crea dos hijos que duermen distinto tiempo y los muestra con ps
void demostracionConPs()
{
    int creados = 0;
    cout << "Padre - PID = " << getpid() << endl;

    pid_t pid1 = fork();
    if (pid1 < 0) {
        cout << "Error en fork() del hijo 1." << endl;
    }
    else if (pid1 == 0) {
        cout << "Hijo 1 - PID = " << getpid() << ", PPID = " << getppid() << endl;
        execl("/bin/sleep", "sleep", "10", (char *)NULL);
        cout << "Error en exec() del hijo 1." << endl;
        _exit(1);
    }
    else {
        creados++;
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        cout << "Error en fork() del hijo 2." << endl;
    }
    else if (pid2 == 0) {
        cout << "Hijo 2 - PID = " << getpid() << ", PPID = " << getppid() << endl;
        execl("/bin/sleep", "sleep", "5", (char *)NULL);
        cout << "Error en exec() del hijo 2." << endl;
        _exit(1);
    }
    else {
        creados++;
    }

    sleep(1);
    cout << "\n--- Procesos vistos con ps ---" << endl;
    verPs();

    int estado;
    for (int i = 0; i < creados; i++) {
        pid_t fin = wait(&estado);
        mostrarEstado(fin, estado);
    }
}

void menuProcesos()
{
    int op;

    do {
        cout << "\n=== Modulo I: Gestion de procesos ===\n";
        cout << "1. Ejecutar dos comandos en procesos hijos concurrentes\n";
        cout << "2. Demostracion con ps\n";
        cout << "0. Volver al menu principal\n";
        cout << "Opcion: ";;
        if (!(cin >> op))
            break;

        switch (op) {
            case 1:
                ejecutarDosComandos();
                break;
            case 2:
                demostracionConPs();
                break;
            case 0: 
                break;
            default:
                cout << "Opcion invalida\n";
        }
    } while (op != 0);
}