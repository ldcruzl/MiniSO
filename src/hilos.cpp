#include "hilos.h"

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <vector>

namespace {

struct Configuracion {
    int productores = 2;
    int consumidores = 2;
    int capacidad = 5;
    int elementos = 20;
};

// Nunca se adquiere el mutex del buffer mientras se posee el de consola.
class Consola {
public:
    template <typename... Partes>
    void escribir(const Partes&... partes)
    {
        std::lock_guard<std::mutex> bloqueo(mutex);
        (std::cout << ... << partes) << '\n';
    }

private:
    std::mutex mutex;
};

struct Resultado {
    int producidos = 0;
    int consumidos = 0;
    int pendientes = 0;
    int duplicadosProduccion = 0;
    int duplicadosConsumo = 0;
    int faltantesProduccion = 0;
    int faltantesConsumo = 0;
    bool cancelada = false;

    bool aprobado(int solicitados) const
    {
        return !cancelada && producidos == solicitados &&
               consumidos == solicitados && pendientes == 0 &&
               duplicadosProduccion == 0 && duplicadosConsumo == 0 &&
               faltantesProduccion == 0 && faltantesConsumo == 0;
    }
};

class BufferCircular {
public:
    BufferCircular(int capacidad, int elementos, Consola& salida)
        : datos(capacidad), producciones(elementos, 0),
          consumos(elementos, 0), consola(salida)
    {
    }

    bool insertar(int elemento, int productor)
    {
        std::unique_lock<std::mutex> bloqueo(mutex);
        if (ocupados == datos.size() && !cancelada) {
            consola.escribir("[PRODUCTOR P", productor,
                             "] Buffer lleno. Esperando espacio...");
        }
        notFull.wait(bloqueo, [this] {
            return ocupados < datos.size() || cancelada;
        });
        if (cancelada) return false;

        datos[entrada] = elemento;
        entrada = (entrada + 1) % datos.size();
        ++ocupados;
        ++producidos;
        ++producciones.at(static_cast<std::size_t>(elemento - 1));
        consola.escribir("[PRODUCTOR P", productor, "] Produjo elemento: ",
                         elemento, ". Buffer: ", ocupados, "/", datos.size());
        bloqueo.unlock();
        notEmpty.notify_one();
        return true;
    }

    bool extraer(int& elemento, int consumidor)
    {
        std::unique_lock<std::mutex> bloqueo(mutex);
        if (ocupados == 0 && !produccionTerminada && !cancelada) {
            consola.escribir("[CONSUMIDOR C", consumidor,
                             "] Buffer vacio. Esperando elementos...");
        }
        notEmpty.wait(bloqueo, [this] {
            return ocupados > 0 || produccionTerminada || cancelada;
        });
        if (cancelada || ocupados == 0) return false;

        elemento = datos[salida];
        salida = (salida + 1) % datos.size();
        --ocupados;
        ++consumidos;
        ++consumos.at(static_cast<std::size_t>(elemento - 1));
        consola.escribir("[CONSUMIDOR C", consumidor, "] Consumio elemento: ",
                         elemento, ". Buffer: ", ocupados, "/", datos.size());
        bloqueo.unlock();
        notFull.notify_one();
        return true;
    }

    void terminarProduccion()
    {
        {
            std::lock_guard<std::mutex> bloqueo(mutex);
            produccionTerminada = true;
        }
        notEmpty.notify_all();
    }

    // Desbloquea ambos grupos si falla un trabajador o la creacion de hilos.
    void cancelar()
    {
        {
            std::lock_guard<std::mutex> bloqueo(mutex);
            cancelada = true;
        }
        notEmpty.notify_all();
        notFull.notify_all();
    }

    Resultado resultado()
    {
        std::lock_guard<std::mutex> bloqueo(mutex);
        Resultado resumen;
        resumen.producidos = producidos;
        resumen.consumidos = consumidos;
        resumen.pendientes = static_cast<int>(ocupados);
        resumen.cancelada = cancelada;
        for (std::size_t i = 0; i < producciones.size(); ++i) {
            if (producciones[i] == 0) ++resumen.faltantesProduccion;
            if (consumos[i] == 0) ++resumen.faltantesConsumo;
            if (producciones[i] > 1)
                resumen.duplicadosProduccion += producciones[i] - 1;
            if (consumos[i] > 1)
                resumen.duplicadosConsumo += consumos[i] - 1;
        }
        return resumen;
    }

private:
    std::vector<int> datos;
    std::vector<int> producciones;
    std::vector<int> consumos;
    std::size_t entrada = 0;
    std::size_t salida = 0;
    std::size_t ocupados = 0;
    int producidos = 0;
    int consumidos = 0;
    bool produccionTerminada = false;
    bool cancelada = false;
    std::mutex mutex;
    std::condition_variable notEmpty;
    std::condition_variable notFull;
    Consola& consola;
};

} // namespace
void menuHilos()
{
    std::cout << "menuHilos: en construccion\n";
}
