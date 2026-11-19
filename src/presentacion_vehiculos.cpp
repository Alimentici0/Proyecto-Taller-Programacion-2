// ====================================================================
// presentacion_vehiculos.cpp
// Submenu "Gestion de Vehiculos". Solo cin/cout y llamadas a la logica.
// ====================================================================
#include "presentacion.h"
#include "logica_vehiculos.h"
#include "utilidades.h"

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdlib>

using namespace std;

static void mostrarVehiculo(const Vehiculo& v) {
    cout << "  ID: " << v.id << " | Placa: " << v.placa << "\n"
         << "  Vehiculo: " << v.marca << " " << v.modelo << " (" << v.anio << ") | Km: " << v.kilometraje << "\n"
         << "  Propietario: " << v.nombrePropietario
         << " | Cedula: " << v.cedulaPropietario
         << " | Telefono: " << v.telefonoPropietario << "\n";
}

static void mostrarVehiculoEnLinea(const Vehiculo& v) {
    cout << "ID: " << v.id << " | Placa: " << v.placa << " | " << v.marca << " " << v.modelo
         << " | Anio: " << v.anio << " | Km: " << v.kilometraje
         << " | Propietario: " << v.nombrePropietario << "\n";
}

// ---- 1. Registrar ----
static void registrarVehiculoUI() {
    Vehiculo v;
    memset(&v, 0, sizeof(Vehiculo));
    cout << "\n--- REGISTRAR NUEVO VEHICULO ---\n";
    if (!leerLinea("Placa: ", v.placa, sizeof(v.placa))) return;
    if (!leerLinea("Marca: ", v.marca, sizeof(v.marca))) return;
    if (!leerLinea("Modelo: ", v.modelo, sizeof(v.modelo))) return;
    if (!leerEntero("Anio: ", v.anio)) return;
    if (!leerEntero("Kilometraje: ", v.kilometraje)) return;
    if (!leerLinea("Nombre del propietario: ", v.nombrePropietario, sizeof(v.nombrePropietario))) return;
    if (!leerLinea("Cedula del propietario: ", v.cedulaPropietario, sizeof(v.cedulaPropietario))) return;
    if (!leerLinea("Telefono del propietario: ", v.telefonoPropietario, sizeof(v.telefonoPropietario))) return;

    int id;
    Resultado r = registrarVehiculo(v, id);
    if (r == EXITO) {
        cout << "Exito! Vehiculo " << v.marca << " " << v.modelo << " registrado con el ID #" << id << ".\n";
    } else if (r == ERR_DUPLICADO) {
        cout << "ERROR: ya existe un vehiculo activo con la placa " << v.placa << ".\n";
    } else {
        mostrarError(r);
    }
}

// ---- 2. Listar (activos) ----
static void listarVehiculosUI() {
    int total;
    Resultado r = cantidadVehiculosRegistrados(total);
    if (r != EXITO) { mostrarError(r); return; }

    // Se recorre por ID con UN registro en memoria a la vez
    int mostrados = 0;
    cout << "\n--- LISTA DE VEHICULOS REGISTRADOS ---\n";
    for (int id = 1; id <= total; id++) {
        Vehiculo v;
        r = obtenerVehiculoPorID(id, v);
        if (r == ERR_ELIMINADO) continue;              // los eliminados no se listan
        if (r != EXITO) { mostrarError(r); return; }
        mostrarVehiculoEnLinea(v);
        mostrados++;
    }
    if (mostrados == 0) cout << "No hay vehiculos registrados.\n";
    else cout << "Total: " << mostrados << " vehiculo(s).\n";
}

// ---- 3. Buscar por ID (acceso directo + mensaje de depuracion) ----
static void buscarVehiculoPorIdUI() {
    int id;
    cout << "\n--- BUSCAR VEHICULO POR ID ---\n";
    if (!leerEntero("ID del vehiculo: ", id)) return;

    Vehiculo v;
    long posicion = -1;
    Resultado r = obtenerVehiculoPorID(id, v, &posicion);

    if (MODO_DEPURACION && posicion >= 0) {
        cout << "[DEPURACION] Registro leido directamente en el byte " << posicion
             << " = " << sizeof(ArchivoHeader) << " + (" << id << " - 1) * " << sizeof(Vehiculo)
             << " (sin recorrer el archivo)\n";
    }

    if (r == EXITO) {
        mostrarVehiculo(v);
    } else if (r == ERR_ELIMINADO) {
        cout << "El vehiculo con ID " << id << " (placa " << v.placa << ", " << v.marca << " " << v.modelo
             << ") fue ELIMINADO.\n";
    } else if (r == ERR_NO_EXISTE) {
        cout << "No existe ningun vehiculo con el ID " << id << ".\n";
    } else {
        mostrarError(r);
    }
}

// ---- 4. Buscar por placa ----
static void buscarVehiculoPorPlacaUI() {
    char placa[10];
    cout << "\n--- BUSCAR VEHICULO POR PLACA ---\n";
    if (!leerLinea("Placa exacta: ", placa, sizeof(placa))) return;

    Vehiculo v;
    Resultado r = obtenerVehiculoPorPlaca(placa, v);
    if (r == EXITO) mostrarVehiculo(v);
    else if (r == ERR_NO_EXISTE) cout << "No hay ningun vehiculo activo con la placa " << placa << ".\n";
    else mostrarError(r);
}

