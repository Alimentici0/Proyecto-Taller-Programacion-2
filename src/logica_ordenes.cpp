#include "logica_ordenes.h"
#include "persistencia_vehiculos.h"
#include "persistencia_mecanicos.h"
#include "persistencia_repuestos.h"
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

// --------------------------------------------------------------------
// Agregar repuesto a una orden
//
// ORDEN DE ESCRITURA: 1) repuestos.bin (se descuenta el stock)
//                     2) ordenes.bin   (se agrega la linea)
//
// Regla que seguimos en TODAS las operaciones: el inventario nunca debe
// SOBREESTIMAR lo que hay. Por eso el stock se descuenta ANTES de que la
// orden reclame las unidades, y (en quitar / anular) se devuelve DESPUES
// de que la orden las libero. Si el programa se interrumpe entre las dos
// escrituras, el stock queda de MENOS (unidades "perdidas" en el sistema
// pero presentes en el estante), nunca de MAS. Ese error es conservador,
// no se le cobra de mas al cliente aunque se reintente, y se corrige con
// "Entrada de mercancia". Ver README_V2.md, pregunta 2.
// --------------------------------------------------------------------
Resultado agregarRepuestoAOrden(int idOrden, int idRepuesto, int cantidad) {
    // ---- VALIDACIONES ----
    if (cantidad <= 0) return ERR_CANTIDAD_INVALIDA;

    OrdenServicio orden;
    Resultado r = cargarOrden(idOrden, orden);
    if (r != EXITO) return r;
    if (strcmp(orden.estado, ESTADO_ABIERTA) != 0) return ERR_ESTADO_INCORRECTO;

    Repuesto rep;
    r = leerRepuestoPorID(idRepuesto, rep);
    if (r == ERR_NO_EXISTE) return ERR_REPUESTO_NO_EXISTE;
    if (r != EXITO) return r;
    if (rep.eliminado) return ERR_REPUESTO_ELIMINADO;
    if (rep.stock < cantidad) return ERR_STOCK_INSUFICIENTE;

    // ¿El repuesto ya esta en la orden? Entonces se acumula en su linea
    int posicion = -1;
    for (int i = 0; i < orden.numLineas; i++) {
        if (orden.lineas[i].idRepuesto == idRepuesto) {
            posicion = i;
            break;
        }
    }

    if (posicion >= 0) {
        orden.lineas[posicion].cantidad += cantidad;      // conserva su precio historico
    } else {
        if (orden.numLineas >= MAX_LINEAS_ORDEN) return ERR_LIMITE_LINEAS;
        LineaDetalle nueva;
        nueva.idRepuesto = idRepuesto;
        nueva.cantidad = cantidad;
        nueva.precioUnitario = rep.precio;                // precio historico = vigente hoy
        orden.lineas[orden.numLineas] = nueva;
        orden.numLineas++;
    }

    // ---- ESCRITURAS ----
    rep.stock -= cantidad;                                // 1) repuestos.bin
    r = sobrescribirRepuesto(rep);
    if (r != EXITO) return r;

    return sobrescribirOrden(orden);                      // 2) ordenes.bin
}

