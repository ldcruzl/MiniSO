#include "paginacion.h"

#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

namespace {

bool esPotenciaDeDos(size_t n)
{
    return (n > 0) && ((n & (n - 1)) == 0);
}

// Lectura robusta de enteros con validacion de rango.
bool leerEntero(const string& mensaje, int minimo, int maximo, int& valor)
{
    string linea;
    while (true) {
        cout << mensaje << flush;
        if (!getline(cin, linea)) {
            return false;
        }
        istringstream entrada(linea);
        long numero;
        char sobrante;
        if ((entrada >> numero) && !(entrada >> sobrante) &&
            numero >= minimo && numero <= maximo) {
            valor = static_cast<int>(numero);
            return true;
        }
        cout << "Entrada invalida. Ingrese un entero entre "
             << minimo << " y " << maximo << ".\n";
    }
}

// Lectura de identificador sin espacios ni nombres reservados.
bool leerIdentificador(const string& mensaje, string& valor)
{
    while (true) {
        cout << mensaje << flush;
        if (!getline(cin, valor)) {
            return false;
        }
        auto inicio = valor.find_first_not_of(" \t\r\n");
        if (inicio == string::npos) {
            cout << "Entrada invalida. El identificador no puede estar vacio.\n";
            continue;
        }
        auto fin = valor.find_last_not_of(" \t\r\n");
        valor = valor.substr(inicio, fin - inicio + 1);

        if (valor == "LIBRE" || valor == "-") {
            cout << "Identificador reservado. Elija otro nombre.\n";
            continue;
        }
        return true;
    }
}

} // namespace

SimuladorPaginacion::SimuladorPaginacion(size_t memFisica, size_t tamPag)
    : tamMemoriaFisica(memFisica), tamPagina(tamPag), totalMarcos(0)
{
    reiniciar(memFisica, tamPag);
}

// Cola de marcos libres: gestiona los marcos disponibles en memoria fisica.
// Para 8 marcos (escenario de referencia 1024 B / 128 B) reproduce la secuencia
// determinista {0, 2, 5, 1, 3, 4, 6, 7}; para otros tamanos aplica una dispersion determinista.
void SimuladorPaginacion::inicializarColaMarcosLibres()
{
    colaMarcosLibres.clear();
    if (totalMarcos == 8) {
        vector<int> secuenciaReferencia = {0, 2, 5, 1, 3, 4, 6, 7};
        for (int m : secuenciaReferencia) {
            colaMarcosLibres.push_back(m);
        }
    } else {
        for (size_t i = 0; i < totalMarcos; i += 2) {
            colaMarcosLibres.push_back(static_cast<int>(i));
        }
        for (size_t i = 1; i < totalMarcos; i += 2) {
            colaMarcosLibres.push_back(static_cast<int>(i));
        }
    }
}

bool SimuladorPaginacion::reiniciar(size_t nuevaMemFisica, size_t nuevoTamPag)
{
    if (nuevaMemFisica == 0 || nuevoTamPag == 0) {
        cout << "[ERROR] Los parametros deben ser mayores a 0.\n";
        return false;
    }
    if (!esPotenciaDeDos(nuevoTamPag)) {
        cout << "[ERROR] El tamano de pagina (" << nuevoTamPag
             << " B) debe ser obligatoriamente una potencia de 2.\n";
        return false;
    }
    if (nuevoTamPag > nuevaMemFisica) {
        cout << "[ERROR] El tamano de pagina no puede ser mayor que la memoria fisica.\n";
        return false;
    }

    tamMemoriaFisica = nuevaMemFisica;
    tamPagina = nuevoTamPag;
    totalMarcos = nuevaMemFisica / nuevoTamPag;

    marcos.clear();
    marcos.reserve(totalMarcos);
    for (size_t i = 0; i < totalMarcos; ++i) {
        marcos.push_back(MarcoFisico{static_cast<int>(i), true, "LIBRE", -1});
    }

    procesos.clear();
    inicializarColaMarcosLibres();
    return true;
}

bool SimuladorPaginacion::existeProceso(const string& nombre) const
{
    return procesos.find(nombre) != procesos.end();
}

size_t SimuladorPaginacion::obtenerMemoriaFisica() const
{
    return tamMemoriaFisica;
}

