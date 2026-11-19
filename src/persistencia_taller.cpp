#include "persistencia_taller.h"

#include <fstream>
#include <ctime>

using namespace std;

bool existeTallerGuardado() {
    return existeArchivo(RUTA_TALLER);
}

Resultado leerTaller(Taller& destino) {
    ifstream f(RUTA_TALLER, ios::binary);
    if (!f) return ERR_ARCHIVO;

    f.read(reinterpret_cast<char*>(&destino), sizeof(Taller));
    return f ? EXITO : ERR_ARCHIVO;
}

Resultado guardarTaller(const Taller& datos) {
    Taller t = datos;
    time_t ahora = time(nullptr);

    Taller anterior;
    if (existeTallerGuardado() && leerTaller(anterior) == EXITO) {
        t.fechaCreacion = anterior.fechaCreacion;     // se conserva la original
    } else {
        t.fechaCreacion = ahora;
    }
    t.fechaUltimaModificacion = ahora;

    ofstream f(RUTA_TALLER, ios::binary | ios::trunc);
    if (!f) return ERR_ARCHIVO;
    f.write(reinterpret_cast<const char*>(&t), sizeof(Taller));
    f.flush();
    return f ? EXITO : ERR_ARCHIVO;
}
