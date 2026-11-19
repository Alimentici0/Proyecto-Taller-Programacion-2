#include "logica_ordenes.h"
#include "persistencia_vehiculos.h"
#include "persistencia_mecanicos.h"
#include "logica_mecanicos.h"
#include "utilidades.h"

#include <cstring>
#include <cstdio>

using namespace std;

// --------------------------------------------------------------------
// Auxiliares privados
// --------------------------------------------------------------------

// Lee una orden y traduce "no existe" al codigo propio de ordenes
static Resultado cargarOrden(int id, OrdenServicio& orden) {
    Resultado r = leerOrdenPorID(id, orden);
    return (r == ERR_NO_EXISTE) ? ERR_ORDEN_NO_EXISTE : r;
}

// Recorre ordenes.bin: ¿el vehiculo ya tiene una orden ABIERTA?
static Resultado vehiculoTieneOrdenAbierta(int idVehiculo, bool& tiene) {
    tiene = false;
    int total;
    Resultado r = cantidadOrdenesRegistradas(total);
    if (r != EXITO) return r;

    for (int id = 1; id <= total; id++) {
        OrdenServicio orden;
        r = leerOrdenPorID(id, orden);
        if (r != EXITO) return r;
        if (orden.idVehiculo == idVehiculo && strcmp(orden.estado, ESTADO_ABIERTA) == 0) {
            tiene = true;
            return EXITO;
        }
    }
    return EXITO;
}

// --------------------------------------------------------------------
// Consultas
// --------------------------------------------------------------------
Resultado cantidadOrdenesRegistradas(int& total) {
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_ORDENES, h);
    if (r != EXITO) return r;
    total = h.cantidadRegistros;
    return EXITO;
}

Resultado obtenerOrdenPorID(int id, OrdenServicio& resultado) {
    return cargarOrden(id, resultado);
}

float calcularTotalRepuestos(const OrdenServicio& orden) {
    float suma = 0.0f;
    for (int i = 0; i < orden.numLineas; i++) {
        suma += orden.lineas[i].cantidad * orden.lineas[i].precioUnitario;
    }
    return suma;
}

float calcularTotalOrden(const OrdenServicio& orden) {
    return orden.manoObra + calcularTotalRepuestos(orden);
}

Resultado obtenerVistaOrden(int id, VistaOrden& vista) {
    memset(&vista, 0, sizeof(VistaOrden));

    Resultado r = cargarOrden(id, vista.orden);
    if (r != EXITO) return r;

    // Datos del vehiculo (lectura cruda: se muestra aunque este eliminado)
    Vehiculo veh;
    if (leerVehiculoPorID(vista.orden.idVehiculo, veh) == EXITO) {
        vista.vehiculoEncontrado = true;
        copiarTexto(vista.placa, sizeof(vista.placa), veh.placa);
        snprintf(vista.descripcionVehiculo, sizeof(vista.descripcionVehiculo),
                 "%s %s (%d)", veh.marca, veh.modelo, veh.anio);
        copiarTexto(vista.propietario, sizeof(vista.propietario), veh.nombrePropietario);
    } else {
        copiarTexto(vista.placa, sizeof(vista.placa), "?");
        copiarTexto(vista.descripcionVehiculo, sizeof(vista.descripcionVehiculo), "(vehiculo no encontrado)");
    }

    // Datos del mecanico (lectura cruda: un mecanico de baja sigue mostrando su nombre)
    Mecanico mec;
    if (leerMecanicoPorID(vista.orden.idMecanico, mec) == EXITO) {
        vista.mecanicoEncontrado = true;
        copiarTexto(vista.nombreMecanico, sizeof(vista.nombreMecanico), mec.nombre);
        vista.mecanicoDeBaja = mec.eliminado;
    } else {
        copiarTexto(vista.nombreMecanico, sizeof(vista.nombreMecanico), "(mecanico no encontrado)");
    }

    vista.totalRepuestos = calcularTotalRepuestos(vista.orden);
    vista.total = calcularTotalOrden(vista.orden);
    return EXITO;
}

Resultado buscarOrdenesDeVehiculo(int idVehiculo, int ids[], int maximo,
                                  int& cantidad, bool& hayMas) {
    cantidad = 0;
    hayMas = false;
    int total;
    Resultado r = cantidadOrdenesRegistradas(total);
    if (r != EXITO) return r;

    for (int id = 1; id <= total; id++) {
        OrdenServicio orden;
        r = leerOrdenPorID(id, orden);
        if (r != EXITO) return r;
        if (orden.idVehiculo != idVehiculo) continue;

        if (cantidad == maximo) {
            hayMas = true;
            break;
        }
        ids[cantidad++] = id;
    }
    return EXITO;
}

// --------------------------------------------------------------------
// Abrir orden
// --------------------------------------------------------------------
Resultado abrirOrden(int idVehiculo, int idMecanico, int kilometrajeIngreso,
                     const char* descripcionFalla, int& idOrden) {
    // ---- VALIDACIONES (no se escribe nada hasta que todas pasen) ----
    if (textoVacio(descripcionFalla)) return ERR_DATOS_INVALIDOS;

    Vehiculo veh;
    Resultado r = leerVehiculoPorID(idVehiculo, veh);
    if (r == ERR_NO_EXISTE) return ERR_VEHICULO_NO_EXISTE;
    if (r != EXITO) return r;
    if (veh.eliminado) return ERR_VEHICULO_ELIMINADO;

    Mecanico mec;
    r = leerMecanicoPorID(idMecanico, mec);
    if (r == ERR_NO_EXISTE) return ERR_MECANICO_NO_EXISTE;
    if (r != EXITO) return r;
    if (mec.eliminado) return ERR_MECANICO_ELIMINADO;

    bool vehiculoOcupado;
    r = vehiculoTieneOrdenAbierta(idVehiculo, vehiculoOcupado);
    if (r != EXITO) return r;
    if (vehiculoOcupado) return ERR_VEHICULO_CON_ORDEN_ABIERTA;

    int abiertas;
    r = contarOrdenesAbiertasDeMecanico(idMecanico, abiertas);
    if (r != EXITO) return r;
    if (abiertas >= MAX_ORDENES_ABIERTAS_MECANICO) return ERR_MECANICO_LIMITE_ORDENES;

    if (kilometrajeIngreso < veh.kilometraje) return ERR_KILOMETRAJE_INVALIDO;

    // ---- ESCRITURAS: primero la orden, despues el kilometraje del vehiculo ----
    OrdenServicio nueva;
    memset(&nueva, 0, sizeof(OrdenServicio));      // ceros: sin basura en disco
    nueva.idVehiculo = idVehiculo;
    nueva.idMecanico = idMecanico;
    obtenerFechaActual(nueva.fechaApertura);
    copiarTexto(nueva.estado, sizeof(nueva.estado), ESTADO_ABIERTA);
    copiarTexto(nueva.descripcionFalla, sizeof(nueva.descripcionFalla), descripcionFalla);
    nueva.kilometrajeIngreso = kilometrajeIngreso;
    nueva.manoObra = 0.0f;
    nueva.numLineas = 0;

    r = guardarOrden(nueva, idOrden);              // 1) ordenes.bin
    if (r != EXITO) return r;

    veh.kilometraje = kilometrajeIngreso;          // 2) vehiculos.bin
    return sobrescribirVehiculo(veh);
}
