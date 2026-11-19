// main.cpp (version provisional): inicializa los archivos binarios.
#include <iostream>
#include "persistencia.h"

using namespace std;

int main() {
    const char* rutaConProblema = nullptr;
    if (inicializarTodosLosArchivos(rutaConProblema) != EXITO) {
        cout << "ERROR: no se pudo inicializar el archivo " << rutaConProblema << "\n";
        return 1;
    }
    cout << "Archivos de datos listos en la carpeta " << CARPETA_DATOS << "/\n";
    return 0;
}