// ---- 5. Buscar por propietario (parcial) ----
static void buscarVehiculoPorPropietarioUI() {
    char texto[100];
    cout << "\n--- BUSCAR VEHICULOS POR PROPIETARIO ---\n";
    if (!leerLinea("Nombre (o parte del nombre): ", texto, sizeof(texto))) return;

    Vehiculo resultados[MAX_RESULTADOS_BUSQUEDA];
    int cantidad;
    bool hayMas;
    Resultado r = buscarVehiculosPorPropietario(texto, resultados, cantidad, hayMas);
    if (r != EXITO) { mostrarError(r); return; }

    if (cantidad == 0) {
        cout << "No se encontraron vehiculos con ese propietario.\n";
        return;
    }
    for (int i = 0; i < cantidad; i++) mostrarVehiculoEnLinea(resultados[i]);
    cout << cantidad << " resultado(s).\n";
    if (hayMas) {
        cout << "AVISO: hay mas coincidencias; solo se muestran las primeras " << MAX_RESULTADOS_BUSQUEDA
             << ". Escriba una busqueda mas especifica.\n";
    }
}

// ---- 6. Actualizar ----
static void actualizarVehiculoUI() {
    int id;
    cout << "\n--- ACTUALIZAR VEHICULO ---\n";
    if (!leerEntero("ID del vehiculo: ", id)) return;

    Vehiculo actual;
    Resultado r = obtenerVehiculoPorID(id, actual);
    if (r == ERR_ELIMINADO) { cout << "El vehiculo con ID " << id << " fue ELIMINADO; no se puede actualizar.\n"; return; }
    if (r != EXITO) { mostrarError(r); return; }

    mostrarVehiculo(actual);
    cout << "(Deje vacio para mantener el valor actual. La placa, el kilometraje y el ID no se modifican.)\n";

    Vehiculo nuevo = actual;
    char texto[100];
    if (!leerLinea("Marca: ", texto, sizeof(texto))) return;
    if (!textoVacio(texto)) copiarTexto(nuevo.marca, sizeof(nuevo.marca), texto);
    if (!leerLinea("Modelo: ", texto, sizeof(texto))) return;
    if (!textoVacio(texto)) copiarTexto(nuevo.modelo, sizeof(nuevo.modelo), texto);
    if (!leerLinea("Anio: ", texto, sizeof(texto))) return;
    if (!textoVacio(texto)) nuevo.anio = atoi(texto);
    if (!leerLinea("Nombre del propietario: ", texto, sizeof(texto))) return;
    if (!textoVacio(texto)) copiarTexto(nuevo.nombrePropietario, sizeof(nuevo.nombrePropietario), texto);
    if (!leerLinea("Cedula del propietario: ", texto, sizeof(texto))) return;
    if (!textoVacio(texto)) copiarTexto(nuevo.cedulaPropietario, sizeof(nuevo.cedulaPropietario), texto);
    if (!leerLinea("Telefono del propietario: ", texto, sizeof(texto))) return;
    if (!textoVacio(texto)) copiarTexto(nuevo.telefonoPropietario, sizeof(nuevo.telefonoPropietario), texto);

    r = actualizarVehiculo(id, nuevo);
    if (r == EXITO) cout << "Vehiculo actualizado.\n";
    else mostrarError(r);
}

// ---- 7. Eliminar ----
static void eliminarVehiculoUI() {
    int id;
    cout << "\n--- ELIMINAR VEHICULO ---\n";
    if (!leerEntero("ID del vehiculo: ", id)) return;

    Vehiculo v;
    Resultado r = obtenerVehiculoPorID(id, v);
    if (r == ERR_ELIMINADO) { cout << "El vehiculo con ID " << id << " ya estaba eliminado.\n"; return; }
    if (r != EXITO) { mostrarError(r); return; }

    r = eliminarVehiculo(id);
    if (r == EXITO) {
        cout << "Vehiculo " << v.marca << " " << v.modelo << " (placa " << v.placa << ") eliminado.\n";
    } else if (r == ERR_TIENE_ORDENES) {
        cout << "ERROR: no se puede eliminar el vehiculo " << v.marca << " " << v.modelo << " (placa "
             << v.placa << "): tiene ordenes de servicio registradas.\n";
    } else {
        mostrarError(r);
    }
}

// ---- Menu ----
void menuVehiculos() {
    int opcion = -1;
    do {
        cout << "\n--- GESTION DE VEHICULOS ---\n"
             << "1. Registrar vehiculo\n"
             << "2. Listar vehiculos\n"
             << "3. Buscar por ID\n"
             << "4. Buscar por placa\n"
             << "5. Buscar por propietario\n"
             << "6. Actualizar vehiculo\n"
             << "7. Eliminar vehiculo\n"
             << "0. Volver\n";
        if (!leerEntero("Seleccione una opcion: ", opcion)) return;

        switch (opcion) {
            case 1: registrarVehiculoUI(); break;
            case 2: listarVehiculosUI(); break;
            case 3: buscarVehiculoPorIdUI(); break;
            case 4: buscarVehiculoPorPlacaUI(); break;
            case 5: buscarVehiculoPorPropietarioUI(); break;
            case 6: actualizarVehiculoUI(); break;
            case 7: eliminarVehiculoUI(); break;
            case 0: break;
            default: cout << "Opcion invalida.\n";
        }
    } while (opcion != 0);
}
