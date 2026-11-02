#include <iostream>
#include "procesos.h"
#include "hilos.h"
#include "particiones.h"
#include "paginacion.h"

using namespace std;

int main()
{
    int op;
    do {
        cout << "\n=== MiniSO ===\n";
        cout << "1. Gestion de procesos\n";
        cout << "2. Gestion de hilos (productor-consumidor)\n";
        cout << "3. Memoria: particionamiento (First-Fit)\n";
        cout << "4. Memoria: paginacion\n";
        cout << "0. Salir\n";
        cout << "Opcion: ";

        if (!(cin >> op))
            break;

        switch (op) {
            case 1:
                menuProcesos();
                break;
            case 2:
                menuHilos();
                break;
            case 3:
                menuParticiones();
                break;
            case 4: 
                menuPaginacion();  
                break;
            case 0: 
                cout << "Saliendo de MiniSO...\n";
                break;
            default: cout << "Opcion invalida\n";
        }
    } while (op != 0);

    return 0;
}