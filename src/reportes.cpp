#include "reportes.h"
#include "persistencia_mecanicos.h"
#include "persistencia_repuestos.h"
#include "persistencia_ordenes.h"
#include "logica_ordenes.h"
#include "utilidades.h"

#include <cstring>

using namespace std;

Resultado totalFacturadoVehiculo(int idVehiculo, float& total) {
    total = 0.0f;
    int cantidadOrdenes;
    Resultado r = cantidadOrdenesRegistradas(cantidadOrdenes);
    if (r != EXITO) return r;

    for (int id = 1; id <= cantidadOrdenes; id++) {
        OrdenServicio orden;                          // una orden en memoria a la vez
        r = leerOrdenPorID(id, orden);
        if (r != EXITO) return r;
        if (orden.idVehiculo == idVehiculo && strcmp(orden.estado, ESTADO_CERRADA) == 0) {
            total += calcularTotalOrden(orden);
        }
    }
    return EXITO;
}

// Unidades de un repuesto consumidas en ordenes CERRADAS
static Resultado unidadesConsumidas(int idRepuesto, int cantidadOrdenes, int& unidades) {
    unidades = 0;
    for (int id = 1; id <= cantidadOrdenes; id++) {
        OrdenServicio orden;
        Resultado r = leerOrdenPorID(id, orden);
        if (r != EXITO) return r;
        if (strcmp(orden.estado, ESTADO_CERRADA) != 0) continue;
        for (int i = 0; i < orden.numLineas; i++) {
            if (orden.lineas[i].idRepuesto == idRepuesto) unidades += orden.lineas[i].cantidad;
        }
    }
    return EXITO;
}

// Para no cargar nada en memoria salvo 10 filas: se calcula el consumo de
// UN repuesto a la vez y se inserta en el "top 10" ordenado si le alcanza.
Resultado reporteRepuestosMasUsados(FilaRepuestoMasUsado top[], int& cantidad) {
    cantidad = 0;
    ArchivoHeader h;
    Resultado r = leerHeader(RUTA_REPUESTOS, h);
    if (r != EXITO) return r;
    int cantidadOrdenes;
    r = cantidadOrdenesRegistradas(cantidadOrdenes);
    if (r != EXITO) return r;

    for (int id = 1; id <= h.cantidadRegistros; id++) {
        Repuesto rep;
        r = leerRepuestoPorID(id, rep);
        if (r != EXITO) return r;

        int unidades;
        r = unidadesConsumidas(id, cantidadOrdenes, unidades);
        if (r != EXITO) return r;
        if (unidades <= 0) continue;

        // Posicion: despues de todos los que tienen >= unidades (asi el ID menor gana los empates)
        int pos = 0;
        while (pos < cantidad && top[pos].unidades >= unidades) pos++;
        if (pos >= TOP_REPUESTOS) continue;           // no entra en el top

        int ultimo = (cantidad < TOP_REPUESTOS) ? cantidad : TOP_REPUESTOS - 1;
        for (int i = ultimo; i > pos; i--) top[i] = top[i - 1];

        top[pos].idRepuesto = id;
        copiarTexto(top[pos].codigo, sizeof(top[pos].codigo), rep.codigo);
        copiarTexto(top[pos].nombre, sizeof(top[pos].nombre), rep.nombre);
        top[pos].unidades = unidades;
        if (cantidad < TOP_REPUESTOS) cantidad++;
    }
    return EXITO;
}

Resultado reporteFacturacionMecanico(int idMecanico, FilaFacturacionMecanico& fila) {
    memset(&fila, 0, sizeof(FilaFacturacionMecanico));

    Mecanico mec;
    Resultado r = leerMecanicoPorID(idMecanico, mec);   // lectura cruda: incluye los de baja
    if (r != EXITO) return r;
    fila.idMecanico = idMecanico;
    copiarTexto(fila.nombre, sizeof(fila.nombre), mec.nombre);
    fila.deBaja = mec.eliminado;

    int cantidadOrdenes;
    r = cantidadOrdenesRegistradas(cantidadOrdenes);
    if (r != EXITO) return r;

    for (int id = 1; id <= cantidadOrdenes; id++) {
        OrdenServicio orden;
        r = leerOrdenPorID(id, orden);
        if (r != EXITO) return r;
        if (orden.idMecanico != idMecanico || strcmp(orden.estado, ESTADO_CERRADA) != 0) continue;

        fila.ordenesCerradas++;
        fila.totalManoObra += orden.manoObra;
        fila.totalRepuestos += calcularTotalRepuestos(orden);
    }
    return EXITO;
}