// --------------------------------------------------------------------
// Quitar repuesto de una orden
// ORDEN DE ESCRITURA: 1) ordenes.bin (la orden suelta las unidades)
//                     2) repuestos.bin (el stock las recupera)
// Es el orden INVERSO a agregar, por la misma regla: el stock nunca
// sobreestima. Una interrupcion deja el stock de menos, no de mas.
// --------------------------------------------------------------------
Resultado quitarRepuestoDeOrden(int idOrden, int idRepuesto) {
    // ---- VALIDACIONES ----
    OrdenServicio orden;
    Resultado r = cargarOrden(idOrden, orden);
    if (r != EXITO) return r;
    if (strcmp(orden.estado, ESTADO_ABIERTA) != 0) return ERR_ESTADO_INCORRECTO;

    int posicion = -1;
    for (int i = 0; i < orden.numLineas; i++) {
        if (orden.lineas[i].idRepuesto == idRepuesto) {
            posicion = i;
            break;
        }
    }
    if (posicion < 0) return ERR_LINEA_NO_EXISTE;

    // El repuesto debe poder leerse ANTES de tocar la orden
    Repuesto rep;
    r = leerRepuestoPorID(idRepuesto, rep);
    if (r == ERR_NO_EXISTE) return ERR_REPUESTO_NO_EXISTE;
    if (r != EXITO) return r;

    int unidadesADevolver = orden.lineas[posicion].cantidad;

    // Quitar la linea: las siguientes se corren una posicion hacia arriba
    // y la ultima queda en ceros (el arreglo es fijo, no se achica).
    for (int i = posicion; i < orden.numLineas - 1; i++) {
        orden.lineas[i] = orden.lineas[i + 1];
    }
    memset(&orden.lineas[orden.numLineas - 1], 0, sizeof(LineaDetalle));
    orden.numLineas--;

    // ---- ESCRITURAS ----
    r = sobrescribirOrden(orden);                         // 1) ordenes.bin
    if (r != EXITO) return r;

    rep.stock += unidadesADevolver;                       // 2) repuestos.bin
    return sobrescribirRepuesto(rep);
}

// --------------------------------------------------------------------
// Cerrar orden (solo escribe ordenes.bin)
// --------------------------------------------------------------------
Resultado cerrarOrden(int idOrden, float manoObra) {
    OrdenServicio orden;
    Resultado r = cargarOrden(idOrden, orden);
    if (r != EXITO) return r;
    if (strcmp(orden.estado, ESTADO_ABIERTA) != 0) return ERR_ESTADO_INCORRECTO;
    if (manoObra < 0) return ERR_MANO_OBRA_INVALIDA;
    if (orden.numLineas == 0 && manoObra <= 0) return ERR_ORDEN_VACIA;

    orden.manoObra = manoObra;
    obtenerFechaActual(orden.fechaCierre);
    copiarTexto(orden.estado, sizeof(orden.estado), ESTADO_CERRADA);
    return sobrescribirOrden(orden);
}

// --------------------------------------------------------------------
// Anular orden
// ORDEN DE ESCRITURA: 1) ordenes.bin (estado ANULADA)  2) repuestos.bin
// (un registro por cada linea). Todos los repuestos se leen y se
// validan ANTES de la primera escritura.
// --------------------------------------------------------------------
Resultado anularOrden(int idOrden) {
    // ---- VALIDACIONES ----
    OrdenServicio orden;
    Resultado r = cargarOrden(idOrden, orden);
    if (r != EXITO) return r;
    if (strcmp(orden.estado, ESTADO_ANULADA) == 0) return ERR_ESTADO_INCORRECTO;
    // (ABIERTA y CERRADA se pueden anular)

    for (int i = 0; i < orden.numLineas; i++) {
        Repuesto rep;
        r = leerRepuestoPorID(orden.lineas[i].idRepuesto, rep);
        if (r == ERR_NO_EXISTE) return ERR_REPUESTO_NO_EXISTE;
        if (r != EXITO) return r;
    }

    // ---- ESCRITURAS ----
    copiarTexto(orden.estado, sizeof(orden.estado), ESTADO_ANULADA);
    r = sobrescribirOrden(orden);                         // 1) ordenes.bin
    if (r != EXITO) return r;

    for (int i = 0; i < orden.numLineas; i++) {           // 2) repuestos.bin
        Repuesto rep;
        r = leerRepuestoPorID(orden.lineas[i].idRepuesto, rep);
        if (r != EXITO) return r;
        rep.stock += orden.lineas[i].cantidad;
        r = sobrescribirRepuesto(rep);
        if (r != EXITO) return r;
    }
    return EXITO;
}
