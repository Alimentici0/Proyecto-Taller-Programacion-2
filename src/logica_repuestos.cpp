#include "logica_repuestos.h"
#include "persistencia_ordenes.h"
#include "utilidades.h"

#include <climits>

using namespace std;

Resultado cantidadRepuestosRegistrados(int& total) {
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_REPUESTOS, h);
    if (r != EXITO) return r;
    total = h.cantidadRegistros;
    return EXITO;
}

Resultado cantidadRepuestosActivos(int& activos) {
    return contarRepuestosActivos(activos);
}

bool repuestoBajoStock(const Repuesto& repuesto) {
    return repuesto.stock <= repuesto.stockMinimo;
}

Resultado registrarRepuesto(const Repuesto& datos, int& idAsignado) {
    if (textoVacio(datos.codigo) || textoVacio(datos.nombre)) return ERR_DATOS_INVALIDOS;
    if (datos.precio <= 0) return ERR_PRECIO_INVALIDO;
    if (datos.stock < 0 || datos.stockMinimo < 0) return ERR_STOCK_INVALIDO;

    Repuesto nuevo = datos;
    recortarEspacios(nuevo.codigo);
    aMayusculas(nuevo.codigo);

    // Codigo unico entre los activos
    Repuesto existente;
    Resultado r = obtenerRepuestoPorCodigo(nuevo.codigo, existente);
    if (r == EXITO) return ERR_DUPLICADO;
    if (r != ERR_NO_EXISTE) return r;

    return guardarRepuesto(nuevo, idAsignado);
}

Resultado obtenerRepuestoPorID(int id, Repuesto& resultado) {
    Resultado r = leerRepuestoPorID(id, resultado);
    if (r != EXITO) return r;
    return resultado.eliminado ? ERR_ELIMINADO : EXITO;
}

Resultado obtenerRepuestoPorCodigo(const char* codigo, Repuesto& resultado) {
    int total;
    Resultado r = cantidadRepuestosRegistrados(total);
    if (r != EXITO) return r;

    for (int id = 1; id <= total; id++) {
        Repuesto actual;
        r = leerRepuestoPorID(id, actual);
        if (r != EXITO) return r;
        if (actual.eliminado) continue;
        if (igualesIgnorandoMayusculas(actual.codigo, codigo)) {
            resultado = actual;
            return EXITO;
        }
    }
    return ERR_NO_EXISTE;
}

Resultado buscarRepuestosPorNombre(const char* texto, Repuesto resultados[],
                                   int& cantidad, bool& hayMas) {
    cantidad = 0;
    hayMas = false;
    if (textoVacio(texto)) return ERR_DATOS_INVALIDOS;

    int total;
    Resultado r = cantidadRepuestosRegistrados(total);
    if (r != EXITO) return r;

    for (int id = 1; id <= total; id++) {
        Repuesto actual;
        r = leerRepuestoPorID(id, actual);
        if (r != EXITO) return r;
        if (actual.eliminado) continue;
        if (!contieneIgnorandoMayusculas(actual.nombre, texto)) continue;

        if (cantidad == MAX_RESULTADOS_BUSQUEDA) {
            hayMas = true;
            break;
        }
        resultados[cantidad++] = actual;
    }
    return EXITO;
}

Resultado registrarEntradaRepuesto(int id, int cantidad) {
    if (cantidad <= 0) return ERR_CANTIDAD_INVALIDA;

    Repuesto actual;
    Resultado r = obtenerRepuestoPorID(id, actual);
    if (r != EXITO) return r;
    if (actual.stock > INT_MAX - cantidad) return ERR_CANTIDAD_INVALIDA;   // evita desbordar el int

    actual.stock += cantidad;
    return sobrescribirRepuesto(actual);
}

Resultado cambiarPrecioRepuesto(int id, float nuevoPrecio) {
    if (nuevoPrecio <= 0) return ERR_PRECIO_INVALIDO;

    Repuesto actual;
    Resultado r = obtenerRepuestoPorID(id, actual);
    if (r != EXITO) return r;

    actual.precio = nuevoPrecio;
    return sobrescribirRepuesto(actual);
}

// Recorre ordenes.bin (una orden a la vez) mirando las lineas de cada una
static Resultado repuestoEstaEnAlgunaOrden(int idRepuesto, bool& enUso) {
    enUso = false;
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_ORDENES, h);
    if (r != EXITO) return r;

    for (int id = 1; id <= h.cantidadRegistros; id++) {
        OrdenServicio orden;
        r = leerOrdenPorID(id, orden);
        if (r != EXITO) return r;
        for (int i = 0; i < orden.numLineas; i++) {
            if (orden.lineas[i].idRepuesto == idRepuesto) {
                enUso = true;
                return EXITO;
            }
        }
    }
    return EXITO;
}

Resultado eliminarRepuesto(int id) {
    Repuesto actual;
    Resultado r = obtenerRepuestoPorID(id, actual);
    if (r != EXITO) return r;

    bool enUso;
    r = repuestoEstaEnAlgunaOrden(id, enUso);
    if (r != EXITO) return r;
    if (enUso) return ERR_REPUESTO_EN_USO;

    return marcarRepuestoEliminado(id);
}
