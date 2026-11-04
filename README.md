# MiniSO

Aplicación de consola en C++ que integra gestión de procesos, hilos y memoria
en Linux. Proyecto de la Unidad 01 de Sistemas Operativos.

## Información del curso

- **Curso:** Sistemas Operativos
- **Docente:** Ing. Hugo Manuel Barraza Vizcarra
- **Jefe de práctica:** Ing. Jimmy Muñoz Miranda

## Módulos

1. **Gestión de procesos:** `fork()`, `exec()`, `wait()` / `waitpid()`, PID/PPID.
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

## Módulo I: gestión de procesos

Desde `./miniso`, selecciona **1** para abrir el módulo y después:

- **1:** ejecuta dos comandos en procesos hijos concurrentes. Se escribe el
  nombre de cada comando (una sola palabra, por ejemplo `ls`, `date` o `pwd`),
  que se busca en `/bin`.
- **2:** demostración con `ps` y `pstree`. Dos hijos ejecutan `sleep` durante los
  segundos que indique el usuario y, mientras corren, el padre muestra los
  procesos con `ps` y el árbol con `pstree`.
- **0:** vuelve al menú principal.

### Organización y llamadas al sistema
La interfaz pública es `menuProcesos()` en `src/procesos.h`; las funciones
auxiliares (`ejecutarDosComandos`, `mostrarEstado`, `verPs`, `verPstree` y
`demostracionConPs`) están en `src/procesos.cpp`.

- **`fork()`:** crea cada proceso hijo. Si falla, se informa el error y se
  continúa con los hijos que sí se crearon.
- **`execl()`:** el hijo reemplaza su programa por `/bin/<comando>`. Si falla,
  informa el error y termina con `_exit(1)`.
- **`getpid()` / `getppid()`:** muestran el PID y el PPID del padre y de cada hijo.
- **`wait()`:** el padre espera una vez por cada hijo creado y recoge al primero
  que termine.
- **`waitpid()`:** espera al proceso auxiliar que ejecuta `ps` o `pstree`, para
  no recoger por error a los hijos `sleep`.
- **Estado de finalización:** se interpreta con `WIFEXITED`, `WEXITSTATUS`,
  `WIFSIGNALED` y `WTERMSIG` (código de salida o señal).

### Validación realizada

| Escenario | Entrada | Resultado |
| --- | --- | --- |
| C1 | `ls` y `date` | Dos hijos con PPID igual al PID del padre; ambos terminan con código 0 |
| C2 | `ls` y `noexiste` | Error de `exec` en el hijo 2, que termina con código 1 |
| C3 | `sleep` de 10 y 5 segundos | `ps` y `pstree` muestran a los hijos; termina antes el de 5 s |
| C4 | tiempo `abc` | `sleep` rechaza el valor, el hijo termina con código 1 y aparece como zombi (`Z+`) en `ps` |

Limitaciones: en la opción 1 cada comando es una sola palabra, sin argumentos, y
se busca solo en `/bin`. Si `pstree` no está instalado: `sudo apt install psmisc`.

## Módulo II: gestión de hilos
Desde `./miniso`, selecciona **2** para abrir el módulo y después:

- **1:** prueba predeterminada (2 productores, 2 consumidores, capacidad 5,
 20 elementos).
- **2:** configura productores, consumidores, capacidad y total de elementos.
- **3:** consulta la descripción del escenario.
- **0:** vuelve al menú principal.

Se requieren entre **2 y 64 hilos de cada tipo**, capacidad entre **1 y 10000**
y entre **1 y 10000 elementos**. Estos límites evitan solicitudes desmesuradas de
recursos y salida. Se rechazan líneas vacías, texto, valores negativos, decimales,
valores fuera de rango y números desbordados. EOF regresa al menú principal,
que termina al detectar también el fin de entrada.

### Organización y sincronización
La interfaz pública sigue siendo `menuHilos()` en `src/hilos.h`. La implementación
y las clases auxiliares son privadas a `src/hilos.cpp`; no usan estado de otros
módulos ni variables globales mutables.

- **`BufferCircular`:** almacena enteros en orden FIFO en un vector de tamaño
  fijo, con índices de entrada/salida y un contador de ocupación. Un mutex
  protege el almacenamiento, los contadores, los registros de integridad y los
  estados de cierre y cancelación.
- **`notFull`:** un productor espera con predicado mientras no haya espacio.
- **`notEmpty`:** un consumidor espera con predicado mientras no haya elementos,
  salvo cierre de producción o cancelación. Las esperas liberan el mutex;
  no se usan pausas ni espera activa para sincronizar.
- **`Consola`:** un segundo mutex mantiene cada mensaje completo. Si se requieren
  ambos bloqueos, siempre se adquiere primero el del búfer y luego el de consola;
  nunca en sentido inverso. La ocupación impresa corresponde a la operación.
- **`ProductorConsumidor`:** crea consumidores y después productores. El productor
  `i` genera `i, i + P, i + 2P, ...` hasta el total solicitado, siendo `P` el número
  de productores. Así no se pierden restos de divisiones y cada ID tiene un dueño.
  Los consumidores registran y muestran cada ID retirado; no hay procesamiento
  adicional simulado ni garantía de reparto equitativo del consumo.

