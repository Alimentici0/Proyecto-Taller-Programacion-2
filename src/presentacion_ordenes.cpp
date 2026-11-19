// ====================================================================
// presentacion_ordenes.cpp
// Submenu "Ordenes de Servicio". Las ordenes se muestran siempre con
// nombres (vehiculo, mecanico, repuestos), nunca con IDs sueltos.
// ====================================================================
#include "presentacion.h"
#include "logica_ordenes.h"
#include "logica_vehiculos.h"
#include "logica_mecanicos.h"
#include "logica_repuestos.h"
#include "reportes.h"
#include "utilidades.h"

#include <iostream>
#include <iomanip>
#include <cstring>

using namespace std;

static void imprimirDinero(float valor) {
    cout << "$" << fixed << setprecision(2) << valor;
}

// ---- Presentacion de una orden completa ----
static void mostrarOrdenCompleta(const VistaOrden& v) {
    const OrdenServicio& o = v.orden;
    cout << "  ORDEN #" << o.id << " [" << o.estado << "]\n"
         << "  Apertura: " << o.fechaApertura;
    if (strcmp(o.estado, ESTADO_ABIERTA) != 0 && o.fechaCierre[0] != '\0') {
        cout << " | Cierre: " << o.fechaCierre;
    }
    cout << "\n  Vehiculo: " << v.placa << " - " << v.descripcionVehiculo
         << " | Propietario: " << v.propietario
         << "\n  Mecanico: " << v.nombreMecanico << (v.mecanicoDeBaja ? " [dado de baja]" : "")
         << "\n  Falla: " << o.descripcionFalla
         << "\n  Km de ingreso: " << o.kilometrajeIngreso << "\n";

    cout << "  Detalle de repuestos:\n";
    if (o.numLineas == 0) cout << "    (sin repuestos)\n";
    for (int i = 0; i < o.numLineas; i++) {
        const LineaDetalle& l = o.lineas[i];
        Repuesto rep;
        Resultado r = obtenerRepuestoPorID(l.idRepuesto, rep);
        const char* codigo = (r == EXITO || r == ERR_ELIMINADO) ? rep.codigo : "?";
        const char* nombre = (r == EXITO || r == ERR_ELIMINADO) ? rep.nombre : "(repuesto no encontrado)";
        cout << "    " << (i + 1) << ". " << codigo << " " << nombre
             << "  x" << l.cantidad << " a ";
        imprimirDinero(l.precioUnitario);
        cout << " = ";
        imprimirDinero(l.cantidad * l.precioUnitario);
        cout << "\n";
    }
    cout << "  Repuestos: "; imprimirDinero(v.totalRepuestos);
    cout << " | Mano de obra: "; imprimirDinero(o.manoObra);
    cout << " | TOTAL: "; imprimirDinero(v.total);
    cout << "\n";
}

static void mostrarOrdenEnLinea(const VistaOrden& v) {
    cout << "Orden #" << v.orden.id << " | " << v.orden.fechaApertura << " | " << v.orden.estado
         << " | " << v.placa << " " << v.descripcionVehiculo
         << " | Mec: " << v.nombreMecanico << " | Total: ";
    imprimirDinero(v.total);
    cout << "\n";
}

// Cuando una operacion falla por el estado, se informa en que estado esta la orden
static void mostrarErrorDeOrden(Resultado r, int idOrden, const char* requisito) {
    if (r == ERR_ESTADO_INCORRECTO) {
        OrdenServicio o;
        if (obtenerOrdenPorID(idOrden, o) == EXITO) {
            cout << "ERROR: la orden #" << idOrden << " esta " << o.estado << "; " << requisito << ".\n";
            return;
        }
    }
    mostrarError(r);
}

// ---- 1. Abrir ----
static void abrirOrdenUI() {
    int idVehiculo, idMecanico, km;
    char descripcion[200];
    cout << "\n--- ABRIR ORDEN DE SERVICIO ---\n";
    if (!leerEntero("ID del vehiculo: ", idVehiculo)) return;

    // Se muestra el vehiculo elegido para que el usuario confirme que es el correcto
    Vehiculo veh;
    Resultado r = obtenerVehiculoPorID(idVehiculo, veh);
    if (r == EXITO) cout << "  -> " << veh.placa << " " << veh.marca << " " << veh.modelo
                         << " (km actual: " << veh.kilometraje << ")\n";

    if (!leerEntero("ID del mecanico: ", idMecanico)) return;
    Mecanico mec;
    r = obtenerMecanicoPorID(idMecanico, mec);
    if (r == EXITO) cout << "  -> " << mec.nombre << " (" << mec.especialidad << ")\n";

    if (!leerEntero("Kilometraje de ingreso: ", km)) return;
    if (!leerLinea("Descripcion de la falla: ", descripcion, sizeof(descripcion))) return;

    int idOrden;
    r = abrirOrden(idVehiculo, idMecanico, km, descripcion, idOrden);
    if (r == EXITO) {
        VistaOrden v;
        obtenerVistaOrden(idOrden, v);
        cout << "Exito! Orden #" << idOrden << " abierta para " << v.placa << " - " << v.descripcionVehiculo
             << " con el mecanico " << v.nombreMecanico << ".\n";
    } else if (r == ERR_KILOMETRAJE_INVALIDO) {
        cout << "ERROR: el kilometraje de ingreso (" << km << ") no puede ser menor al registrado del vehiculo ("
             << veh.kilometraje << ").\n";
    } else if (r == ERR_VEHICULO_CON_ORDEN_ABIERTA) {
        cout << "ERROR: el vehiculo " << veh.placa << " ya tiene una orden ABIERTA.\n";
    } else if (r == ERR_MECANICO_LIMITE_ORDENES) {
        cout << "ERROR: " << mec.nombre << " ya tiene " << MAX_ORDENES_ABIERTAS_MECANICO
             << " ordenes ABIERTAS (limite).\n";
    } else {
        mostrarError(r);
    }
}

