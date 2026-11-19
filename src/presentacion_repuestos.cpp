// ====================================================================
// presentacion_repuestos.cpp
// Submenu "Inventario de Repuestos".
// ====================================================================
#include "presentacion.h"
#include "logica_repuestos.h"

#include <iostream>
#include <iomanip>
#include <cstring>

using namespace std;

static void mostrarRepuestoEnLinea(const Repuesto& r) {
    cout << "ID: " << r.id << " | " << r.codigo << " | " << r.nombre;
    if (r.marca[0] != '\0') cout << " (" << r.marca << ")";
    cout << " | $" << fixed << setprecision(2) << r.precio
         << " | Stock: " << r.stock << " (min " << r.stockMinimo << ")";
    if (repuestoBajoStock(r)) cout << "  ** BAJO STOCK **";
    cout << "\n";
}

// ---- 1. Registrar ----
static void registrarRepuestoUI() {
    Repuesto r;
    memset(&r, 0, sizeof(Repuesto));
    cout << "\n--- REGISTRAR NUEVO REPUESTO ---\n";
    if (!leerLinea("Codigo: ", r.codigo, sizeof(r.codigo))) return;
    if (!leerLinea("Nombre: ", r.nombre, sizeof(r.nombre))) return;
    if (!leerLinea("Marca (opcional): ", r.marca, sizeof(r.marca))) return;
    if (!leerDecimal("Precio unitario: ", r.precio)) return;
    if (!leerEntero("Stock inicial: ", r.stock)) return;
    if (!leerEntero("Stock minimo: ", r.stockMinimo)) return;

    int id;
    Resultado res = registrarRepuesto(r, id);
    if (res == EXITO) cout << "Exito! Repuesto " << r.nombre << " registrado con el ID #" << id << ".\n";
    else if (res == ERR_DUPLICADO) cout << "ERROR: ya existe un repuesto activo con el codigo " << r.codigo << ".\n";
    else mostrarError(res);
}

// ---- 2. Listar (activos) ----
static void listarRepuestosUI() {
    int total;
    Resultado res = cantidadRepuestosRegistrados(total);
    if (res != EXITO) { mostrarError(res); return; }

    cout << "\n--- INVENTARIO DE REPUESTOS ---\n";
    int mostrados = 0;
    for (int id = 1; id <= total; id++) {
        Repuesto r;
        res = obtenerRepuestoPorID(id, r);
        if (res == ERR_ELIMINADO) continue;
        if (res != EXITO) { mostrarError(res); return; }
        mostrarRepuestoEnLinea(r);
        mostrados++;
    }
    if (mostrados == 0) cout << "No hay repuestos registrados.\n";
}

// ---- 3. Buscar por ID ----
static void buscarRepuestoPorIdUI() {
    int id;
    cout << "\n--- BUSCAR REPUESTO POR ID ---\n";
    if (!leerEntero("ID del repuesto: ", id)) return;

    Repuesto r;
    Resultado res = obtenerRepuestoPorID(id, r);
    if (res == EXITO) mostrarRepuestoEnLinea(r);
    else if (res == ERR_ELIMINADO) cout << "El repuesto con ID " << id << " (" << r.codigo << " - " << r.nombre << ") fue ELIMINADO.\n";
    else if (res == ERR_NO_EXISTE) cout << "No existe ningun repuesto con el ID " << id << ".\n";
    else mostrarError(res);
}

// ---- 4. Buscar por codigo ----
static void buscarRepuestoPorCodigoUI() {
    char codigo[20];
    cout << "\n--- BUSCAR REPUESTO POR CODIGO ---\n";
    if (!leerLinea("Codigo exacto: ", codigo, sizeof(codigo))) return;

    Repuesto r;
    Resultado res = obtenerRepuestoPorCodigo(codigo, r);
    if (res == EXITO) mostrarRepuestoEnLinea(r);
    else if (res == ERR_NO_EXISTE) cout << "No hay ningun repuesto activo con el codigo " << codigo << ".\n";
    else mostrarError(res);
}

