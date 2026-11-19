// main.cpp (version provisional): muestra el tamano en bytes de cada
// estructura que se escribira en disco.
#include <iostream>
#include "estructuras.h"

using namespace std;

int main() {
    cout << "sizeof(ArchivoHeader)  = " << sizeof(ArchivoHeader) << " bytes\n";
    cout << "sizeof(Taller)         = " << sizeof(Taller) << " bytes\n";
    cout << "sizeof(Vehiculo)       = " << sizeof(Vehiculo) << " bytes\n";
    cout << "sizeof(Mecanico)       = " << sizeof(Mecanico) << " bytes\n";
    cout << "sizeof(Repuesto)       = " << sizeof(Repuesto) << " bytes\n";
    cout << "sizeof(LineaDetalle)   = " << sizeof(LineaDetalle) << " bytes\n";
    cout << "sizeof(OrdenServicio)  = " << sizeof(OrdenServicio) << " bytes\n";
    return 0;
}
