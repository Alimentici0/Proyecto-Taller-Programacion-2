// ====================================================================
// presentacion_menu.cpp
// Arranque (primer uso: datos del taller) y menu principal.
// ====================================================================
#include "presentacion.h"
#include "logica_taller.h"

#include <iostream>
#include <cstring>
#include <cstdio>

using namespace std;

// Primer arranque: pide los datos del taller y crea taller.bin
bool pedirYRegistrarTaller() {
    Taller t;
    memset(&t, 0, sizeof(Taller));

    cout << "\n=== PRIMER ARRANQUE: DATOS DEL TALLER ===\n";
    while (true) {
        if (!leerLinea("Nombre del taller: ", t.nombre, sizeof(t.nombre))) return false;
        if (!leerLinea("RIF: ", t.rif, sizeof(t.rif))) return false;
        if (!leerLinea("Direccion: ", t.direccion, sizeof(t.direccion))) return false;
        if (!leerLinea("Telefono: ", t.telefono, sizeof(t.telefono))) return false;

        Resultado r = registrarTaller(t);
        if (r == EXITO) {
            cout << "Taller registrado correctamente.\n";
            return true;
        }
        mostrarError(r);
        if (r == ERR_ARCHIVO) return false;
    }
}

void ejecutarMenuPrincipal(const char* nombreTaller) {
    int opcion = -1;
    do {
        char linea[80];
        snprintf(linea, sizeof(linea), "   Taller: %s", nombreTaller);

        cout << "\n";
        imprimirBordeCaja('=');
        imprimirLineaCaja("   SISTEMA DE GESTION DE TALLER");
        imprimirLineaCaja(linea);
        imprimirBordeCaja('=');
        imprimirLineaCaja("1. Gestion de Vehiculos");
        imprimirLineaCaja("2. Gestion de Mecanicos");
        imprimirLineaCaja("3. Inventario de Repuestos");
        imprimirLineaCaja("4. Ordenes de Servicio");
        imprimirLineaCaja("5. Reportes");
        imprimirLineaCaja("6. Mantenimiento");
        imprimirLineaCaja("0. Salir");
        imprimirBordeCaja('=');

        if (!leerEntero("Seleccione una opcion: ", opcion)) return;     // fin de la entrada

        switch (opcion) {
            case 1: menuVehiculos(); break;
            case 2: menuMecanicos(); break;
            case 3: menuRepuestos(); break;
            case 4: menuOrdenes(); break;
            case 5: cout << "(Reportes: pendiente)\n"; break;
            case 6: cout << "(Mantenimiento: pendiente)\n"; break;
            case 0: cout << "Hasta pronto. Los datos quedaron guardados en la carpeta datos/.\n"; break;
            default: cout << "Opcion invalida.\n";
        }
    } while (opcion != 0);
}
