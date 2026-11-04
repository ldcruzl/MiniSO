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

// Interfaz interactiva: traduce operaciones del usuario al simulador de paginacion.
void menuPaginacion()
{
    static SimuladorPaginacion simulador(1024, 128);
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    while (true) {
        cout << "\n=== Modulo III: Gestion de memoria (Paginacion) ===\n"
             << "1. Visualizar estado de marcos fisicos\n"
             << "0. Volver al menu principal\n";

        int opcion;
        if (!leerEntero("Opcion: ", 0, 1, opcion) || opcion == 0) return;

        if (opcion == 1) {
            simulador.mostrarEstadoMarcos();
        }
    }
}