// ---- 2. Ver una orden ----
static void verOrdenUI() {
    int id;
    cout << "\n--- VER ORDEN ---\n";
    if (!leerEntero("ID de la orden: ", id)) return;

    VistaOrden v;
    Resultado r = obtenerVistaOrden(id, v);
    if (r == EXITO) mostrarOrdenCompleta(v);
    else if (r == ERR_ORDEN_NO_EXISTE) cout << "No existe ninguna orden con el ID " << id << ".\n";
    else mostrarError(r);
}

// ---- 3. Listar por estado ----
static void listarPorEstadoUI() {
    char estado[12];
    cout << "\n--- LISTAR ORDENES POR ESTADO ---\n";
    if (!leerLinea("Estado (ABIERTA/CERRADA/ANULADA): ", estado, sizeof(estado))) return;
    aMayusculas(estado);
    if (strcmp(estado, ESTADO_ABIERTA) != 0 && strcmp(estado, ESTADO_CERRADA) != 0 &&
        strcmp(estado, ESTADO_ANULADA) != 0) {
        cout << "Estado invalido. Use ABIERTA, CERRADA o ANULADA.\n";
        return;
    }

    int total;
    Resultado r = cantidadOrdenesRegistradas(total);
    if (r != EXITO) { mostrarError(r); return; }

    int mostradas = 0;
    for (int id = 1; id <= total; id++) {          // una orden en memoria a la vez
        VistaOrden v;
        r = obtenerVistaOrden(id, v);
        if (r != EXITO) { mostrarError(r); return; }
        if (strcmp(v.orden.estado, estado) != 0) continue;
        mostrarOrdenEnLinea(v);
        mostradas++;
    }
    if (mostradas == 0) cout << "No hay ordenes en estado " << estado << ".\n";
    else cout << mostradas << " orden(es) " << estado << ".\n";
}

// ---- 4. Historial de un vehiculo ----
void mostrarHistorialVehiculo(const char* placa) {
    Vehiculo veh;
    Resultado r = obtenerVehiculoPorPlaca(placa, veh);
    if (r == ERR_NO_EXISTE) { cout << "No hay ningun vehiculo activo con la placa " << placa << ".\n"; return; }
    if (r != EXITO) { mostrarError(r); return; }

    cout << "\nVehiculo: " << veh.placa << " - " << veh.marca << " " << veh.modelo << " (" << veh.anio
         << ") | Propietario: " << veh.nombrePropietario << "\n";

    int ids[MAX_HISTORIAL_VEHICULO];
    int cantidad;
    bool hayMas;
    r = buscarOrdenesDeVehiculo(veh.id, ids, MAX_HISTORIAL_VEHICULO, cantidad, hayMas);
    if (r != EXITO) { mostrarError(r); return; }

    if (cantidad == 0) { cout << "Este vehiculo no tiene ordenes de servicio.\n"; return; }
    for (int i = 0; i < cantidad; i++) {
        VistaOrden v;
        if (obtenerVistaOrden(ids[i], v) != EXITO) continue;
        cout << "-----\n";
        mostrarOrdenCompleta(v);
    }
    cout << "-----\n" << cantidad << " orden(es).";
    if (hayMas) cout << " (hay mas ordenes; se muestran las primeras " << MAX_HISTORIAL_VEHICULO << ")";
    cout << "\n";

    float facturado;
    if (totalFacturadoVehiculo(veh.id, facturado) == EXITO) {
        cout << "Total facturado (solo ordenes CERRADAS): ";
        imprimirDinero(facturado);
        cout << "\n";
    }
}

static void historialVehiculoUI() {
    char placa[10];
    cout << "\n--- HISTORIAL DE UN VEHICULO ---\n";
    if (!leerLinea("Placa: ", placa, sizeof(placa))) return;
    mostrarHistorialVehiculo(placa);
}

