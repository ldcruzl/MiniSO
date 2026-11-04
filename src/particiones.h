#ifndef PARTICIONES_H
#define PARTICIONES_H

#include <cstddef>
#include <string>
#include <vector>

// Estructura que representa una particion contigua de memoria variable.
// El inicio define la direccion base y libre indica disponibilidad.
struct BloqueMemoria {
    int id;
    std::string proceso;
    std::size_t inicio;
    std::size_t tamano;
    bool libre;
};

class SimuladorParticiones {
public:
    explicit SimuladorParticiones(std::size_t capacidadInicial = 1000);

    void reiniciar(std::size_t nuevaCapacidad);
    std::size_t obtenerMemoriaTotal() const;
    bool existeProceso(const std::string& proceso) const;
    const std::vector<BloqueMemoria>& obtenerBloques() const;

private:
    std::size_t capacidadTotal;
    std::vector<BloqueMemoria> bloques;
    int siguienteIdBloque;
};

void menuParticiones();

#endif