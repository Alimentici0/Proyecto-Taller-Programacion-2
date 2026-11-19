// ====================================================================
// persistencia_repuestos.h
// Las cinco operaciones de persistencia para repuestos.bin.
// (Se repiten por entidad a proposito; en el Proyecto 3 se unificaran.)
//
// Convencion de retorno comun a las cuatro entidades:
//   EXITO         -> la operacion se realizo
//   ERR_NO_EXISTE -> el ID no corresponde a ningun registro del archivo
//   ERR_ARCHIVO   -> no se pudo abrir/leer/escribir el archivo
//   ERR_ELIMINADO -> (solo marcarEliminado) ya estaba eliminado
// ====================================================================
#ifndef PERSISTENCIA_REPUESTOS_H
#define PERSISTENCIA_REPUESTOS_H

#include "persistencia.h"

// Asigna el ID desde el header, eliminado = false y las marcas de
// tiempo; escribe el registro y actualiza el header.
// El ID asignado se entrega en idAsignado.
Resultado guardarRepuesto(const Repuesto& nuevo, int& idAsignado);

// Ubica el registro por calculo (sin recorrer el archivo). Lo entrega
// tanto si esta activo como si esta eliminado. Si posicionLeida no es
// nullptr recibe el byte exacto donde se hizo la lectura (sirve para
// el mensaje de depuracion).
Resultado leerRepuestoPorID(int id, Repuesto& destino, long* posicionLeida = nullptr);

// Escribe el registro en su posicion original (segun rep.id) y
// actualiza rep.fechaUltimaModificacion.
Resultado sobrescribirRepuesto(Repuesto& rep);

// Borrado logico: eliminado = true y registrosActivos-- en el header.
Resultado marcarRepuestoEliminado(int id);

// Cantidad de registros activos, tomada del header.
Resultado contarRepuestosActivos(int& cantidad);

#endif
