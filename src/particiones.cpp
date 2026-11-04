#include "particiones.h"

#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {

// Lectura robusta de enteros con validacion de rango, sin desbordar el flujo de entrada.
bool leerEntero(const std::string& mensaje, int minimo, int maximo, int& valor)
{
    std::string linea;
    while (true) {
        std::cout << mensaje << std::flush;
        if (!std::getline(std::cin, linea)) {
            return false;
        }
        std::istringstream entrada(linea);
        long numero;
        char sobrante;
        if ((entrada >> numero) && !(entrada >> sobrante) &&
            numero >= minimo && numero <= maximo) {
            valor = static_cast<int>(numero);
            return true;
        }
        std::cout << "Entrada invalida. Ingrese un entero entre "
                  << minimo << " y " << maximo << ".\n";
    }
}

// Lectura de identificador de proceso omitiendo espacios y nombres reservados.
bool leerIdentificador(const std::string& mensaje, std::string& valor)
{
    while (true) {
        std::cout << mensaje << std::flush;
        if (!std::getline(std::cin, valor)) {
            return false;
        }
        auto inicio = valor.find_first_not_of(" \t\r\n");
        if (inicio == std::string::npos) {
            std::cout << "Entrada invalida. El identificador no puede estar vacio.\n";
            continue;
        }
        auto fin = valor.find_last_not_of(" \t\r\n");
        valor = valor.substr(inicio, fin - inicio + 1);

        if (valor == "LIBRE" || valor == "-") {
            std::cout << "Identificador reservado. Elija otro nombre.\n";
            continue;
        }
        return true;
    }
}

} // namespace

SimuladorParticiones::SimuladorParticiones(std::size_t capacidadInicial)
    : capacidadTotal(capacidadInicial), siguienteIdBloque(1)
{
    reiniciar(capacidadInicial);
}

void SimuladorParticiones::reiniciar(std::size_t nuevaCapacidad)
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

// Metricas: calcula memoria ocupada, libre y el mayor bloque libre contiguo en una sola pasada.
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

bool SimuladorParticiones::existeProceso(const std::string& proceso) const
{
    for (const auto& b : bloques) {
        if (!b.libre && b.proceso == proceso) {
            return true;
        }
    }
    return false;
}

std::size_t SimuladorParticiones::obtenerMemoriaTotal() const
{
    return capacidadTotal;
}

const std::vector<BloqueMemoria>& SimuladorParticiones::obtenerBloques() const
{
    return bloques;
}