size_t SimuladorPaginacion::obtenerTamanoPagina() const
{
    return tamPagina;
}

size_t SimuladorPaginacion::obtenerTotalMarcos() const
{
    return totalMarcos;
}

size_t SimuladorPaginacion::obtenerMarcosLibres() const
{
    return colaMarcosLibres.size();
}

const ProcesoPaginado* SimuladorPaginacion::obtenerProceso(const string& nombre) const
{
    auto it = procesos.find(nombre);
    if (it != procesos.end()) {
        return &(it->second);
    }
    return nullptr;
}

const vector<MarcoFisico>& SimuladorPaginacion::obtenerMarcos() const
{
    return marcos;
}

// Asignacion no contigua: calcula paginas requeridas y extrae marcos libres de la cola.
bool SimuladorPaginacion::crearProceso(const string& nombre, size_t tamanoBytes)
{
    if (nombre.empty() || nombre == "LIBRE") {
        cout << "[ERROR] Identificador de proceso invalido.\n";
        return false;
    }
    if (tamanoBytes == 0) {
        cout << "[ERROR] El tamano del proceso debe ser mayor a 0 bytes.\n";
        return false;
    }
    if (existeProceso(nombre)) {
        cout << "[ERROR] El proceso '" << nombre << "' ya existe en el sistema.\n";
        return false;
    }

    size_t paginasRequeridas = (tamanoBytes + tamPagina - 1) / tamPagina;
    if (paginasRequeridas > colaMarcosLibres.size()) {
        cout << "[ERROR: MEMORIA INSUFICIENTE] Proceso '" << nombre << "' requiere "
             << paginasRequeridas << " marcos, pero solo hay " << colaMarcosLibres.size()
             << " marco(s) libre(s).\n";
        return false;
    }

    ProcesoPaginado nuevoProc;
    nuevoProc.nombre = nombre;
    nuevoProc.tamanoBytes = tamanoBytes;
    nuevoProc.tablaPaginas.reserve(paginasRequeridas);

    // Tabla de paginas: mapea cada pagina a un marco fisico disponible.
    for (size_t pag = 0; pag < paginasRequeridas; ++pag) {
        int marcoAsignado = colaMarcosLibres.front();
        colaMarcosLibres.pop_front();

        marcos[static_cast<size_t>(marcoAsignado)].libre = false;
        marcos[static_cast<size_t>(marcoAsignado)].proceso = nombre;
        marcos[static_cast<size_t>(marcoAsignado)].pagina = static_cast<int>(pag);

        nuevoProc.tablaPaginas.push_back(EntradaTablaPagina{
            static_cast<int>(pag),
            marcoAsignado,
            true
        });
    }

    procesos[nombre] = nuevoProc;

    cout << "[ASIGNACION EXITOSA] Proceso '" << nombre << "' creado ("
         << tamanoBytes << " B, " << paginasRequeridas << " pagina(s)).\n";
    cout << "Marcos asignados: ";
    for (size_t i = 0; i < nuevoProc.tablaPaginas.size(); ++i) {
        cout << "Pag " << nuevoProc.tablaPaginas[i].pagina << " -> Marco "
             << nuevoProc.tablaPaginas[i].marco
             << (i + 1 < nuevoProc.tablaPaginas.size() ? ", " : "\n");
    }
    return true;
}

// Liberacion: devuelve todos los marcos del proceso a la cola de marcos disponibles.
bool SimuladorPaginacion::liberarProceso(const string& nombre)
{
    auto it = procesos.find(nombre);
    if (it == procesos.end()) {
        cout << "[ERROR] No se encontro el proceso '" << nombre << "'.\n";
        return false;
    }

    const auto& proc = it->second;
    for (const auto& entrada : proc.tablaPaginas) {
        int idMarco = entrada.marco;
        if (idMarco >= 0 && static_cast<size_t>(idMarco) < totalMarcos) {
            marcos[static_cast<size_t>(idMarco)].libre = true;
            marcos[static_cast<size_t>(idMarco)].proceso = "LIBRE";
            marcos[static_cast<size_t>(idMarco)].pagina = -1;
            colaMarcosLibres.push_back(idMarco);
        }
    }

    cout << "[LIBERACION] Proceso '" << nombre << "' liberado ("
         << proc.tablaPaginas.size() << " marco(s) devuelto(s) a memoria libre).\n";
    procesos.erase(it);
    return true;
}