// ---- 5. Buscar por nombre (parcial) ----
static void buscarRepuestoPorNombreUI() {
    char texto[100];
    cout << "\n--- BUSCAR REPUESTOS POR NOMBRE ---\n";
    if (!leerLinea("Nombre (o parte): ", texto, sizeof(texto))) return;

    Repuesto resultados[MAX_RESULTADOS_BUSQUEDA];
    int cantidad;
    bool hayMas;
    Resultado res = buscarRepuestosPorNombre(texto, resultados, cantidad, hayMas);
    if (res != EXITO) { mostrarError(res); return; }

    if (cantidad == 0) { cout << "No se encontraron repuestos con ese nombre.\n"; return; }
    for (int i = 0; i < cantidad; i++) mostrarRepuestoEnLinea(resultados[i]);
    cout << cantidad << " resultado(s).\n";
    if (hayMas) {
        cout << "AVISO: hay mas coincidencias; solo se muestran las primeras " << MAX_RESULTADOS_BUSQUEDA
             << ". Escriba una busqueda mas especifica.\n";
    }
}

// ---- 6. Entrada de mercancia ----
static void entradaRepuestoUI() {
    int id, cantidad;
    cout << "\n--- ENTRADA DE MERCANCIA ---\n";
    if (!leerEntero("ID del repuesto: ", id)) return;
    if (!leerEntero("Cantidad que ingresa: ", cantidad)) return;

    Resultado res = registrarEntradaRepuesto(id, cantidad);
    if (res != EXITO) { mostrarError(res); return; }

    Repuesto r;
    if (obtenerRepuestoPorID(id, r) == EXITO) {
        cout << "Entrada registrada. " << r.nombre << " ahora tiene " << r.stock << " unidades.\n";
    }
}

// ---- 7. Cambiar precio ----
static void cambiarPrecioUI() {
    int id;
    float nuevoPrecio;
    cout << "\n--- CAMBIAR PRECIO ---\n";
    if (!leerEntero("ID del repuesto: ", id)) return;
    if (!leerDecimal("Nuevo precio: ", nuevoPrecio)) return;

    Resultado res = cambiarPrecioRepuesto(id, nuevoPrecio);
    if (res == EXITO) cout << "Precio actualizado. Las ordenes ya registradas conservan el precio con que se agrego cada linea.\n";
    else mostrarError(res);
}

// ---- 8. Eliminar ----
static void eliminarRepuestoUI() {
    int id;
    cout << "\n--- ELIMINAR REPUESTO ---\n";
    if (!leerEntero("ID del repuesto: ", id)) return;

    Repuesto r;
    Resultado res = obtenerRepuestoPorID(id, r);
    if (res == ERR_ELIMINADO) { cout << "El repuesto con ID " << id << " ya estaba eliminado.\n"; return; }
    if (res != EXITO) { mostrarError(res); return; }

    res = eliminarRepuesto(id);
    if (res == EXITO) cout << "Repuesto " << r.codigo << " - " << r.nombre << " eliminado.\n";
    else if (res == ERR_REPUESTO_EN_USO) {
        cout << "ERROR: no se puede eliminar " << r.codigo << " - " << r.nombre
             << ": aparece en el detalle de alguna orden de servicio.\n";
    } else mostrarError(res);
}

void menuRepuestos() {
    int opcion = -1;
    do {
        cout << "\n--- INVENTARIO DE REPUESTOS ---\n"
             << "1. Registrar repuesto\n"
             << "2. Listar repuestos\n"
             << "3. Buscar por ID\n"
             << "4. Buscar por codigo\n"
             << "5. Buscar por nombre\n"
             << "6. Entrada de mercancia\n"
             << "7. Cambiar precio\n"
             << "8. Eliminar repuesto\n"
             << "0. Volver\n";
        if (!leerEntero("Seleccione una opcion: ", opcion)) return;

        switch (opcion) {
            case 1: registrarRepuestoUI(); break;
            case 2: listarRepuestosUI(); break;
            case 3: buscarRepuestoPorIdUI(); break;
            case 4: buscarRepuestoPorCodigoUI(); break;
            case 5: buscarRepuestoPorNombreUI(); break;
            case 6: entradaRepuestoUI(); break;
            case 7: cambiarPrecioUI(); break;
            case 8: eliminarRepuestoUI(); break;
            case 0: break;
            default: cout << "Opcion invalida.\n";
        }
    } while (opcion != 0);
}
