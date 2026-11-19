// ====================================================================
// presentacion_reportes.cpp
// Submenu "Reportes". Los datos vienen de la logica (que lee de los
// archivos); aqui solo se dibujan.
// ====================================================================
#include "presentacion.h"
#include "reportes.h"
#include "logica_mecanicos.h"

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdio>
#include <string>

using namespace std;

// Formatea un monto como "$1234.50" (para alinearlo con setw)
static string dinero(float valor) {
    char texto[40];
    snprintf(texto, sizeof(texto), "$%.2f", valor);
    return texto;
}

// Reporte 1: historial de un vehiculo + total facturado
static void reporteHistorialVehiculoUI() {
    char placa[10];
    cout << "\n=== REPORTE 1: HISTORIAL DE UN VEHICULO ===\n";
    if (!leerLinea("Placa: ", placa, sizeof(placa))) return;
    mostrarHistorialVehiculo(placa);
}

// Reporte 2: top 10 de repuestos mas usados
static void reporteRepuestosMasUsadosUI() {
    cout << "\n=== REPORTE 2: REPUESTOS MAS USADOS (ordenes CERRADAS) ===\n";
    FilaRepuestoMasUsado top[TOP_REPUESTOS];
    int cantidad;
    Resultado r = reporteRepuestosMasUsados(top, cantidad);
    if (r != EXITO) { mostrarError(r); return; }

    if (cantidad == 0) { cout << "Todavia no hay repuestos consumidos en ordenes cerradas.\n"; return; }

    cout << left << setw(5) << "Pos" << setw(14) << "Codigo" << setw(34) << "Repuesto" << "Unidades\n";
    cout << string(63, '-') << "\n";
    for (int i = 0; i < cantidad; i++) {
        cout << left << setw(5) << (i + 1) << setw(14) << top[i].codigo
             << setw(34) << top[i].nombre << top[i].unidades << "\n";
    }
}

// Reporte 3: facturacion por mecanico
static void reporteFacturacionMecanicosUI() {
    cout << "\n=== REPORTE 3: FACTURACION POR MECANICO (ordenes CERRADAS) ===\n";
    int total;
    Resultado r = cantidadMecanicosRegistrados(total);
    if (r != EXITO) { mostrarError(r); return; }
    if (total == 0) { cout << "No hay mecanicos registrados.\n"; return; }

    cout << left << setw(26) << "Mecanico" << right << setw(9) << "Ordenes"
         << setw(14) << "Mano obra" << setw(14) << "Repuestos" << setw(14) << "Total" << "\n";
    cout << string(77, '-') << "\n";

    float granManoObra = 0, granRepuestos = 0;
    int granOrdenes = 0;
    for (int id = 1; id <= total; id++) {            // un mecanico a la vez
        FilaFacturacionMecanico fila;
        r = reporteFacturacionMecanico(id, fila);
        if (r != EXITO) { mostrarError(r); return; }

        char nombre[140];
        snprintf(nombre, sizeof(nombre), "%s%s", fila.nombre, fila.deBaja ? " (baja)" : "");
        cout << left << setw(26) << nombre << right << setw(9) << fila.ordenesCerradas
             << setw(14) << dinero(fila.totalManoObra) << setw(14) << dinero(fila.totalRepuestos)
             << setw(14) << dinero(fila.totalManoObra + fila.totalRepuestos) << "\n";
        granOrdenes += fila.ordenesCerradas;
        granManoObra += fila.totalManoObra;
        granRepuestos += fila.totalRepuestos;
    }
    cout << string(77, '-') << "\n";
    cout << left << setw(26) << "TOTAL TALLER" << right << setw(9) << granOrdenes
         << setw(14) << dinero(granManoObra) << setw(14) << dinero(granRepuestos)
         << setw(14) << dinero(granManoObra + granRepuestos) << "\n";
}

void menuReportes() {
    int opcion = -1;
    do {
        cout << "\n--- REPORTES ---\n"
             << "1. Historial de un vehiculo\n"
             << "2. Repuestos mas usados (top 10)\n"
             << "3. Facturacion por mecanico\n"
             << "0. Volver\n";
        if (!leerEntero("Seleccione una opcion: ", opcion)) return;

        switch (opcion) {
            case 1: reporteHistorialVehiculoUI(); break;
            case 2: reporteRepuestosMasUsadosUI(); break;
            case 3: reporteFacturacionMecanicosUI(); break;
            case 0: break;
            default: cout << "Opcion invalida.\n";
        }
    } while (opcion != 0);
}
