# MiniSO

Aplicación de consola en C++ que integra gestión de procesos, hilos y memoria
en Linux. Proyecto de la Unidad 01 de Sistemas Operativos.

## Información del curso

- **Curso:** Sistemas Operativos
- **Docente:** Ing. Hugo Manuel Barraza Vizcarra
- **Jefe de práctica:** Ing. Jimmy Muñoz Miranda

## Módulos

1. **Gestión de procesos:** `fork()`, `exec()`, `waitpid()`, PID/PPID.
2. **Gestión de hilos:** productor-consumidor con mutex y variables de condición.
3. **Gestión de memoria:** particionamiento (First-Fit) y paginación.

## Requisitos

- Linux (Ubuntu recomendado)
- g++ con soporte C++17 y `make`

## Compilación y ejecución

    make
    ./miniso

Para limpiar los archivos compilados: `make clean`.

## Estructura del proyecto

    MiniSO/
    ├── README.md
    ├── Makefile
    └── src/
        ├── main.cpp
        ├── procesos.cpp / procesos.h
        ├── hilos.cpp / hilos.h
        ├── particiones.cpp / particiones.h
        └── paginacion.cpp / paginacion.h

## Integrantes

- Luis David Cruz Llica
- (integrante 2)
- (integrante 3)