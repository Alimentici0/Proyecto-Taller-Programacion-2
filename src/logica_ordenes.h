// ====================================================================
// logica_ordenes.h
// CAPA DE LOGICA - ORDENES DE SERVICIO (convenciones: ver logica_vehiculos.h)
//
// Varias operaciones escriben en mas de un archivo. Regla: TODAS las
// validaciones se hacen ANTES de la primera escritura; si algo falla,
// no se escribe nada. Y cuando hay que escribir en dos archivos, se
// escribe PRIMERO la orden y DESPUES el inventario/vehiculo (el porque
// esta explicado en README_V2.md, seccion "Orden de escritura").
// ====================================================================
#ifndef LOGICA_ORDENES_H
#define LOGICA_ORDENES_H

#include "persistencia_ordenes.h"

const int MAX_HISTORIAL_VEHICULO = 50;    // ordenes que entrega el historial de un vehiculo

// Una orden junto con los datos legibles de su vehiculo y su mecanico,
// leidos de sus archivos. Existe para que la presentacion muestre
// nombres y no IDs sueltos. NO se guarda en disco.
struct VistaOrden {
    OrdenServicio orden;
    bool  vehiculoEncontrado;
    char  placa[10];
    char  descripcionVehiculo[110];     // "Marca Modelo (anio)"
    char  propietario[100];
    bool  mecanicoEncontrado;
    char  nombreMecanico[100];
    bool  mecanicoDeBaja;               // true si el mecanico fue dado de baja
    float totalRepuestos;
    float total;                        // mano de obra + repuestos
};

// --------------------------------------------------------------------
// Abrir y consultar
// --------------------------------------------------------------------

// Validaciones (en este orden): descripcion no vacia, vehiculo activo,
// mecanico activo, el vehiculo no tiene otra orden ABIERTA, el mecanico
// tiene menos de 3 ABIERTAS, y el kilometraje de ingreso no es menor
// que el del vehiculo. Fecha de apertura = fecha actual.
// Escribe: ordenes.bin (nueva orden) y despues vehiculos.bin (kilometraje).
// Errores: ERR_DATOS_INVALIDOS, ERR_VEHICULO_NO_EXISTE, ERR_VEHICULO_ELIMINADO,
//          ERR_MECANICO_NO_EXISTE, ERR_MECANICO_ELIMINADO,
//          ERR_VEHICULO_CON_ORDEN_ABIERTA, ERR_MECANICO_LIMITE_ORDENES,
//          ERR_KILOMETRAJE_INVALIDO, ERR_ARCHIVO.
Resultado abrirOrden(int idVehiculo, int idMecanico, int kilometrajeIngreso,
                     const char* descripcionFalla, int& idOrden);

// Lectura directa por ID. EXITO | ERR_ORDEN_NO_EXISTE | ERR_ARCHIVO.
// (Las ordenes nunca se eliminan, asi que no existe ERR_ELIMINADO aqui.)
Resultado obtenerOrdenPorID(int id, OrdenServicio& resultado);

// Igual que la anterior, pero con nombres de vehiculo y mecanico.
// Funciona aunque el mecanico este dado de baja.
Resultado obtenerVistaOrden(int id, VistaOrden& vista);

// Mano de obra + suma(cantidad * precio historico de cada linea).
float calcularTotalRepuestos(const OrdenServicio& orden);
float calcularTotalOrden(const OrdenServicio& orden);

// IDs de las ordenes de un vehiculo, de la mas antigua a la mas nueva
// (maximo MAX_HISTORIAL_VEHICULO; 'hayMas' avisa si habia mas).
Resultado buscarOrdenesDeVehiculo(int idVehiculo, int ids[], int maximo,
                                  int& cantidad, bool& hayMas);

// Para recorrer ordenes.bin por ID (1..total) desde la presentacion.
Resultado cantidadOrdenesRegistradas(int& total);

#endif
