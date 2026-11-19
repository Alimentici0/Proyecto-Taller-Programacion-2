#include "logica_taller.h"
#include "utilidades.h"

bool tallerRegistrado() {
    return existeTallerGuardado();
}

Resultado registrarTaller(const Taller& datos) {
    if (textoVacio(datos.nombre) || textoVacio(datos.rif)) return ERR_DATOS_INVALIDOS;
    return guardarTaller(datos);
}

Resultado cargarTaller(Taller& resultado) {
    return leerTaller(resultado);
}
