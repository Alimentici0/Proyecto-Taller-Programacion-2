// ====================================================================
// persistencia_vehiculos.h
// Las cinco operaciones de persistencia para vehiculos.bin.
// (Se repiten por entidad a proposito; en el Proyecto 3 se unificaran.)
//
// Convencion de retorno comun a las cuatro entidades:
//   EXITO         -> la operacion se realizo
//   ERR_NO_EXISTE -> el ID no corresponde a ningun registro del archivo
//   ERR_ARCHIVO   -> no se pudo abrir/leer/escribir el archivo
//   ERR_ELIMINADO -> (solo marcarEliminado) ya estaba eliminado
// ====================================================================
#ifndef PERSISTENCIA_VEHICULOS_H
#define PERSISTENCIA_VEHICULOS_H

#include "persistencia.h"

// Asigna el ID desde el header, eliminado = false y las marcas de
// tiempo; escribe el registro y actualiza el header.
// El ID asignado se entrega en idAsignado.
Resultado guardarVehiculo(const Vehiculo& nuevo, int& idAsignado);

// Ubica el registro por calculo (sin recorrer el archivo). Lo entrega
// este activo o eliminado. Si posicionLeida no es nullptr recibe el
// byte exacto donde se hizo la lectura (para el mensaje de depuracion).
Resultado leerVehiculoPorID(int id, Vehiculo& destino, long* posicionLeida = nullptr);

// Escribe el registro en su posicion original (segun v.id) y
// actualiza v.fechaUltimaModificacion.
Resultado sobrescribirVehiculo(Vehiculo& v);

// Borrado logico: eliminado = true y registrosActivos-- en el header.
Resultado marcarVehiculoEliminado(int id);

// Cantidad de registros activos, tomada del header.
Resultado contarVehiculosActivos(int& cantidad);

#endif
