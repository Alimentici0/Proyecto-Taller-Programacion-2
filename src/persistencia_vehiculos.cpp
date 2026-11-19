#include "persistencia_vehiculos.h"

#include <fstream>
#include <ctime>

using namespace std;

Resultado guardarVehiculo(const Vehiculo& nuevo, int& idAsignado) {
    // 1. Leer el header para saber el proximo ID y cuantos registros hay
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_VEHICULOS, h);
    if (r != EXITO) return r;

    // 2. Completar los campos que asigna el sistema
    Vehiculo v = nuevo;
    time_t ahora = time(nullptr);
    v.id = h.proximoID;
    v.eliminado = false;
    v.fechaCreacion = ahora;
    v.fechaUltimaModificacion = ahora;

    // 3. Escribir al final. El nuevo registro ocupa el indice fisico
    //    'cantidadRegistros' (justo despues del ultimo). Se calcula en
    //    vez de usar seekp(0, ios::end) para que, si un cierre abrupto
    //    dejo un registro huerfano sin actualizar el header, se pise.
    fstream f(RUTA_VEHICULOS, ios::in | ios::out | ios::binary);
    if (!f) return ERR_ARCHIVO;
    f.seekp(posicionRegistro(h.cantidadRegistros, sizeof(Vehiculo)));
    f.write(reinterpret_cast<const char*>(&v), sizeof(Vehiculo));
    f.flush();
    if (!f) return ERR_ARCHIVO;
    f.close();

    // 4. Actualizar el header
    h.cantidadRegistros++;
    h.registrosActivos++;
    h.proximoID++;
    r = actualizarHeader(RUTA_VEHICULOS, h);
    if (r != EXITO) return r;

    idAsignado = v.id;
    return EXITO;
}

Resultado leerVehiculoPorID(int id, Vehiculo& destino, long* posicionLeida) {
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_VEHICULOS, h);
    if (r != EXITO) return r;

    // Relacion ID <-> indice fisico: indice = id - 1
    if (id < 1 || id > h.cantidadRegistros) return ERR_NO_EXISTE;

    ifstream f(RUTA_VEHICULOS, ios::binary);
    if (!f) return ERR_ARCHIVO;

    long posicion = posicionPorID(id, sizeof(Vehiculo));
    f.seekg(posicion);
    f.read(reinterpret_cast<char*>(&destino), sizeof(Vehiculo));
    if (!f) return ERR_ARCHIVO;

    if (posicionLeida) *posicionLeida = posicion;
    return EXITO;
}

Resultado sobrescribirVehiculo(Vehiculo& v) {
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_VEHICULOS, h);
    if (r != EXITO) return r;
    if (v.id < 1 || v.id > h.cantidadRegistros) return ERR_NO_EXISTE;

    v.fechaUltimaModificacion = time(nullptr);

    fstream f(RUTA_VEHICULOS, ios::in | ios::out | ios::binary);
    if (!f) return ERR_ARCHIVO;
    f.seekp(posicionPorID(v.id, sizeof(Vehiculo)));
    f.write(reinterpret_cast<const char*>(&v), sizeof(Vehiculo));
    f.flush();
    return f ? EXITO : ERR_ARCHIVO;
}

Resultado marcarVehiculoEliminado(int id) {
    Vehiculo v;
    Resultado r = leerVehiculoPorID(id, v);
    if (r != EXITO) return r;
    if (v.eliminado) return ERR_ELIMINADO;     // evita restar dos veces en el header

    v.eliminado = true;
    r = sobrescribirVehiculo(v);
    if (r != EXITO) return r;

    ArchivoHeader h;
    r = leerHeader(RUTA_VEHICULOS, h);
    if (r != EXITO) return r;
    h.registrosActivos--;
    return actualizarHeader(RUTA_VEHICULOS, h);
}

Resultado contarVehiculosActivos(int& cantidad) {
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_VEHICULOS, h);
    if (r != EXITO) return r;
    cantidad = h.registrosActivos;
    return EXITO;
}
