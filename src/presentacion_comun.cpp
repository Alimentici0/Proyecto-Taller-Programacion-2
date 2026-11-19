// ====================================================================
// presentacion_comun.cpp
// Lectura segura de datos (con getline, sin los problemas de mezclar
// cin >> con cin.getline) y traduccion de codigos de error a mensajes.
// ====================================================================
#include "presentacion.h"
#include "utilidades.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <cerrno>

using namespace std;

static bool ecoEntrada = false;

void activarEcoEntrada(bool activo) {
    ecoEntrada = activo;
}

// Devuelve false si se acabo la entrada (EOF)
bool leerLinea(const char* mensaje, char* destino, size_t capacidad) {
    cout << mensaje;
    string linea;
    if (!getline(cin, linea)) return false;
    if (!linea.empty() && linea[linea.size() - 1] == '\r') linea.erase(linea.size() - 1);

    if (ecoEntrada) cout << linea << "\n";

    if (linea.size() > capacidad - 1) {
        cout << "  (aviso: el texto se recorto a " << (capacidad - 1) << " caracteres)\n";
    }
    copiarTexto(destino, capacidad, linea.c_str());
    return true;
}

bool leerEntero(const char* mensaje, int& valor) {
    char texto[64];
    while (true) {
        if (!leerLinea(mensaje, texto, sizeof(texto))) return false;
        char* fin = nullptr;
        errno = 0;
        long numero = strtol(texto, &fin, 10);
        bool valido = (fin != texto) && textoVacio(fin) && errno == 0 &&
                      numero >= -2147483647L && numero <= 2147483647L;
        if (valido) {
            valor = static_cast<int>(numero);
            return true;
        }
        cout << "  Entrada invalida: escriba un numero entero.\n";
    }
}

bool leerDecimal(const char* mensaje, float& valor) {
    char texto[64];
    while (true) {
        if (!leerLinea(mensaje, texto, sizeof(texto))) return false;
        for (char* c = texto; *c; c++) if (*c == ',') *c = '.';     // acepta coma decimal
        char* fin = nullptr;
        errno = 0;
        double numero = strtod(texto, &fin);
        if (fin != texto && textoVacio(fin) && errno == 0) {
            valor = static_cast<float>(numero);
            return true;
        }
        cout << "  Entrada invalida: escriba un numero (ej. 12.50).\n";
    }
}

// --------------------------------------------------------------------
// Cajas del menu (ASCII para que se vean igual en cualquier consola)
// --------------------------------------------------------------------
void imprimirBordeCaja(char relleno) {
    cout << "+" << string(43, relleno) << "+\n";
}

void imprimirLineaCaja(const char* texto) {
    cout << "| " << left << setw(41) << texto << " |\n";
}

// --------------------------------------------------------------------
// Resultado -> mensaje legible
// --------------------------------------------------------------------
const char* mensajeResultado(Resultado r) {
    switch (r) {
        case EXITO:                          return "Operacion realizada.";
        case ERR_ARCHIVO:                    return "No se pudo leer o escribir un archivo de datos. La operacion no continuo.";
        case ERR_NO_EXISTE:                  return "No existe un registro con ese ID.";
        case ERR_ELIMINADO:                  return "El registro fue eliminado (o dado de baja).";
        case ERR_DATOS_INVALIDOS:            return "Hay campos obligatorios vacios.";
        case ERR_ANIO_INVALIDO:              return "El anio debe estar entre 1950 y 2027.";
        case ERR_KILOMETRAJE_INVALIDO:       return "Kilometraje invalido (debe ser >= 0 y no menor al ya registrado del vehiculo).";
        case ERR_DUPLICADO:                  return "Ya existe un registro con ese dato unico (placa, cedula o codigo).";
        case ERR_ESPECIALIDAD_INVALIDA:      return "Especialidad invalida. Use MOTOR, FRENOS, ELECTRICIDAD, SUSPENSION o GENERAL.";
        case ERR_FECHA_INVALIDA:             return "Fecha invalida. Use el formato AAAA-MM-DD.";
        case ERR_PRECIO_INVALIDO:            return "El precio debe ser mayor que 0.";
        case ERR_STOCK_INVALIDO:             return "El stock y el stock minimo no pueden ser negativos.";
        case ERR_CANTIDAD_INVALIDA:          return "La cantidad debe ser mayor que 0.";
        case ERR_TIENE_ORDENES:              return "No se puede eliminar: el vehiculo tiene ordenes registradas.";
        case ERR_TIENE_ORDENES_ABIERTAS:     return "No se puede dar de baja: el mecanico tiene ordenes ABIERTAS.";
        case ERR_REPUESTO_EN_USO:            return "No se puede eliminar: el repuesto aparece en el detalle de alguna orden.";
        case ERR_VEHICULO_NO_EXISTE:         return "El vehiculo indicado no existe.";
        case ERR_VEHICULO_ELIMINADO:         return "El vehiculo indicado fue eliminado.";
        case ERR_MECANICO_NO_EXISTE:         return "El mecanico indicado no existe.";
        case ERR_MECANICO_ELIMINADO:         return "El mecanico indicado esta dado de baja y no puede recibir ordenes.";
        case ERR_VEHICULO_CON_ORDEN_ABIERTA: return "El vehiculo ya tiene una orden ABIERTA.";
        case ERR_MECANICO_LIMITE_ORDENES:    return "El mecanico ya tiene 3 ordenes ABIERTAS (limite).";
        case ERR_ORDEN_NO_EXISTE:            return "La orden indicada no existe.";
        case ERR_ESTADO_INCORRECTO:          return "La orden no esta en un estado que permita esta operacion.";
        case ERR_REPUESTO_NO_EXISTE:         return "El repuesto indicado no existe.";
        case ERR_REPUESTO_ELIMINADO:         return "El repuesto indicado fue eliminado.";
        case ERR_STOCK_INSUFICIENTE:         return "Stock insuficiente para esa cantidad.";
        case ERR_LIMITE_LINEAS:              return "La orden alcanzo el maximo de lineas de detalle; la linea no se agrego.";
        case ERR_LINEA_NO_EXISTE:            return "Ese repuesto no esta en el detalle de la orden.";
        case ERR_ORDEN_VACIA:                return "No se puede cerrar: la orden necesita al menos un repuesto o mano de obra > 0.";
        case ERR_MANO_OBRA_INVALIDA:         return "La mano de obra no puede ser negativa.";
    }
    return "Error desconocido.";
}

void mostrarError(Resultado r) {
    cout << "ERROR: " << mensajeResultado(r) << "\n";
}