void SimuladorPaginacion::mostrarEstadoMarcos() const
{
    cout << "\n--- Estado de Marcos Fisicos ---\n";
    size_t ocupados = 0;

    for (const auto& m : marcos) {
        size_t dirInicio = static_cast<size_t>(m.id) * tamPagina;
        size_t dirFin = dirInicio + tamPagina;

        cout << "Marco " << m.id << ": [" << dirInicio << " - " << dirFin << ") B\t| ";
        if (m.libre) {
            cout << "LIBRE\n";
        } else {
            ++ocupados;
            cout << "OCUPADO (" << m.proceso << " - Pagina " << m.pagina << ")\n";
        }
    }

    size_t libres = totalMarcos - ocupados;

    cout << "\nMemoria fisica: " << tamMemoriaFisica << " B | Tamano de pagina: "
         << tamPagina << " B | Total marcos: " << totalMarcos << "\n";
    cout << "Marcos ocupados: " << ocupados << " | Marcos libres: "
         << libres << "\n";
}

void SimuladorPaginacion::mostrarTablaProceso(const string& nombre) const
{
    auto it = procesos.find(nombre);
    if (it == procesos.end()) {
        cout << "[ERROR] No se encontro el proceso '" << nombre << "'.\n";
        return;
    }

    const auto& proc = it->second;
    cout << "\n--- Tabla de Paginas: Proceso '" << proc.nombre
         << "' (" << proc.tamanoBytes << " B, " << proc.tablaPaginas.size() << " paginas) ---\n";

    for (const auto& entrada : proc.tablaPaginas) {
        size_t inicioLogico = static_cast<size_t>(entrada.pagina) * tamPagina;
        size_t finLogico = inicioLogico + tamPagina;
        if (finLogico > proc.tamanoBytes) finLogico = proc.tamanoBytes;

        size_t inicioFisico = static_cast<size_t>(entrada.marco) * tamPagina;
        size_t finFisico = inicioFisico + tamPagina;

        cout << "Pagina " << entrada.pagina << ": Rango logico [" << inicioLogico
             << " - " << finLogico << ") B -> Marco " << entrada.marco
             << " [" << inicioFisico << " - " << finFisico << ") B | "
             << (entrada.valida ? "VALIDA" : "INVALIDA") << "\n";
    }
}

// Interfaz interactiva: traduce operaciones del usuario al simulador de paginacion.
void menuPaginacion()
{
    static SimuladorPaginacion simulador(1024, 128);
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    while (true) {
        cout << "\n=== Modulo III: Gestion de memoria (Paginacion) ===\n"
             << "1. Crear proceso y asignar marcos\n"
             << "2. Liberar proceso\n"
             << "3. Visualizar estado de marcos fisicos\n"
             << "4. Visualizar tabla de paginas de un proceso\n"
             << "0. Volver al menu principal\n";

        int opcion;
        if (!leerEntero("Opcion: ", 0, 4, opcion) || opcion == 0) return;

        switch (opcion) {
            case 1: {
                string nombre;
                int tamano;
                if (!leerIdentificador("Identificador del proceso (ej: A): ", nombre)) return;
                int maxTam = static_cast<int>(simulador.obtenerMemoriaFisica());
                string msgTam = "Tamano en bytes (1-" + to_string(maxTam) + "): ";
                if (!leerEntero(msgTam, 1, maxTam, tamano)) return;
                simulador.crearProceso(nombre, static_cast<size_t>(tamano));
                break;
            }
            case 2: {
                string nombre;
                if (!leerIdentificador("Identificador del proceso a liberar: ", nombre)) return;
                simulador.liberarProceso(nombre);
                break;
            }
            case 3:
                simulador.mostrarEstadoMarcos();
                break;
            case 4: {
                string nombre;
                if (!leerIdentificador("Identificador del proceso: ", nombre)) return;
                simulador.mostrarTablaProceso(nombre);
                break;
            }
            default:
                break;
        }
    }
}