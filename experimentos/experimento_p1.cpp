// ====================================================================
// experimento_p1.cpp  -  Pregunta 1 del README_V2.md
//
// Escribe con write() una OrdenServicio del PROYECTO 1 "tal cual" (con su
// puntero DetalleOrden*) y la lee desde OTRA ejecucion del programa.
//
//   ./experimento_p1 escribir   -> crea orden_p1.bin
//   ./experimento_p1 leer       -> (en otra ejecucion) lee orden_p1.bin e
//                                  intenta acceder a sus lineas
//
// Las estructuras son copia exacta de las del Proyecto 1 (etapa1/).
// Compilar:  make experimento_p1
// ====================================================================
#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

struct DetalleOrden {
    int idRepuesto;
    int cantidad;
    float precioUnitario;
};

struct OrdenServicio {
    int id;
    int idVehiculo;
    int idMecanico;
    char fechaApertura[11];
    char fechaCierre[11];
    char estado[12];
    char descripcionFalla[200];
    int kilometrajeIngreso;
    float manoObra;

    DetalleOrden* detalles;      // <- puntero a memoria dinamica
    int numDetalles;
    int capacidadDetalles;
};

static const char* ARCHIVO = "orden_p1.bin";

static void escribir() {
    OrdenServicio o;
    memset(&o, 0, sizeof(o));
    o.id = 1;
    o.idVehiculo = 1;
    o.idMecanico = 1;
    strcpy(o.estado, "ABIERTA");
    strcpy(o.descripcionFalla, "Cambio de aceite");

    o.capacidadDetalles = 5;
    o.detalles = new DetalleOrden[o.capacidadDetalles];     // memoria del HEAP de ESTA ejecucion
    o.detalles[0] = { 7, 2, 12.5f };
    o.detalles[1] = { 9, 1, 30.0f };
    o.numDetalles = 2;

    cout << "[escribir] sizeof(OrdenServicio) = " << sizeof(OrdenServicio) << " bytes\n";
    cout << "[escribir] direccion guardada en 'detalles' = " << static_cast<void*>(o.detalles) << "\n";
    cout << "[escribir] linea 0 en memoria: repuesto " << o.detalles[0].idRepuesto
         << " x" << o.detalles[0].cantidad << "\n";

    ofstream f(ARCHIVO, ios::binary | ios::trunc);
    f.write(reinterpret_cast<const char*>(&o), sizeof(OrdenServicio));   // se escribe el PUNTERO, no las lineas
    f.close();
    cout << "[escribir] orden escrita en " << ARCHIVO << ". Cierre el programa y ejecute 'leer'.\n";
    delete[] o.detalles;
}

static void leer() {
    OrdenServicio o;
    ifstream f(ARCHIVO, ios::binary);
    if (!f) { cout << "No existe " << ARCHIVO << ": ejecute primero 'escribir'.\n"; return; }
    f.read(reinterpret_cast<char*>(&o), sizeof(OrdenServicio));

    cout << "[leer] id=" << o.id << " estado=" << o.estado << " falla=" << o.descripcionFalla << "\n";
    cout << "[leer] numDetalles=" << o.numDetalles << " (los campos simples SI se recuperaron)\n";
    cout << "[leer] direccion leida en 'detalles' = " << static_cast<void*>(o.detalles)
         << "  (es la direccion de la ejecucion anterior)\n";
    cout << "[leer] intentando acceder a la linea 0...\n";
    cout.flush();

    // Acceso a una direccion que pertenecia a OTRO proceso: comportamiento indefinido.
    cout << "[leer] linea 0: repuesto " << o.detalles[0].idRepuesto
         << " x" << o.detalles[0].cantidad << "\n";
}

int main(int argc, char* argv[]) {
    if (argc != 2) { cout << "Uso: " << argv[0] << " escribir | leer\n"; return 1; }
    if (strcmp(argv[1], "escribir") == 0) escribir();
    else if (strcmp(argv[1], "leer") == 0) leer();
    else cout << "Opcion desconocida.\n";
    return 0;
}
