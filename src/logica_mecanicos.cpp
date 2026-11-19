#include "logica_mecanicos.h"
#include "persistencia_ordenes.h"
#include "utilidades.h"

#include <cstring>

using namespace std;

static const char* const ESPECIALIDADES[] = { "MOTOR", "FRENOS", "ELECTRICIDAD", "SUSPENSION", "GENERAL" };
static const int CANTIDAD_ESPECIALIDADES = 5;

bool especialidadValida(const char* especialidad) {
    for (int i = 0; i < CANTIDAD_ESPECIALIDADES; i++) {
        if (igualesIgnorandoMayusculas(especialidad, ESPECIALIDADES[i])) return true;
    }
    return false;
}

Resultado cantidadMecanicosRegistrados(int& total) {
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_MECANICOS, h);
    if (r != EXITO) return r;
    total = h.cantidadRegistros;
    return EXITO;
}

Resultado cantidadMecanicosActivos(int& activos) {
    return contarMecanicosActivos(activos);
}

// Recorre mecanicos.bin buscando una cedula (activos y de baja)
static Resultado cedulaYaRegistrada(const char* cedula, bool& existe) {
    existe = false;
    int total;
    Resultado r = cantidadMecanicosRegistrados(total);
    if (r != EXITO) return r;

    for (int id = 1; id <= total; id++) {
        Mecanico actual;
        r = leerMecanicoPorID(id, actual);
        if (r != EXITO) return r;
        if (igualesIgnorandoMayusculas(actual.cedula, cedula)) {
            existe = true;
            return EXITO;
        }
    }
    return EXITO;
}

Resultado registrarMecanico(const Mecanico& datos, int& idAsignado) {
    // Validaciones antes de escribir
    if (textoVacio(datos.nombre) || textoVacio(datos.cedula)) return ERR_DATOS_INVALIDOS;
    if (!especialidadValida(datos.especialidad)) return ERR_ESPECIALIDAD_INVALIDA;
    if (!fechaValida(datos.fechaIngreso)) return ERR_FECHA_INVALIDA;

    Mecanico nuevo = datos;
    recortarEspacios(nuevo.cedula);
    aMayusculas(nuevo.especialidad);

    bool duplicada;
    Resultado r = cedulaYaRegistrada(nuevo.cedula, duplicada);
    if (r != EXITO) return r;
    if (duplicada) return ERR_DUPLICADO;

    return guardarMecanico(nuevo, idAsignado);
}

Resultado obtenerMecanicoPorID(int id, Mecanico& resultado) {
    Resultado r = leerMecanicoPorID(id, resultado);
    if (r != EXITO) return r;
    return resultado.eliminado ? ERR_ELIMINADO : EXITO;
}

Resultado contarOrdenesAbiertasDeMecanico(int idMecanico, int& cantidad) {
    cantidad = 0;
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_ORDENES, h);
    if (r != EXITO) return r;

    for (int id = 1; id <= h.cantidadRegistros; id++) {
        OrdenServicio orden;                       // una sola orden en memoria
        r = leerOrdenPorID(id, orden);
        if (r != EXITO) return r;
        if (orden.idMecanico == idMecanico && strcmp(orden.estado, ESTADO_ABIERTA) == 0) {
            cantidad++;
        }
    }
    return EXITO;
}

Resultado darDeBajaMecanico(int id) {
    Mecanico actual;
    Resultado r = obtenerMecanicoPorID(id, actual);
    if (r != EXITO) return r;                      // no existe o ya esta de baja

    int abiertas;
    r = contarOrdenesAbiertasDeMecanico(id, abiertas);
    if (r != EXITO) return r;
    if (abiertas > 0) return ERR_TIENE_ORDENES_ABIERTAS;

    // Borrado logico: el registro NO se mueve ni se borra, asi las
    // ordenes CERRADAS y ANULADAS siguen encontrando su nombre.
    return marcarMecanicoEliminado(id);
}
