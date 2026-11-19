#include "logica_vehiculos.h"
#include "persistencia_ordenes.h"
#include "utilidades.h"

#include <cstring>

using namespace std;

// --------------------------------------------------------------------
// Auxiliares privados
// --------------------------------------------------------------------

// Validaciones de los campos que se pueden editar (marca, modelo, anio, propietario)
static Resultado validarCamposEditables(const Vehiculo& v) {
    if (textoVacio(v.marca) || textoVacio(v.modelo) || textoVacio(v.nombrePropietario)) {
        return ERR_DATOS_INVALIDOS;
    }
    if (v.anio < ANIO_MINIMO || v.anio > ANIO_MAXIMO) return ERR_ANIO_INVALIDO;
    return EXITO;
}

// Recorre ordenes.bin (una orden en memoria a la vez) buscando alguna de este vehiculo
static Resultado vehiculoTieneOrdenes(int idVehiculo, bool& tiene) {
    tiene = false;
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_ORDENES, h);
    if (r != EXITO) return r;

    for (int id = 1; id <= h.cantidadRegistros; id++) {
        OrdenServicio orden;
        r = leerOrdenPorID(id, orden);
        if (r != EXITO) return r;
        if (orden.idVehiculo == idVehiculo) {
            tiene = true;
            return EXITO;
        }
    }
    return EXITO;
}

// --------------------------------------------------------------------
// Registrar
// --------------------------------------------------------------------
Resultado registrarVehiculo(const Vehiculo& datos, int& idAsignado) {
    // 1. Validaciones (antes de escribir nada)
    if (textoVacio(datos.placa)) return ERR_DATOS_INVALIDOS;
    Resultado r = validarCamposEditables(datos);
    if (r != EXITO) return r;
    if (datos.kilometraje < 0) return ERR_KILOMETRAJE_INVALIDO;

    Vehiculo nuevo = datos;
    recortarEspacios(nuevo.placa);
    aMayusculas(nuevo.placa);

    // 2. Placa unica entre los vehiculos ACTIVOS
    //    (la placa de un vehiculo eliminado puede volver a registrarse)
    Vehiculo existente;
    r = obtenerVehiculoPorPlaca(nuevo.placa, existente);
    if (r == EXITO) return ERR_DUPLICADO;
    if (r != ERR_NO_EXISTE) return r;

    // 3. Guardar
    return guardarVehiculo(nuevo, idAsignado);
}

// --------------------------------------------------------------------
// Consultas
// --------------------------------------------------------------------
Resultado obtenerVehiculoPorID(int id, Vehiculo& resultado, long* posicion) {
    Resultado r = leerVehiculoPorID(id, resultado, posicion);
    if (r != EXITO) return r;
    return resultado.eliminado ? ERR_ELIMINADO : EXITO;
}

Resultado obtenerVehiculoPorPlaca(const char* placa, Vehiculo& resultado) {
    int total;
    Resultado r = cantidadVehiculosRegistrados(total);
    if (r != EXITO) return r;

    for (int id = 1; id <= total; id++) {
        Vehiculo actual;                       // un solo registro en memoria
        r = leerVehiculoPorID(id, actual);
        if (r != EXITO) return r;
        if (actual.eliminado) continue;
        if (igualesIgnorandoMayusculas(actual.placa, placa)) {
            resultado = actual;
            return EXITO;
        }
    }
    return ERR_NO_EXISTE;
}

Resultado buscarVehiculosPorPropietario(const char* texto, Vehiculo resultados[],
                                        int& cantidad, bool& hayMas) {
    cantidad = 0;
    hayMas = false;
    if (textoVacio(texto)) return ERR_DATOS_INVALIDOS;

    int total;
    Resultado r = cantidadVehiculosRegistrados(total);
    if (r != EXITO) return r;

    for (int id = 1; id <= total; id++) {
        Vehiculo actual;
        r = leerVehiculoPorID(id, actual);
        if (r != EXITO) return r;
        if (actual.eliminado) continue;
        if (!contieneIgnorandoMayusculas(actual.nombrePropietario, texto)) continue;

        if (cantidad == MAX_RESULTADOS_BUSQUEDA) {   // ya esta lleno: hay mas
            hayMas = true;
            break;
        }
        resultados[cantidad++] = actual;
    }
    return EXITO;
}

Resultado cantidadVehiculosRegistrados(int& total) {
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_VEHICULOS, h);
    if (r != EXITO) return r;
    total = h.cantidadRegistros;
    return EXITO;
}

Resultado cantidadVehiculosActivos(int& activos) {
    return contarVehiculosActivos(activos);
}

// --------------------------------------------------------------------
// Actualizar y eliminar
// --------------------------------------------------------------------
Resultado actualizarVehiculo(int id, const Vehiculo& nuevosDatos) {
    Vehiculo actual;
    Resultado r = obtenerVehiculoPorID(id, actual);
    if (r != EXITO) return r;                         // no existe o esta eliminado

    r = validarCamposEditables(nuevosDatos);
    if (r != EXITO) return r;

    // Solo se copian los campos editables; placa, km e ID se conservan
    copiarTexto(actual.marca, sizeof(actual.marca), nuevosDatos.marca);
    copiarTexto(actual.modelo, sizeof(actual.modelo), nuevosDatos.modelo);
    actual.anio = nuevosDatos.anio;
    copiarTexto(actual.nombrePropietario, sizeof(actual.nombrePropietario), nuevosDatos.nombrePropietario);
    copiarTexto(actual.cedulaPropietario, sizeof(actual.cedulaPropietario), nuevosDatos.cedulaPropietario);
    copiarTexto(actual.telefonoPropietario, sizeof(actual.telefonoPropietario), nuevosDatos.telefonoPropietario);

    return sobrescribirVehiculo(actual);
}

Resultado eliminarVehiculo(int id) {
    Vehiculo actual;
    Resultado r = obtenerVehiculoPorID(id, actual);
    if (r != EXITO) return r;

    bool tieneOrdenes;
    r = vehiculoTieneOrdenes(id, tieneOrdenes);
    if (r != EXITO) return r;
    if (tieneOrdenes) return ERR_TIENE_ORDENES;

    return marcarVehiculoEliminado(id);
}