El coordinador une todos los productores, marca el fin de producción bajo mutex
y despierta a todos los consumidores. Estos solo terminan normalmente cuando el
búfer está vacío y la producción ha cerrado. Después se unen todos los consumidores.
Si falla la creación de un hilo o un trabajador lanza una excepción, se cancela
la ejecución, se despiertan ambos grupos y se unen los hilos creados. Una ejecución
cancelada se informa como **NO APROBADA**; no garantiza completar los elementos.
El destructor también cancela y une cualquier hilo pendiente como protección RAII.

El resumen cuenta las operaciones reales y examina la frecuencia de **cada ID**
producido y consumido. Solo aprueba si todos los solicitados aparecen exactamente
una vez en ambos registros, no queda ningún elemento y no hubo cancelación.
El orden entre hilos es no determinista y la salida detallada influye en los tiempos;
este módulo es una demostración de sincronización, no un benchmark.

### Validación realizada
Compilación base y final con `make clean` y `make`, usando el Makefile existente,
C++17, `-Wall -Wextra` y `-pthread`, sin errores ni advertencias.

| Escenario | Productores | Consumidores | Capacidad | Elementos | Resultado |
| --- | ---: | ---: | ---: | ---: | --- |
| Predeterminado | 2 | 2 | 5 | 20 | Aprobado |
| Personalizado | 2 | 2 | 3 | 12 | Aprobado |
| Más consumidores | 3 | 4 | 2 | 20 | Aprobado |
| Búfer mínimo | 2 | 2 | 1 | 20 | Aprobado |
| Un elemento | 2 | 2 | 5 | 1 | Aprobado |
| Cien elementos | 2 | 2 | 5 | 100 | Aprobado |

Un verificador temporal ejecutó estos seis escenarios y 120 repeticiones
adicionales. Contrastó los IDs, el orden FIFO, la ocupación, los hilos iniciados y
finalizados, el resumen y el retorno al menú, con tiempo límite por ejecución.
También probó entradas inválidas (incluyendo desbordamiento y sufijos), EOF en
cada pregunta, descripción, ejecuciones consecutivas y reingreso: **138 ejecuciones**
aprobadas en total. Se comprobó el acceso al Módulo I y a los marcadores existentes
de particionamiento y paginación, sin modificarlos.

Se inyectó un fallo de `pthread_create` en cada una de las cuatro posiciones de la
prueba predeterminada mediante una biblioteca temporal precargada. En los cuatro
casos se verificaron cancelación, finalización de todos los hilos ya creados y
retorno al menú, sin bloqueos. La API de producción sigue usando `std::thread`.
Una compilación temporal con UndefinedBehaviorSanitizer ejecutó además el escenario
3 productores, 4 consumidores, capacidad 1 y 100 elementos sin diagnósticos.
Estas comprobaciones no constituyen una prueba formal de ausencia de carreras.
Los verificadores, bibliotecas y ejecutables de diagnóstico no se incorporan al
repositorio. Para repetir manualmente cada escenario, usa la opción personalizada
con los valores de la tabla; para retirar los artefactos de compilación, `make clean`.

## Módulo III: gestión de memoria — particionamiento

El simulador implementa particionamiento contiguo de memoria variable mediante
el algoritmo First-Fit. Permite configurar la capacidad total, asignar y liberar
memoria por proceso y visualizar el estado de los bloques.

- **Asignación First-Fit:** selecciona el primer bloque libre con espacio
  suficiente. Si sobra memoria, divide el bloque y crea una partición libre
  residual.
- **Liberación y coalescing:** al liberar un proceso, los bloques libres
  contiguos se fusionan automáticamente para recuperar espacios mayores.
- **Fragmentación externa:** identifica cuando la memoria libre total es
  suficiente para una solicitud, pero está distribuida en bloques que no
  alcanzan individualmente el tamaño requerido.
- **Estado de memoria:** muestra los rangos, tamaños y disponibilidad de los
  bloques, además de métricas de memoria ocupada, libre y mayor bloque
  contiguo.

La capacidad predeterminada es de **1000 KB** y puede modificarse desde el
menú del módulo.

## Módulo III: gestión de memoria — paginación

El simulador implementa administración de memoria virtual mediante paginación.
Permite configurar la memoria física y el tamaño de página, crear y liberar
procesos y consultar sus tablas de páginas.

- **Asignación de marcos:** divide los procesos en páginas y las asigna a marcos
  físicos disponibles de forma no contigua.
- **Tabla de páginas:** mantiene la correspondencia entre las páginas de cada
  proceso y los marcos físicos asignados.
- **Traducción de direcciones:** convierte direcciones lógicas en físicas
  mediante el número de página, el desplazamiento y el marco correspondiente.
  También detecta direcciones inválidas o procesos inexistentes.
- **Estado de memoria:** muestra los marcos físicos, sus rangos de direcciones
  y su disponibilidad.
- **Reconfiguración:** permite modificar la memoria física y el tamaño de página,
  reiniciando las asignaciones existentes.

El tamaño de página debe ser una potencia de 2. La configuración predeterminada
es de **1024 B de memoria física**, páginas de **128 B** y **8 marcos**.

## Integrantes

- Luis David Cruz Llica
- Angel Catari Paccoricuna
- Alex Jeanpier Chata Chino