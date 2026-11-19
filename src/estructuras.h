// ====================================================================
// estructuras.h
// Estructuras de datos del Proyecto 2 (representacion en DISCO).
//
// Regla: una estructura que se escribe con write() NO puede tener
// punteros, arreglos dinamicos ni std::string. Solo tipos de tamano
// fijo (int, float, bool, time_t, char[]).
// ====================================================================
#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <ctime>
#include <type_traits>

// --------------------------------------------------------------------
// Constantes del sistema
// --------------------------------------------------------------------
const int MAX_LINEAS_ORDEN = 20;            // lineas de detalle por orden (>= 15 exigido)
const int MAX_RESULTADOS_BUSQUEDA = 20;     // maximo de resultados de una busqueda parcial
const int MAX_ORDENES_ABIERTAS_MECANICO = 3;
const int ANIO_MINIMO = 1950;
const int ANIO_MAXIMO = 2027;

// Estados posibles de una orden (se guardan como texto en char[12])
const char* const ESTADO_ABIERTA = "ABIERTA";
const char* const ESTADO_CERRADA = "CERRADA";
const char* const ESTADO_ANULADA = "ANULADA";

// --------------------------------------------------------------------
// Header que lleva al inicio cada archivo de entidades
// --------------------------------------------------------------------
struct ArchivoHeader {
    int cantidadRegistros;   // Total historico (incluye eliminados logicamente)
    int proximoID;           // Siguiente ID a asignar
    int registrosActivos;    // Registros con eliminado == false
    int version;             // Version del formato del archivo (inicia en 1)
};

// --------------------------------------------------------------------
// Entidades
// --------------------------------------------------------------------

// taller.bin: un unico registro, SIN header
struct Taller {
    char   nombre[100];
    char   rif[20];
    char   direccion[200];
    char   telefono[20];
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

struct Vehiculo {
    int    id;
    char   placa[10];
    char   marca[50];
    char   modelo[50];
    int    anio;
    int    kilometraje;
    char   nombrePropietario[100];
    char   cedulaPropietario[20];
    char   telefonoPropietario[20];

    bool   eliminado;
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

struct Mecanico {
    int    id;
    char   nombre[100];
    char   cedula[20];
    char   especialidad[20];
    char   fechaIngreso[11];

    bool   eliminado;               // true = dado de baja
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

struct Repuesto {
    int    id;
    char   codigo[20];
    char   nombre[100];
    char   marca[50];
    float  precio;
    int    stock;
    int    stockMinimo;

    bool   eliminado;
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

// --------------------------------------------------------------------
// DISENO DEL DETALLE DE LA ORDEN (seccion 3 del enunciado)
//
// En el Proyecto 1 el detalle era DetalleOrden* (arreglo dinamico).
// En disco un puntero no sirve, asi que cada linea es una estructura
// de tamano fijo y el detalle completo es un ARREGLO FIJO dentro de la
// propia OrdenServicio:
//
//     LineaDetalle lineas[MAX_LINEAS_ORDEN];  + int numLineas;
//
// Las lineas en uso son las primeras numLineas; el resto es espacio
// reservado (desperdicio) pero mantiene todos los registros del mismo
// tamano. Ver README_V2.md seccion 3 para el analisis completo.
// --------------------------------------------------------------------
struct LineaDetalle {
    int   idRepuesto;
    int   cantidad;
    float precioUnitario;    // precio historico: el vigente al agregar la linea
};

struct OrdenServicio {
    int    id;
    int    idVehiculo;
    int    idMecanico;
    char   fechaApertura[11];
    char   fechaCierre[11];
    char   estado[12];                  // "ABIERTA", "CERRADA" o "ANULADA"
    char   descripcionFalla[200];
    int    kilometrajeIngreso;
    float  manoObra;

    int          numLineas;             // lineas en uso (0..MAX_LINEAS_ORDEN)
    LineaDetalle lineas[MAX_LINEAS_ORDEN];

    bool   eliminado;                   // Las ordenes no se eliminan: siempre false
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

// Garantia en compilacion de que las estructuras se pueden copiar byte a byte
static_assert(std::is_trivially_copyable<ArchivoHeader>::value, "ArchivoHeader debe ser copiable a bytes");
static_assert(std::is_trivially_copyable<Taller>::value, "Taller debe ser copiable a bytes");
static_assert(std::is_trivially_copyable<Vehiculo>::value, "Vehiculo debe ser copiable a bytes");
static_assert(std::is_trivially_copyable<Mecanico>::value, "Mecanico debe ser copiable a bytes");
static_assert(std::is_trivially_copyable<Repuesto>::value, "Repuesto debe ser copiable a bytes");
static_assert(std::is_trivially_copyable<LineaDetalle>::value, "LineaDetalle debe ser copiable a bytes");
static_assert(std::is_trivially_copyable<OrdenServicio>::value, "OrdenServicio debe ser copiable a bytes");

#endif
