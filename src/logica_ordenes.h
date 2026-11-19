// ====================================================================
// logica_ordenes.h
// CAPA DE LOGICA - ORDENES DE SERVICIO (convenciones: ver logica_vehiculos.h)
//
// Varias operaciones escriben en mas de un archivo. Reglas:
//  1) TODAS las validaciones se hacen ANTES de la primera escritura; si
//     algo falla, no se escribe nada.
//  2) El orden de escritura entre archivos sigue un principio: el
//     inventario NUNCA debe sobreestimar lo que hay. El stock se descuenta
//     antes de que la orden reclame unidades (agregar) y se devuelve
//     despues de que la orden las libera (quitar, anular). Asi, si el
//     programa se interrumpe a mitad, el stock queda de menos (error
//     conservador y corregible con una entrada), nunca de mas.
//     Detalle y justificacion: README_V2.md, pregunta 2.
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

// --------------------------------------------------------------------
// Operaciones sobre el detalle y el estado de la orden
// --------------------------------------------------------------------

// Solo ordenes ABIERTAS. Cantidad > 0 y stock suficiente (que se
// descuenta). La linea nueva guarda el precio vigente como precio
// historico. Si el repuesto ya esta en la orden, acumula las unidades en
// SU linea (que conserva su precio historico original).
// Escribe: repuestos.bin (el stock) y despues ordenes.bin (la linea).
// Errores: ERR_CANTIDAD_INVALIDA, ERR_ORDEN_NO_EXISTE, ERR_ESTADO_INCORRECTO,
//          ERR_REPUESTO_NO_EXISTE, ERR_REPUESTO_ELIMINADO,
//          ERR_STOCK_INSUFICIENTE, ERR_LIMITE_LINEAS, ERR_ARCHIVO.
Resultado agregarRepuestoAOrden(int idOrden, int idRepuesto, int cantidad);

// Solo ordenes ABIERTAS. Elimina la linea COMPLETA y devuelve sus
// unidades al inventario. Las lineas siguientes se corren una posicion.
// Escribe: ordenes.bin (quita la linea) y despues repuestos.bin (devuelve el stock).
// Errores: ERR_ORDEN_NO_EXISTE, ERR_ESTADO_INCORRECTO, ERR_LINEA_NO_EXISTE,
//          ERR_REPUESTO_NO_EXISTE, ERR_ARCHIVO.
Resultado quitarRepuestoDeOrden(int idOrden, int idRepuesto);

// Solo ordenes ABIERTAS. Mano de obra >= 0 y la orden debe quedar con al
// menos una linea o con mano de obra > 0. Fecha de cierre = fecha actual.
// Escribe: ordenes.bin.
// Errores: ERR_ORDEN_NO_EXISTE, ERR_ESTADO_INCORRECTO, ERR_MANO_OBRA_INVALIDA,
//          ERR_ORDEN_VACIA, ERR_ARCHIVO.
Resultado cerrarOrden(int idOrden, float manoObra);

// Solo ordenes ABIERTAS o CERRADAS. Devuelve al inventario las unidades de
// TODAS sus lineas. Las lineas se conservan en la orden como historial.
// Escribe: ordenes.bin (estado ANULADA) y despues repuestos.bin (stock).
// Errores: ERR_ORDEN_NO_EXISTE, ERR_ESTADO_INCORRECTO,
//          ERR_REPUESTO_NO_EXISTE, ERR_ARCHIVO.
Resultado anularOrden(int idOrden);

#endif
