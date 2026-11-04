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

void menuParticiones()
{
    std::cout << "\n=== Modulo III: Gestion de memoria (Particionamiento) ===\n";
    std::cout << "[INFO] Modulo inicializado.\n";
}