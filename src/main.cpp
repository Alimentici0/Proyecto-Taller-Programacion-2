// ====================================================================
// main.cpp - Sistema de Gestion de Taller (Proyecto 2)
//
// No hay arreglos del sistema en RAM: los datos viven en datos/*.bin y
// cada operacion lee o escribe solo los registros que necesita.
//
// Uso:  ./taller          (modo normal)
//       ./taller --eco    (repite en pantalla lo que se lee; para pruebas
//                          con archivos de entrada redirigidos)
// ====================================================================
#include <iostream>
#include <cstring>

#include "persistencia.h"
#include "logica_taller.h"
#include "presentacion.h"

using namespace std;

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--eco") == 0) activarEcoEntrada(true);
    }

    // 1. Inicializar los archivos de entidades que no existan
    const char* rutaConProblema = nullptr;
    if (inicializarTodosLosArchivos(rutaConProblema) != EXITO) {
        cout << "ERROR: no se pudo preparar el archivo '" << rutaConProblema << "'. El programa no puede continuar.\n";
        return 1;
    }

    // 2. taller.bin: se crea en el primer arranque y despues solo se lee
    if (!tallerRegistrado()) {
        if (!pedirYRegistrarTaller()) {
            cout << "No se registro el taller. Saliendo.\n";
            return 1;
        }
    }
    Taller taller;
    if (cargarTaller(taller) != EXITO) {
        cout << "ERROR: no se pudo leer '" << RUTA_TALLER << "'.\n";
        return 1;
    }

    // 3. Menu principal
    ejecutarMenuPrincipal(taller.nombre);
    return 0;
}
