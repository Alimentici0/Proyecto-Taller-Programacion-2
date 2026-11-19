#include "persistencia_mecanicos.h"

#include <fstream>
#include <ctime>

using namespace std;

Resultado guardarMecanico(const Mecanico& nuevo, int& idAsignado) {
    // 1. Leer el header para saber el proximo ID y cuantos registros hay
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_MECANICOS, h);
    if (r != EXITO) return r;

    // 2. Completar los campos que asigna el sistema
    Mecanico m = nuevo;
    time_t ahora = time(nullptr);
    m.id = h.proximoID;
    m.eliminado = false;
    m.fechaCreacion = ahora;
    m.fechaUltimaModificacion = ahora;

    // 3. Escribir al final. El nuevo registro ocupa el indice fisico
    //    'cantidadRegistros' (justo despues del ultimo). Se calcula en
    //    vez de usar seekp(0, ios::end) para que, si un cierre abrupto
    //    dejo un registro huerfano sin actualizar el header, se pise.
    fstream f(RUTA_MECANICOS, ios::in | ios::out | ios::binary);
    if (!f) return ERR_ARCHIVO;
    f.seekp(posicionRegistro(h.cantidadRegistros, sizeof(Mecanico)));
    f.write(reinterpret_cast<const char*>(&m), sizeof(Mecanico));
    f.flush();
    if (!f) return ERR_ARCHIVO;
    f.close();

    // 4. Actualizar el header
    h.cantidadRegistros++;
    h.registrosActivos++;
    h.proximoID++;
    r = actualizarHeader(RUTA_MECANICOS, h);
    if (r != EXITO) return r;

    idAsignado = m.id;
    return EXITO;
}

Resultado leerMecanicoPorID(int id, Mecanico& destino, long* posicionLeida) {
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_MECANICOS, h);
    if (r != EXITO) return r;

    // Relacion ID <-> indice fisico: indice = id - 1
    if (id < 1 || id > h.cantidadRegistros) return ERR_NO_EXISTE;

    ifstream f(RUTA_MECANICOS, ios::binary);
    if (!f) return ERR_ARCHIVO;

    long posicion = posicionPorID(id, sizeof(Mecanico));
    f.seekg(posicion);
    f.read(reinterpret_cast<char*>(&destino), sizeof(Mecanico));
    if (!f) return ERR_ARCHIVO;

    if (posicionLeida) *posicionLeida = posicion;
    return EXITO;
}

Resultado sobrescribirMecanico(Mecanico& m) {
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_MECANICOS, h);
    if (r != EXITO) return r;
    if (m.id < 1 || m.id > h.cantidadRegistros) return ERR_NO_EXISTE;

    m.fechaUltimaModificacion = time(nullptr);

    fstream f(RUTA_MECANICOS, ios::in | ios::out | ios::binary);
    if (!f) return ERR_ARCHIVO;
    f.seekp(posicionPorID(m.id, sizeof(Mecanico)));
    f.write(reinterpret_cast<const char*>(&m), sizeof(Mecanico));
    f.flush();
    return f ? EXITO : ERR_ARCHIVO;
}

Resultado marcarMecanicoEliminado(int id) {
    Mecanico m;
    Resultado r = leerMecanicoPorID(id, m);
    if (r != EXITO) return r;
    if (m.eliminado) return ERR_ELIMINADO;     // evita restar dos veces en el header

    m.eliminado = true;
    r = sobrescribirMecanico(m);
    if (r != EXITO) return r;

    ArchivoHeader h;
    r = leerHeader(RUTA_MECANICOS, h);
    if (r != EXITO) return r;
    h.registrosActivos--;
    return actualizarHeader(RUTA_MECANICOS, h);
}

Resultado contarMecanicosActivos(int& cantidad) {
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_MECANICOS, h);
    if (r != EXITO) return r;
    cantidad = h.registrosActivos;
    return EXITO;
}
