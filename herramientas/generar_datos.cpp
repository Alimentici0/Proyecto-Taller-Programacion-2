// ====================================================================
// generar_datos.cpp
// Herramienta que crea los archivos .bin pre-poblados de la carpeta datos/.
//
// Usa la capa de LOGICA (no escribe bytes a mano), asi el stock, los
// precios historicos y las reglas de negocio quedan consistentes. Al
// final reescribe las FECHAS de las ordenes para que el historial se
// vea realista (la logica siempre pone la fecha de hoy).
//
// Compilar y ejecutar desde la raiz del proyecto:  make datos
// ATENCION: borra los .bin actuales de datos/ y los vuelve a crear.
// ====================================================================
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <filesystem>

#include "logica_taller.h"
#include "logica_vehiculos.h"
#include "logica_mecanicos.h"
#include "logica_repuestos.h"
#include "logica_ordenes.h"
#include "utilidades.h"

using namespace std;

static void exigir(Resultado r, const char* contexto) {
    if (r != EXITO) {
        cerr << "FALLO al generar datos (" << contexto << "), codigo de resultado " << r << "\n";
        exit(1);
    }
}

static void crearVehiculo(const char* placa, const char* marca, const char* modelo, int anio, int km,
                          const char* propietario, const char* cedula, const char* telefono) {
    Vehiculo v;
    memset(&v, 0, sizeof(Vehiculo));
    copiarTexto(v.placa, sizeof(v.placa), placa);
    copiarTexto(v.marca, sizeof(v.marca), marca);
    copiarTexto(v.modelo, sizeof(v.modelo), modelo);
    v.anio = anio;
    v.kilometraje = km;
    copiarTexto(v.nombrePropietario, sizeof(v.nombrePropietario), propietario);
    copiarTexto(v.cedulaPropietario, sizeof(v.cedulaPropietario), cedula);
    copiarTexto(v.telefonoPropietario, sizeof(v.telefonoPropietario), telefono);
    int id;
    exigir(registrarVehiculo(v, id), placa);
}

static void crearMecanico(const char* nombre, const char* cedula, const char* especialidad, const char* ingreso) {
    Mecanico m;
    memset(&m, 0, sizeof(Mecanico));
    copiarTexto(m.nombre, sizeof(m.nombre), nombre);
    copiarTexto(m.cedula, sizeof(m.cedula), cedula);
    copiarTexto(m.especialidad, sizeof(m.especialidad), especialidad);
    copiarTexto(m.fechaIngreso, sizeof(m.fechaIngreso), ingreso);
    int id;
    exigir(registrarMecanico(m, id), nombre);
}

static void crearRepuesto(const char* codigo, const char* nombre, const char* marca, float precio, int stock, int minimo) {
    Repuesto r;
    memset(&r, 0, sizeof(Repuesto));
    copiarTexto(r.codigo, sizeof(r.codigo), codigo);
    copiarTexto(r.nombre, sizeof(r.nombre), nombre);
    copiarTexto(r.marca, sizeof(r.marca), marca);
    r.precio = precio;
    r.stock = stock;
    r.stockMinimo = minimo;
    int id;
    exigir(registrarRepuesto(r, id), codigo);
}

// Reescribe las fechas de una orden (la logica siempre usa la de hoy)
static void fijarFechas(int idOrden, const char* apertura, const char* cierre) {
    OrdenServicio o;
    exigir(leerOrdenPorID(idOrden, o), "leer orden para fijar fechas");
    copiarTexto(o.fechaApertura, sizeof(o.fechaApertura), apertura);
    copiarTexto(o.fechaCierre, sizeof(o.fechaCierre), cierre);
    exigir(sobrescribirOrden(o), "fijar fechas");
}

struct Item { int idRepuesto; int cantidad; };

