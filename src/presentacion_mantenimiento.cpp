// ====================================================================
// presentacion_mantenimiento.cpp
// Submenu "Mantenimiento": verificar integridad y crear backup.
// ====================================================================
#include "presentacion.h"
#include "mantenimiento.h"

#include <iostream>
#include <cstdio>

using namespace std;

static void imprimirContador(const char* etiqueta, int valor) {
    char linea[60];
    snprintf(linea, sizeof(linea), "%s %d", etiqueta, valor);
    imprimirLineaCaja(linea);
}

static void verificarIntegridadUI() {
    cout << "\n--- VERIFICACION DE INTEGRIDAD ---\n";
    ReporteIntegridad rep;
    Resultado r = verificarIntegridad(rep);
    if (r != EXITO) { mostrarError(r); return; }

    for (int i = 0; i < rep.mensajesGuardados; i++) cout << rep.mensajes[i] << "\n";
    if (rep.totalProblemas > rep.mensajesGuardados) {
        cout << "... y " << (rep.totalProblemas - rep.mensajesGuardados) << " problema(s) mas no listados.\n";
    }
    if (rep.totalProblemas == 0) cout << "No se encontraron problemas.\n";

    cout << "\n";
    imprimirBordeCaja('=');
    imprimirLineaCaja("   VERIFICACION DE INTEGRIDAD");
    imprimirBordeCaja('=');
    imprimirContador("Vehiculos verificados:  ", rep.vehiculosVerificados);
    imprimirContador("Mecanicos verificados:  ", rep.mecanicosVerificados);
    imprimirContador("Repuestos verificados:  ", rep.repuestosVerificados);
    imprimirContador("Ordenes verificadas:    ", rep.ordenesVerificadas);
    imprimirContador("Problemas encontrados:  ", rep.totalProblemas);
    imprimirBordeCaja('=');
}

static void crearBackupUI() {
    cout << "\n--- CREAR BACKUP ---\n";
    char ruta[100];
    Resultado r = crearBackup(ruta, sizeof(ruta));
    if (r == EXITO) cout << "Backup creado en la carpeta " << ruta << "/ (copia de todos los archivos .bin).\n";
    else { mostrarError(r); cout << "El backup quedo incompleto en " << ruta << "/.\n"; }
}

void menuMantenimiento() {
    int opcion = -1;
    do {
        cout << "\n--- MANTENIMIENTO ---\n"
             << "1. Verificar integridad de los datos\n"
             << "2. Crear backup\n"
             << "0. Volver\n";
        if (!leerEntero("Seleccione una opcion: ", opcion)) return;

        switch (opcion) {
            case 1: verificarIntegridadUI(); break;
            case 2: crearBackupUI(); break;
            case 0: break;
            default: cout << "Opcion invalida.\n";
        }
    } while (opcion != 0);
}