// First-Fit: recorre la memoria desde el inicio y selecciona el primer bloque libre
// cuyo tamano sea suficiente (tamano >= tamanoSolicitado).
bool SimuladorParticiones::asignarFirstFit(const std::string& proceso, std::size_t tamanoSolicitado)
{
    if (proceso.empty() || proceso == "LIBRE") {
        std::cout << "[ERROR] Identificador de proceso invalido.\n";
        return false;
    }
    if (tamanoSolicitado == 0) {
        std::cout << "[ERROR] El tamano solicitado debe ser mayor a 0 KB.\n";
        return false;
    }
    if (tamanoSolicitado > capacidadTotal) {
        std::cout << "[ERROR] El tamano solicitado (" << tamanoSolicitado
                  << " KB) supera la capacidad total del sistema (" << capacidadTotal << " KB).\n";
        return false;
    }
    if (existeProceso(proceso)) {
        std::cout << "[ERROR] El proceso '" << proceso
                  << "' ya cuenta con memoria asignada en el sistema.\n";
        return false;
    }

    for (std::size_t i = 0; i < bloques.size(); ++i) {
        if (bloques[i].libre && bloques[i].tamano >= tamanoSolicitado) {
            std::size_t dirInicio = bloques[i].inicio;

            // Particionamiento dinamico (splitting): si el bloque libre supera la solicitud,
            // se divide creando una particion ocupada y un nuevo bloque libre residual.
            if (bloques[i].tamano == tamanoSolicitado) {
                bloques[i].libre = false;
                bloques[i].proceso = proceso;
            } else {
                std::size_t remanente = bloques[i].tamano - tamanoSolicitado;
                std::size_t inicioRemanente = dirInicio + tamanoSolicitado;

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

            std::cout << "[ASIGNACION] Proceso '" << proceso << "' asignado en bloque ["
                      << dirInicio << " - " << (dirInicio + tamanoSolicitado)
                      << ") KB (Tamano: " << tamanoSolicitado << " KB).\n";
            return true;
        }
    }

    // Fragmentacion externa: existe memoria libre total suficiente,
    // pero ningun bloque contiguo individual alcanza el tamano solicitado.
    MetricasMemoria m = calcularMetricas();
    if (tamanoSolicitado <= m.libre) {
        std::cout << "[FRAGMENTACION EXTERNA] No se pudo asignar " << tamanoSolicitado
                  << " KB al proceso '" << proceso << "'.\n";
        std::cout << "Memoria libre total: " << m.libre << " KB | Mayor bloque contiguo: "
                  << m.mayorLibre << " KB.\n";
        std::cout << "Causa: Existe suficiente memoria acumulada, pero ningun bloque contiguo alcanza el tamano solicitado.\n";
    } else {
        std::cout << "[ERROR: MEMORIA INSUFICIENTE] No fue posible asignar " << tamanoSolicitado
                  << " KB al proceso '" << proceso << "'.\n";
        std::cout << "Memoria libre total: " << m.libre << " KB | Requerimiento: " << tamanoSolicitado << " KB.\n";
    }
    return false;
}

// Liberacion: marca el bloque del proceso como libre y dispara la coalescencia.
bool SimuladorParticiones::liberar(const std::string& proceso)
{
    if (proceso.empty()) return false;

    for (auto& b : bloques) {
        if (!b.libre && b.proceso == proceso) {
            std::cout << "[LIBERACION] Proceso '" << proceso << "' liberado del bloque ["
                      << b.inicio << " - " << (b.inicio + b.tamano) << ") KB.\n";
            b.libre = true;
            b.proceso = "LIBRE";
            coalescer();
            return true;
        }
    }

    std::cout << "[ERROR] No se encontro memoria asignada al proceso '" << proceso << "'.\n";
    return false;
}

// Coalescing: fusiona bloques libres contiguos adyacentes para consolidar
// fragmentos y combatir la fragmentacion externa producida tras liberaciones.
void SimuladorParticiones::coalescer()
{
    bool huboFusion = false;
    for (std::size_t i = 0; i + 1 < bloques.size(); ) {
        if (bloques[i].libre && bloques[i + 1].libre) {
            bloques[i].tamano += bloques[i + 1].tamano;
            bloques.erase(bloques.begin() + i + 1);
            huboFusion = true;
        } else {
            ++i;
        }
    }
    if (huboFusion) {
        std::cout << "[COALESCING] Se fusionaron bloques libres contiguos.\n";
    }
}

void SimuladorParticiones::mostrarMemoria() const
{
    std::cout << "\n--- Estado de la Memoria ---\n";
    std::size_t ocupados = 0;
    std::size_t libres = 0;

    for (std::size_t i = 0; i < bloques.size(); ++i) {
        const auto& b = bloques[i];
        std::string estado = b.libre ? "LIBRE" : ("OCUPADO (" + b.proceso + ")");
        if (b.libre) ++libres; else ++ocupados;

        std::cout << "Bloque " << (i + 1) << ": [" << b.inicio << " - "
                  << (b.inicio + b.tamano) << ") KB\t| Tamano: " << b.tamano
                  << " KB\t| " << estado << "\n";
    }

    MetricasMemoria m = calcularMetricas();
    double pctOcupado = (capacidadTotal > 0) ? (static_cast<double>(m.ocupada) * 100.0 / capacidadTotal) : 0.0;
    double pctLibre = (capacidadTotal > 0) ? (static_cast<double>(m.libre) * 100.0 / capacidadTotal) : 0.0;

    std::cout << "\nMemoria total: " << capacidadTotal << " KB | Ocupada: "
              << m.ocupada << " KB (" << std::fixed << std::setprecision(1) << pctOcupado
              << "%) | Libre: " << m.libre << " KB (" << pctLibre << "%)\n";
    std::cout << "Mayor bloque libre contiguo: " << m.mayorLibre << " KB\n";
    std::cout << "Total de bloques: " << bloques.size() << " (" << ocupados
              << " ocupado" << (ocupados == 1 ? "" : "s") << ", " << libres
              << " libre" << (libres == 1 ? "" : "s") << ")\n";
}

// Interfaz interactiva: traduce las operaciones seleccionadas por el usuario hacia el simulador de memoria.
void menuParticiones()
{
    static SimuladorParticiones simulador(1000);
    // main usa operator>>: retirar solo el resto de su linea antes de getline.
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    while (true) {
        std::cout << "\n=== Modulo III: Gestion de memoria (Particionamiento) ===\n"
                  << "1. Asignar memoria a proceso (First-Fit)\n"
                  << "2. Liberar memoria de proceso\n"
                  << "3. Visualizar estado de la memoria\n"
                  << "0. Volver al menu principal\n";

        int opcion;
        if (!leerEntero("Opcion: ", 0, 3, opcion) || opcion == 0) return;

        switch (opcion) {
            case 1: {
                std::string proceso;
                int tamano;
                if (!leerIdentificador("Identificador del proceso (ej: P1): ", proceso)) return;
                int maxPermitido = static_cast<int>(simulador.obtenerMemoriaTotal());
                std::string msgTamano = "Tamano en KB (1-" + std::to_string(maxPermitido) + "): ";
                if (!leerEntero(msgTamano, 1, maxPermitido, tamano)) return;
                simulador.asignarFirstFit(proceso, static_cast<std::size_t>(tamano));
                break;
            }
            case 2: {
                std::string proceso;
                if (!leerIdentificador("Identificador del proceso a liberar: ", proceso)) return;
                simulador.liberar(proceso);
                break;
            }
            case 3:
                simulador.mostrarMemoria();
                break;
            default:
                break;
        }
    }
}