// Abre una orden, agrega sus lineas y la deja CERRADA (cierre != nullptr) o ABIERTA
static int crearOrden(int idVeh, int idMec, int km, const char* falla,
                      const Item* items, int cantidadItems,
                      float manoObra, bool cerrar, const char* apertura, const char* cierre) {
    int idOrden;
    exigir(abrirOrden(idVeh, idMec, km, falla, idOrden), falla);
    for (int i = 0; i < cantidadItems; i++) {
        exigir(agregarRepuestoAOrden(idOrden, items[i].idRepuesto, items[i].cantidad), "agregar repuesto");
    }
    if (cerrar) exigir(cerrarOrden(idOrden, manoObra), "cerrar orden");
    fijarFechas(idOrden, apertura, cierre);
    return idOrden;
}

int main() {
    // 0. Partir de cero
    for (int i = 0; i < CANTIDAD_ARCHIVOS_BIN; i++) filesystem::remove(ARCHIVOS_BIN[i]);
    const char* rutaConProblema = nullptr;
    exigir(inicializarTodosLosArchivos(rutaConProblema), "inicializar archivos");

    // 1. Taller
    Taller t;
    memset(&t, 0, sizeof(Taller));
    copiarTexto(t.nombre, sizeof(t.nombre), "Autoservicio El Lago C.A.");
    copiarTexto(t.rif, sizeof(t.rif), "J-30123456-7");
    copiarTexto(t.direccion, sizeof(t.direccion), "Av. 5 de Julio, Maracaibo, Zulia");
    copiarTexto(t.telefono, sizeof(t.telefono), "0261-7001234");
    exigir(registrarTaller(t), "taller");

    // 2. Vehiculos (IDs 1..9)
    crearVehiculo("AA123BB", "Toyota",    "Corolla", 2015,  85000, "Carlos Rivas",     "V-12345678", "0414-1112233");
    crearVehiculo("AB456CD", "Chevrolet", "Aveo",    2012, 120000, "Maria Gonzalez",   "V-14567890", "0424-2223344");
    crearVehiculo("AC789EF", "Ford",      "Fiesta",  2018,  45000, "Luis Fernandez",   "V-16789012", "0412-3334455");
    crearVehiculo("AD012GH", "Honda",     "Civic",   2020,  30000, "Ana Martinez",     "V-18901234", "0416-4445566");
    crearVehiculo("AE345IJ", "Hyundai",   "Accent",  2016,  98000, "Pedro Ramirez",    "V-11223344", "0414-5556677");
    crearVehiculo("AF678KL", "Kia",       "Rio",     2019,  20000, "Sofia Perez",      "V-22334455", "0424-6667788");
    crearVehiculo("AG901MN", "Toyota",    "Hilux",   2014, 150000, "Jorge Morales",    "V-9876543",  "0412-7778899");
    crearVehiculo("AH234OP", "Nissan",    "Sentra",  2021,  12000, "Elena Castillo",   "V-25667788", "0416-8889900");
    crearVehiculo("AJ567QR", "Chevrolet", "Spark",   2010, 160000, "Ramon Duarte",     "V-7654321",  "0414-9990011");

    // 3. Mecanicos (IDs 1..4)
    crearMecanico("Jose Perez",      "V-12345678", "MOTOR",        "2018-03-12");
    crearMecanico("Ana Villalobos",  "V-14567890", "FRENOS",       "2019-07-01");
    crearMecanico("Luis Fuenmayor",  "V-16789012", "ELECTRICIDAD", "2017-05-20");
    crearMecanico("Marta Soto",      "V-18901234", "GENERAL",      "2021-01-15");

    // 4. Repuestos (IDs 1..13)
    crearRepuesto("FIL-ACE-01", "Filtro de aceite",             "Fram",     6.50f,  40, 10);   // 1
    crearRepuesto("FIL-AIR-01", "Filtro de aire",               "Fram",     8.00f,  25,  8);   // 2
    crearRepuesto("PAS-FRE-01", "Pastillas de freno delanteras","Brembo",  28.00f,  15,  5);   // 3
    crearRepuesto("BUJ-NGK-01", "Bujia NGK",                    "NGK",      4.25f,  60, 20);   // 4
    crearRepuesto("BAT-12V-01", "Bateria 12V 75Ah",             "Bosch",   95.00f,   6,  3);   // 5
    crearRepuesto("ACE-10W40",  "Aceite 10W-40 1L",             "Castrol",  7.80f,  80, 20);   // 6
    crearRepuesto("COR-DIS-01", "Correa de distribucion",       "Gates",   32.00f,  10,  4);   // 7
    crearRepuesto("AMO-DEL-01", "Amortiguador delantero",       "Monroe",  55.00f,   8,  4);   // 8
    crearRepuesto("LIQ-FRE-01", "Liquido de frenos DOT4",       "Prestone", 5.50f,  30, 10);   // 9
    crearRepuesto("BOM-AGU-01", "Bomba de agua",                "Gates",   42.00f,   5,  3);   // 10
    crearRepuesto("ALT-001",    "Alternador 90A",               "Bosch",  120.00f,   3,  2);   // 11
    crearRepuesto("REF-CLT-01", "Refrigerante 1L",              "Prestone", 4.00f,  50, 15);   // 12
    crearRepuesto("OBS-999",    "Repuesto descontinuado",       "Varios",   1.00f,   1,  0);   // 13 (se elimina abajo)

    // 5. Ordenes (IDs 1..10) con distintos estados
    {   Item i[] = { {6,4}, {1,1}, {2,1}, {4,4} };
        crearOrden(1, 1,  86000, "Cambio de aceite y filtros", i, 4,  40.0f, true,  "2026-09-03", "2026-09-03"); }
    {   Item i[] = { {3,1}, {9,2} };
        crearOrden(2, 2, 120500, "Los frenos chillan",         i, 2,  35.0f, true,  "2026-09-10", "2026-09-11"); }
    {   Item i[] = { {5,1} };
        crearOrden(3, 3,  45500, "Falla en el sistema electrico", i, 1, 20.0f, true, "2026-09-15", "2026-09-15"); }
    {   Item i[] = { {7,1}, {10,1}, {6,4}, {12,2} };
        crearOrden(4, 1,  30800, "Cambio de correa de distribucion", i, 4, 120.0f, true, "2026-09-22", "2026-09-24"); }
    {   Item i[] = { {8,2}, {3,1} };                         // se cierra y luego se ANULA
        int o = crearOrden(5, 2, 98500, "Amortiguadores desgastados", i, 2, 80.0f, true, "2026-10-01", "2026-10-02");
        exigir(anularOrden(o), "anular orden 5"); }
    {   Item i[] = { {1,1}, {2,1}, {4,4}, {6,4}, {9,1} };
        crearOrden(6, 4,  20100, "Mantenimiento de 20000 km",  i, 5,  60.0f, true,  "2026-10-08", "2026-10-08"); }
    {   Item i[] = { {11,1}, {5,1} };
        crearOrden(7, 3, 150400, "El alternador no carga",     i, 2,  50.0f, true,  "2026-10-14", "2026-10-15"); }
    {   Item i[] = { {8,1} };                                // ABIERTA
        crearOrden(1, 4,  90500, "Ruido en la suspension",     i, 1,   0.0f, false, "2026-11-10", ""); }
    {   Item i[] = { {12,2}, {10,1}, {9,1} };                // ABIERTA
        crearOrden(2, 1, 123000, "Recalentamiento del motor",  i, 3,   0.0f, false, "2026-11-12", ""); }
    {   Item i[] = { {4,4}, {2,1} };                         // ABIERTA y luego ANULADA
        int o = crearOrden(8, 2,  12300, "Revision general",   i, 2,   0.0f, false, "2026-11-05", "");
        exigir(anularOrden(o), "anular orden 10"); }

    // 6. Un mecanico de baja (sin ordenes abiertas) y registros eliminados
    exigir(darDeBajaMecanico(3), "baja del mecanico 3");
    exigir(eliminarVehiculo(9), "eliminar vehiculo 9");
    exigir(eliminarRepuesto(13), "eliminar repuesto 13");

    cout << "Datos de ejemplo generados en la carpeta " << CARPETA_DATOS << "/\n";
    return 0;
}
