#include "persistencia_repuestos.h"

#include <fstream>
#include <ctime>

using namespace std;

Resultado guardarRepuesto(const Repuesto& nuevo, int& idAsignado) {
    // 1. Leer el header para saber el proximo ID y cuantos registros hay
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_REPUESTOS, h);
    if (r != EXITO) return r;

    // 2. Completar los campos que asigna el sistema
    Repuesto rep = nuevo;
    time_t ahora = time(nullptr);
    rep.id = h.proximoID;
    rep.eliminado = false;
    rep.fechaCreacion = ahora;
    rep.fechaUltimaModificacion = ahora;

    // 3. Escribir al final. El nuevo registro ocupa el indice fisico
    //    'cantidadRegistros' (justo despues del ultimo). Se calcula en
    //    vez de usar seekp(0, ios::end) para que, si un cierre abrupto
    //    dejo un registro huerfano sin actualizar el header, se pise.
    fstream f(RUTA_REPUESTOS, ios::in | ios::out | ios::binary);
    if (!f) return ERR_ARCHIVO;
    f.seekp(posicionRegistro(h.cantidadRegistros, sizeof(Repuesto)));
    f.write(reinterpret_cast<const char*>(&rep), sizeof(Repuesto));
    f.flush();
    if (!f) return ERR_ARCHIVO;
    f.close();

    // 4. Actualizar el header
    h.cantidadRegistros++;
    h.registrosActivos++;
    h.proximoID++;
    r = actualizarHeader(RUTA_REPUESTOS, h);
    if (r != EXITO) return r;

    idAsignado = rep.id;
    return EXITO;
}

Resultado leerRepuestoPorID(int id, Repuesto& destino, long* posicionLeida) {
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_REPUESTOS, h);
    if (r != EXITO) return r;

    // Relacion ID <-> indice fisico: indice = id - 1
    if (id < 1 || id > h.cantidadRegistros) return ERR_NO_EXISTE;

    ifstream f(RUTA_REPUESTOS, ios::binary);
    if (!f) return ERR_ARCHIVO;

    long posicion = posicionPorID(id, sizeof(Repuesto));
    f.seekg(posicion);
    f.read(reinterpret_cast<char*>(&destino), sizeof(Repuesto));
    if (!f) return ERR_ARCHIVO;

    if (posicionLeida) *posicionLeida = posicion;
    return EXITO;
}

Resultado sobrescribirRepuesto(Repuesto& rep) {
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_REPUESTOS, h);
    if (r != EXITO) return r;
    if (rep.id < 1 || rep.id > h.cantidadRegistros) return ERR_NO_EXISTE;

    rep.fechaUltimaModificacion = time(nullptr);

    fstream f(RUTA_REPUESTOS, ios::in | ios::out | ios::binary);
    if (!f) return ERR_ARCHIVO;
    f.seekp(posicionPorID(rep.id, sizeof(Repuesto)));
    f.write(reinterpret_cast<const char*>(&rep), sizeof(Repuesto));
    f.flush();
    return f ? EXITO : ERR_ARCHIVO;
}

Resultado marcarRepuestoEliminado(int id) {
    Repuesto rep;
    Resultado r = leerRepuestoPorID(id, rep);
    if (r != EXITO) return r;
    if (rep.eliminado) return ERR_ELIMINADO;     // evita restar dos veces en el header

    rep.eliminado = true;
    r = sobrescribirRepuesto(rep);
    if (r != EXITO) return r;

    ArchivoHeader h;
    r = leerHeader(RUTA_REPUESTOS, h);
    if (r != EXITO) return r;
    h.registrosActivos--;
    return actualizarHeader(RUTA_REPUESTOS, h);
}

Resultado contarRepuestosActivos(int& cantidad) {
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_REPUESTOS, h);
    if (r != EXITO) return r;
    cantidad = h.registrosActivos;
    return EXITO;
}