// ---- 5. Agregar repuesto ----
static void agregarRepuestoUI() {
    int idOrden, idRepuesto, cantidad;
    cout << "\n--- AGREGAR REPUESTO A UNA ORDEN ---\n";
    if (!leerEntero("ID de la orden: ", idOrden)) return;
    if (!leerEntero("ID del repuesto: ", idRepuesto)) return;

    Repuesto rep;
    Resultado r = obtenerRepuestoPorID(idRepuesto, rep);
    if (r == EXITO) {
        cout << "  -> " << rep.codigo << " " << rep.nombre << " | Stock disponible: " << rep.stock << " | Precio: ";
        imprimirDinero(rep.precio);
        cout << "\n";
    }
    if (!leerEntero("Cantidad: ", cantidad)) return;

    r = agregarRepuestoAOrden(idOrden, idRepuesto, cantidad);
    if (r == EXITO) {
        Repuesto despues;
        obtenerRepuestoPorID(idRepuesto, despues);
        cout << "Agregado: " << cantidad << " x " << despues.nombre << " a la orden #" << idOrden
             << ". Stock restante: " << despues.stock << ".\n";
    } else if (r == ERR_STOCK_INSUFICIENTE) {
        cout << "ERROR: stock insuficiente de " << rep.nombre << ": hay " << rep.stock
             << " y se pidieron " << cantidad << ".\n";
    } else if (r == ERR_LIMITE_LINEAS) {
        cout << "ERROR: la orden #" << idOrden << " ya tiene el maximo de " << MAX_LINEAS_ORDEN
             << " lineas de detalle; el repuesto NO se agrego.\n";
    } else {
        mostrarErrorDeOrden(r, idOrden, "solo se pueden agregar repuestos a ordenes ABIERTAS");
    }
}

// ---- 6. Quitar repuesto ----
static void quitarRepuestoUI() {
    int idOrden, idRepuesto;
    cout << "\n--- QUITAR REPUESTO DE UNA ORDEN ---\n";
    if (!leerEntero("ID de la orden: ", idOrden)) return;
    if (!leerEntero("ID del repuesto a quitar: ", idRepuesto)) return;

    Repuesto antes;
    bool conocido = (obtenerRepuestoPorID(idRepuesto, antes) != ERR_NO_EXISTE);
    Resultado r = quitarRepuestoDeOrden(idOrden, idRepuesto);
    if (r == EXITO) {
        Repuesto despues;
        obtenerRepuestoPorID(idRepuesto, despues);
        cout << "Linea eliminada de la orden #" << idOrden << ". Las unidades volvieron al inventario";
        if (conocido) cout << " (" << despues.nombre << ": stock " << despues.stock << ")";
        cout << ".\n";
    } else {
        mostrarErrorDeOrden(r, idOrden, "solo se pueden quitar repuestos de ordenes ABIERTAS");
    }
}

// ---- 7. Cerrar ----
static void cerrarOrdenUI() {
    int idOrden;
    float manoObra;
    cout << "\n--- CERRAR ORDEN ---\n";
    if (!leerEntero("ID de la orden: ", idOrden)) return;
    if (!leerDecimal("Mano de obra ($): ", manoObra)) return;

    Resultado r = cerrarOrden(idOrden, manoObra);
    if (r == EXITO) {
        VistaOrden v;
        obtenerVistaOrden(idOrden, v);
        cout << "Orden #" << idOrden << " CERRADA el " << v.orden.fechaCierre << ". Total a cobrar: ";
        imprimirDinero(v.total);
        cout << "\n";
    } else {
        mostrarErrorDeOrden(r, idOrden, "solo se pueden cerrar ordenes ABIERTAS");
    }
}

// ---- 8. Anular ----
static void anularOrdenUI() {
    int idOrden;
    cout << "\n--- ANULAR ORDEN ---\n";
    if (!leerEntero("ID de la orden: ", idOrden)) return;

    Resultado r = anularOrden(idOrden);
    if (r == EXITO) {
        cout << "Orden #" << idOrden << " ANULADA. Los repuestos de todas sus lineas volvieron al inventario.\n";
    } else {
        mostrarErrorDeOrden(r, idOrden, "solo se pueden anular ordenes ABIERTAS o CERRADAS");
    }
}

void menuOrdenes() {
    int opcion = -1;
    do {
        cout << "\n--- ORDENES DE SERVICIO ---\n"
             << "1. Abrir orden\n"
             << "2. Ver orden por ID\n"
             << "3. Listar ordenes por estado\n"
             << "4. Historial de un vehiculo\n"
             << "5. Agregar repuesto a una orden\n"
             << "6. Quitar repuesto de una orden\n"
             << "7. Cerrar orden\n"
             << "8. Anular orden\n"
             << "0. Volver\n";
        if (!leerEntero("Seleccione una opcion: ", opcion)) return;

        switch (opcion) {
            case 1: abrirOrdenUI(); break;
            case 2: verOrdenUI(); break;
            case 3: listarPorEstadoUI(); break;
            case 4: historialVehiculoUI(); break;
            case 5: agregarRepuestoUI(); break;
            case 6: quitarRepuestoUI(); break;
            case 7: cerrarOrdenUI(); break;
            case 8: anularOrdenUI(); break;
            case 0: break;
            default: cout << "Opcion invalida.\n";
        }
    } while (opcion != 0);
}
