#include "persistencia_ordenes.h"

#include <fstream>
#include <ctime>

using namespace std;

Resultado guardarOrden(const OrdenServicio& nuevo, int& idAsignado) {
    // 1. Leer el header para saber el proximo ID y cuantos registros hay
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_ORDENES, h);
    if (r != EXITO) return r;

    // 2. Completar los campos que asigna el sistema
    OrdenServicio o = nuevo;
    time_t ahora = time(nullptr);
    o.id = h.proximoID;
    o.eliminado = false;
    o.fechaCreacion = ahora;
    o.fechaUltimaModificacion = ahora;

    // 3. Escribir al final. El nuevo registro ocupa el indice fisico
    //    'cantidadRegistros' (justo despues del ultimo). Se calcula en
    //    vez de usar seekp(0, ios::end) para que, si un cierre abrupto
    //    dejo un registro huerfano sin actualizar el header, se pise.
    fstream f(RUTA_ORDENES, ios::in | ios::out | ios::binary);
    if (!f) return ERR_ARCHIVO;
    f.seekp(posicionRegistro(h.cantidadRegistros, sizeof(OrdenServicio)));
    f.write(reinterpret_cast<const char*>(&o), sizeof(OrdenServicio));
    f.flush();
    if (!f) return ERR_ARCHIVO;
    f.close();

    // 4. Actualizar el header
    h.cantidadRegistros++;
    h.registrosActivos++;
    h.proximoID++;
    r = actualizarHeader(RUTA_ORDENES, h);
    if (r != EXITO) return r;

    idAsignado = o.id;
    return EXITO;
}

Resultado leerOrdenPorID(int id, OrdenServicio& destino, long* posicionLeida) {
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_ORDENES, h);
    if (r != EXITO) return r;

    // Relacion ID <-> indice fisico: indice = id - 1
    if (id < 1 || id > h.cantidadRegistros) return ERR_NO_EXISTE;

    ifstream f(RUTA_ORDENES, ios::binary);
    if (!f) return ERR_ARCHIVO;

    long posicion = posicionPorID(id, sizeof(OrdenServicio));
    f.seekg(posicion);
    f.read(reinterpret_cast<char*>(&destino), sizeof(OrdenServicio));
    if (!f) return ERR_ARCHIVO;

    if (posicionLeida) *posicionLeida = posicion;
    return EXITO;
}

Resultado sobrescribirOrden(OrdenServicio& o) {
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_ORDENES, h);
    if (r != EXITO) return r;
    if (o.id < 1 || o.id > h.cantidadRegistros) return ERR_NO_EXISTE;

    o.fechaUltimaModificacion = time(nullptr);

    fstream f(RUTA_ORDENES, ios::in | ios::out | ios::binary);
    if (!f) return ERR_ARCHIVO;
    f.seekp(posicionPorID(o.id, sizeof(OrdenServicio)));
    f.write(reinterpret_cast<const char*>(&o), sizeof(OrdenServicio));
    f.flush();
    return f ? EXITO : ERR_ARCHIVO;
}

Resultado marcarOrdenEliminada(int id) {
    OrdenServicio o;
    Resultado r = leerOrdenPorID(id, o);
    if (r != EXITO) return r;
    if (o.eliminado) return ERR_ELIMINADO;     // evita restar dos veces en el header

    o.eliminado = true;
    r = sobrescribirOrden(o);
    if (r != EXITO) return r;

    ArchivoHeader h;
    r = leerHeader(RUTA_ORDENES, h);
    if (r != EXITO) return r;
    h.registrosActivos--;
    return actualizarHeader(RUTA_ORDENES, h);
}

Resultado contarOrdenesActivas(int& cantidad) {
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_ORDENES, h);
    if (r != EXITO) return r;
    cantidad = h.registrosActivos;
    return EXITO;
}
