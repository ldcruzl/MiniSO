#include "particiones.h"

#include <iostream>
#include <string>
#include <vector>

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

    std::cout << "[ERROR] No fue posible asignar " << tamanoSolicitado
              << " KB al proceso '" << proceso << "'.\n";
    return false;
}

// Liberacion: marca el bloque del proceso como libre.
bool SimuladorParticiones::liberar(const std::string& proceso)
{
    if (proceso.empty()) return false;

    for (auto& b : bloques) {
        if (!b.libre && b.proceso == proceso) {
            std::cout << "[LIBERACION] Proceso '" << proceso << "' liberado del bloque ["
                      << b.inicio << " - " << (b.inicio + b.tamano) << ") KB.\n";
            b.libre = true;
            b.proceso = "LIBRE";
            return true;
        }
    }

    std::cout << "[ERROR] No se encontro memoria asignada al proceso '" << proceso << "'.\n";
    return false;
}

void menuParticiones()
{
    std::cout << "\n=== Modulo III: Gestion de memoria (Particionamiento) ===\n";
    std::cout << "[INFO] Modulo inicializado.\n";
}