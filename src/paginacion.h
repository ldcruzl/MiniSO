#ifndef PAGINACION_H
#define PAGINACION_H

#include <cstddef>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

// Estructura que representa un marco de memoria fisica.
struct MarcoFisico {
    int id;
    bool libre;
    std::string proceso;
    int pagina;
};

// Entrada individual en la tabla de paginas de un proceso.
struct EntradaTablaPagina {
    int pagina;
    int marco;
    bool valida;
};

// Modela el espacio de direcciones y la tabla de paginas de un proceso.
struct ProcesoPaginado {
    std::string nombre;
    std::size_t tamanoBytes;
    std::vector<EntradaTablaPagina> tablaPaginas;
};

class SimuladorPaginacion {
public:
    explicit SimuladorPaginacion(std::size_t memFisica = 1024, std::size_t tamPag = 128);

    bool reiniciar(std::size_t nuevaMemFisica, std::size_t nuevoTamPag);
    bool crearProceso(const std::string& nombre, std::size_t tamanoBytes);
    bool liberarProceso(const std::string& nombre);

    void mostrarEstadoMarcos() const;
    void mostrarTablaProceso(const std::string& nombre) const;

    std::size_t obtenerMemoriaFisica() const;
    std::size_t obtenerTamanoPagina() const;
    std::size_t obtenerTotalMarcos() const;
    std::size_t obtenerMarcosLibres() const;
    bool existeProceso(const std::string& nombre) const;
    const ProcesoPaginado* obtenerProceso(const std::string& nombre) const;
    const std::vector<MarcoFisico>& obtenerMarcos() const;

private:
    std::size_t tamMemoriaFisica;
    std::size_t tamPagina;
    std::size_t totalMarcos;

    std::vector<MarcoFisico> marcos;
    std::deque<int> colaMarcosLibres;
    std::unordered_map<std::string, ProcesoPaginado> procesos;

    void inicializarColaMarcosLibres();
};

void menuPaginacion();

#endif