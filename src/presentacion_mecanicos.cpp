// ====================================================================
// presentacion_mecanicos.cpp
// Submenu "Gestion de Mecanicos".
// ====================================================================
#include "presentacion.h"
#include "logica_mecanicos.h"

#include <iostream>
#include <cstring>

using namespace std;

static void mostrarMecanico(const Mecanico& m, bool deBaja) {
    cout << "  ID: " << m.id << " | " << m.nombre << (deBaja ? "  [DADO DE BAJA]" : "") << "\n"
         << "  Cedula: " << m.cedula << " | Especialidad: " << m.especialidad
         << " | Ingreso: " << m.fechaIngreso << "\n";
}

// ---- 1. Registrar ----
static void registrarMecanicoUI() {
    Mecanico m;
    memset(&m, 0, sizeof(Mecanico));
    cout << "\n--- REGISTRAR NUEVO MECANICO ---\n";
    if (!leerLinea("Nombre: ", m.nombre, sizeof(m.nombre))) return;
    if (!leerLinea("Cedula: ", m.cedula, sizeof(m.cedula))) return;
    if (!leerLinea("Especialidad (MOTOR/FRENOS/ELECTRICIDAD/SUSPENSION/GENERAL): ",
                   m.especialidad, sizeof(m.especialidad))) return;
    if (!leerLinea("Fecha de ingreso (AAAA-MM-DD): ", m.fechaIngreso, sizeof(m.fechaIngreso))) return;

    int id;
    Resultado r = registrarMecanico(m, id);
    if (r == EXITO) cout << "Exito! Mecanico " << m.nombre << " registrado con el ID #" << id << ".\n";
    else if (r == ERR_DUPLICADO) cout << "ERROR: ya existe un mecanico con la cedula " << m.cedula << ".\n";
    else mostrarError(r);
}

// ---- 2. Listar (activos y dados de baja, marcados) ----
static void listarMecanicosUI() {
    int total;
    Resultado r = cantidadMecanicosRegistrados(total);
    if (r != EXITO) { mostrarError(r); return; }

    cout << "\n--- LISTA DE MECANICOS ---\n";
    int mostrados = 0;
    for (int id = 1; id <= total; id++) {
        Mecanico m;
        r = obtenerMecanicoPorID(id, m);
        if (r != EXITO && r != ERR_ELIMINADO) { mostrarError(r); return; }

        cout << "ID: " << m.id << " | " << m.nombre << " | Cedula: " << m.cedula
             << " | " << m.especialidad << " | Ingreso: " << m.fechaIngreso;
        if (r == ERR_ELIMINADO) {
            cout << " | [DADO DE BAJA]";
        } else {
            int abiertas;
            if (contarOrdenesAbiertasDeMecanico(id, abiertas) == EXITO) {
                cout << " | Ordenes abiertas: " << abiertas;
            }
        }
        cout << "\n";
        mostrados++;
    }
    if (mostrados == 0) cout << "No hay mecanicos registrados.\n";
}

// ---- 3. Buscar por ID ----
static void buscarMecanicoPorIdUI() {
    int id;
    cout << "\n--- BUSCAR MECANICO POR ID ---\n";
    if (!leerEntero("ID del mecanico: ", id)) return;

    Mecanico m;
    Resultado r = obtenerMecanicoPorID(id, m);
    if (r == EXITO) mostrarMecanico(m, false);
    else if (r == ERR_ELIMINADO) mostrarMecanico(m, true);
    else if (r == ERR_NO_EXISTE) cout << "No existe ningun mecanico con el ID " << id << ".\n";
    else mostrarError(r);
}

// ---- 4. Dar de baja ----
static void darDeBajaMecanicoUI() {
    int id;
    cout << "\n--- DAR DE BAJA A UN MECANICO ---\n";
    if (!leerEntero("ID del mecanico: ", id)) return;

    Mecanico m;
    Resultado r = obtenerMecanicoPorID(id, m);
    if (r == ERR_ELIMINADO) { cout << "El mecanico " << m.nombre << " ya estaba dado de baja.\n"; return; }
    if (r != EXITO) { mostrarError(r); return; }

    r = darDeBajaMecanico(id);
    if (r == EXITO) {
        cout << "Mecanico " << m.nombre << " dado de baja. Su historial de ordenes se conserva.\n";
    } else if (r == ERR_TIENE_ORDENES_ABIERTAS) {
        int abiertas = 0;
        contarOrdenesAbiertasDeMecanico(id, abiertas);
        cout << "ERROR: no se puede dar de baja a " << m.nombre << ": tiene " << abiertas
             << " orden(es) ABIERTA(S). Cierrelas o anulelas primero.\n";
    } else {
        mostrarError(r);
    }
}

void menuMecanicos() {
    int opcion = -1;
    do {
        cout << "\n--- GESTION DE MECANICOS ---\n"
             << "1. Registrar mecanico\n"
             << "2. Listar mecanicos\n"
             << "3. Buscar por ID\n"
             << "4. Dar de baja\n"
             << "0. Volver\n";
        if (!leerEntero("Seleccione una opcion: ", opcion)) return;

        switch (opcion) {
            case 1: registrarMecanicoUI(); break;
            case 2: listarMecanicosUI(); break;
            case 3: buscarMecanicoPorIdUI(); break;
            case 4: darDeBajaMecanicoUI(); break;
            case 0: break;
            default: cout << "Opcion invalida.\n";
        }
    } while (opcion != 0);
}
