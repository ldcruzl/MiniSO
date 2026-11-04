#include "particiones.h"

#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

namespace {

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

// Lectura de identificador omitiendo espacios y nombres reservados.
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

SimuladorParticiones::SimuladorParticiones(size_t capacidadInicial)
    : capacidadTotal(capacidadInicial), siguienteIdBloque(1)
{
    reiniciar(capacidadInicial);
}

void SimuladorParticiones::reiniciar(size_t nuevaCapacidad)
{
    capacidadTotal = nuevaCapacidad;
    siguienteIdBloque = 1;
    bloques.clear();
    bloques.push_back(BloqueMemoria{
        siguienteIdBloque++,
        "LIBRE",
        0,
        capacidadTotal,
        true
    });
}

// Metricas: calcula memoria ocupada, libre y mayor bloque continuo en una sola pasada.
MetricasMemoria SimuladorParticiones::calcularMetricas() const
{
    MetricasMemoria m;
    for (const auto& b : bloques) {
        if (b.libre) {
            m.libre += b.tamano;
            if (b.tamano > m.mayorLibre) {
                m.mayorLibre = b.tamano;
            }
        } else {
            m.ocupada += b.tamano;
        }
    }
    return m;
}

bool SimuladorParticiones::existeProceso(const string& proceso) const
{
    for (const auto& b : bloques) {
        if (!b.libre && b.proceso == proceso) {
            return true;
        }
    }
    return false;
}

size_t SimuladorParticiones::obtenerMemoriaTotal() const
{
    return capacidadTotal;
}

const vector<BloqueMemoria>& SimuladorParticiones::obtenerBloques() const
{
    return bloques;
}

// First-Fit: recorre la memoria desde el inicio y selecciona el primer bloque libre
// cuyo tamano sea suficiente (tamano >= tamanoSolicitado).
bool SimuladorParticiones::asignarFirstFit(const string& proceso, size_t tamanoSolicitado)
{
    if (proceso.empty() || proceso == "LIBRE") {
        cout << "[ERROR] Identificador de proceso invalido.\n";
        return false;
    }
    if (tamanoSolicitado == 0) {
        cout << "[ERROR] El tamano solicitado debe ser mayor a 0 KB.\n";
        return false;
    }
    if (tamanoSolicitado > capacidadTotal) {
        cout << "[ERROR] El tamano solicitado (" << tamanoSolicitado
             << " KB) supera la capacidad total (" << capacidadTotal << " KB).\n";
        return false;
    }
    if (existeProceso(proceso)) {
        cout << "[ERROR] El proceso '" << proceso
             << "' ya cuenta con memoria asignada en el sistema.\n";
        return false;
    }

    for (size_t i = 0; i < bloques.size(); ++i) {
        if (bloques[i].libre && bloques[i].tamano >= tamanoSolicitado) {
            size_t dirInicio = bloques[i].inicio;
            
            // Particionamiento dinamico (splitting): si el bloque libre supera la solicitud,
            // se divide creando una particion ocupada y un nuevo bloque libre residual.
            if (bloques[i].tamano == tamanoSolicitado) {
                bloques[i].libre = false;
                bloques[i].proceso = proceso;
            } else {
                size_t remanente = bloques[i].tamano - tamanoSolicitado;
                size_t inicioRemanente = dirInicio + tamanoSolicitado;

                bloques[i].tamano = tamanoSolicitado;
                bloques[i].libre = false;
                bloques[i].proceso = proceso;

                BloqueMemoria bloqueResiduo{
                    siguienteIdBloque++,
                    "LIBRE",
                    inicioRemanente,
                    remanente,
                    true
                };
                bloques.insert(bloques.begin() + i + 1, bloqueResiduo);
            }

            cout << "[ASIGNACION] Proceso '" << proceso << "' asignado en bloque ["
                 << dirInicio << " - " << (dirInicio + tamanoSolicitado)
                 << ") KB (Tamano: " << tamanoSolicitado << " KB).\n";
            return true;
        }
    }

    // Fragmentacion externa: hay suficiente memoria libre global, pero no en un bloque contiguo.
    MetricasMemoria m = calcularMetricas();
    if (tamanoSolicitado <= m.libre) {
        cout << "[FRAGMENTACION EXTERNA] No se pudo asignar " << tamanoSolicitado
             << " KB al proceso '" << proceso << "'.\n";
        cout << "Memoria libre total: " << m.libre << " KB | Mayor bloque contiguo: "
             << m.mayorLibre << " KB.\n";
        cout << "Causa: Existe suficiente memoria acumulada, pero ningun bloque contiguo alcanza el tamano solicitado.\n";
    } else {
        cout << "[ERROR: MEMORIA INSUFICIENTE] No fue posible asignar " << tamanoSolicitado
             << " KB al proceso '" << proceso << "'.\n";
        cout << "Memoria libre total: " << m.libre << " KB | Requerimiento: " << tamanoSolicitado << " KB.\n";
    }
    return false;
}

// Liberacion: marca el bloque como libre y ejecuta coalescing automatico.
bool SimuladorParticiones::liberar(const string& proceso)
{
    if (proceso.empty()) return false;

    for (auto& b : bloques) {
        if (!b.libre && b.proceso == proceso) {
            cout << "[LIBERACION] Proceso '" << proceso << "' liberado del bloque ["
                 << b.inicio << " - " << (b.inicio + b.tamano) << ") KB.\n";
            b.libre = true;
            b.proceso = "LIBRE";
            coalescer();
            return true;
        }
    }

    cout << "[ERROR] No se encontro memoria asignada al proceso '" << proceso << "'.\n";
    return false;
}

// Coalescing: fusiona de inmediato bloques contiguos libres para consolidar espacio.
void SimuladorParticiones::coalescer()
{
    bool huboFusion = false;
    for (size_t i = 0; i + 1 < bloques.size(); ) {
        if (bloques[i].libre && bloques[i + 1].libre) {
            bloques[i].tamano += bloques[i + 1].tamano;
            bloques.erase(bloques.begin() + i + 1);
            huboFusion = true;
        } else {
            ++i;
        }
    }
    if (huboFusion) {
        cout << "[COALESCING] Se fusionaron bloques libres contiguos.\n";
    }
}

void SimuladorParticiones::mostrarMemoria() const
{
    cout << "\n--- Estado de la Memoria ---\n";
    size_t ocupados = 0;
    size_t libres = 0;

    for (size_t i = 0; i < bloques.size(); ++i) {
        const auto& b = bloques[i];
        string estado = b.libre ? "LIBRE" : ("OCUPADO (" + b.proceso + ")");
        if (b.libre) ++libres; else ++ocupados;

        cout << "Bloque " << (i + 1) << ": [" << b.inicio << " - "
             << (b.inicio + b.tamano) << ") KB\t| Tamano: " << b.tamano
             << " KB\t| " << estado << "\n";
    }

    MetricasMemoria m = calcularMetricas();
    double pctOcupado = (capacidadTotal > 0) ? (static_cast<double>(m.ocupada) * 100.0 / capacidadTotal) : 0.0;
    double pctLibre = (capacidadTotal > 0) ? (static_cast<double>(m.libre) * 100.0 / capacidadTotal) : 0.0;

    cout << "\nMemoria total: " << capacidadTotal << " KB | Ocupada: "
         << m.ocupada << " KB (" << fixed << setprecision(1) << pctOcupado
         << "%) | Libre: " << m.libre << " KB (" << pctLibre << "%)\n";
    cout << "Mayor bloque libre contiguo: " << m.mayorLibre << " KB\n";
    cout << "Total de bloques: " << bloques.size() << " (" << ocupados
         << " ocupado" << (ocupados == 1 ? "" : "s") << ", " << libres
         << " libre" << (libres == 1 ? "" : "s") << ")\n";
}

void SimuladorParticiones::mostrarDescripcion() const
{
    cout << "\nParticionamiento contiguo de memoria con algoritmo First-Fit.\n"
         << "La memoria se divide dinamicamente en particiones variables segun la demanda.\n"
         << "First-Fit asigna en el primer bloque libre con tamano suficiente, dividiendo\n"
         << "el excedente como un nuevo bloque libre residual.\n"
         << "Al liberar un bloque, la coalescencia fusiona de inmediato bloques contiguos libres.\n"
         << "La fragmentacion externa ocurre cuando la memoria libre total acumulada es\n"
         << "suficiente para una solicitud, pero ningun bloque contiguo individual alcanza su tamano.\n"
         << "Capacidad predeterminada: 1000 KB. Todas las operaciones son interactivas.\n";
}

// Interfaz interactiva: traduce operaciones del usuario al simulador de particiones.
void menuParticiones()
{
    static SimuladorParticiones simulador(1000);
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    while (true) {
        cout << "\n=== Modulo III: Gestion de memoria (Particionamiento) ===\n"
             << "1. Asignar memoria a proceso (First-Fit)\n"
             << "2. Liberar memoria de proceso\n"
             << "3. Visualizar estado de la memoria\n"
             << "4. Reconfigurar capacidad de memoria\n"
             << "5. Mostrar descripcion del modulo\n"
             << "0. Volver al menu principal\n";

        int opcion;
        if (!leerEntero("Opcion: ", 0, 5, opcion) || opcion == 0) return;

        switch (opcion) {
            case 1: {
                string proceso;
                int tamano;
                if (!leerIdentificador("Identificador del proceso (ej: P1): ", proceso)) return;
                int maxPermitido = static_cast<int>(simulador.obtenerMemoriaTotal());
                string msgTamano = "Tamano en KB (1-" + to_string(maxPermitido) + "): ";
                if (!leerEntero(msgTamano, 1, maxPermitido, tamano)) return;
                simulador.asignarFirstFit(proceso, static_cast<size_t>(tamano));
                break;
            }
            case 2: {
                string proceso;
                if (!leerIdentificador("Identificador del proceso a liberar: ", proceso)) return;
                simulador.liberar(proceso);
                break;
            }
            case 3:
                simulador.mostrarMemoria();
                break;
            case 4: {
                int nuevaCap;
                if (!leerEntero("Nueva capacidad total de memoria en KB (1-100000): ",
                                1, 100000, nuevaCap)) return;
                simulador.reiniciar(static_cast<size_t>(nuevaCap));
                cout << "[EXITO] Memoria reiniciada a " << nuevaCap << " KB.\n";
                break;
            }
            case 5:
                simulador.mostrarDescripcion();
                break;
            default:
                break;
        }
    }
}
