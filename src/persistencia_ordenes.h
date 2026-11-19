// ====================================================================
// persistencia_ordenes.h
// Las cinco operaciones de persistencia para ordenes.bin.
// Cada registro es una OrdenServicio COMPLETA: incluye sus lineas de
// detalle (arreglo fijo), asi que leer o escribir una orden con sus
// lineas es una sola lectura/escritura de sizeof(OrdenServicio) bytes.
// (Se repiten por entidad a proposito; en el Proyecto 3 se unificaran.)
//
// Convencion de retorno comun a las cuatro entidades:
//   EXITO         -> la operacion se realizo
//   ERR_NO_EXISTE -> el ID no corresponde a ningun registro del archivo
//   ERR_ARCHIVO   -> no se pudo abrir/leer/escribir el archivo
//   ERR_ELIMINADO -> (solo marcarEliminado) ya estaba eliminado
// ====================================================================
#ifndef PERSISTENCIA_ORDENES_H
#define PERSISTENCIA_ORDENES_H

#include "persistencia.h"

// Asigna el ID desde el header, eliminado = false y las marcas de
// tiempo; escribe el registro y actualiza el header.
// El ID asignado se entrega en idAsignado.
Resultado guardarOrden(const OrdenServicio& nuevo, int& idAsignado);

// Ubica el registro por calculo (sin recorrer el archivo). Lo entrega
// tanto si esta activo como si esta eliminado. Si posicionLeida no es
// nullptr recibe el byte exacto donde se hizo la lectura (sirve para
// el mensaje de depuracion).
Resultado leerOrdenPorID(int id, OrdenServicio& destino, long* posicionLeida = nullptr);

// Escribe el registro en su posicion original (segun o.id) y
// actualiza o.fechaUltimaModificacion.
Resultado sobrescribirOrden(OrdenServicio& o);

// Borrado logico. (El sistema nunca lo usa: las ordenes no se eliminan,
// se ANULAN. Existe porque cada entidad tiene las mismas cinco operaciones.)
Resultado marcarOrdenEliminada(int id);

// Cantidad de registros activos, tomada del header.
Resultado contarOrdenesActivas(int& cantidad);

#endif
