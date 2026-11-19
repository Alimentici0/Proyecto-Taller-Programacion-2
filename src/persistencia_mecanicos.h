// ====================================================================
// persistencia_mecanicos.h
// Las cinco operaciones de persistencia para mecanicos.bin.
// (Se repiten por entidad a proposito; en el Proyecto 3 se unificaran.)
//
// Convencion de retorno comun a las cuatro entidades:
//   EXITO         -> la operacion se realizo
//   ERR_NO_EXISTE -> el ID no corresponde a ningun registro del archivo
//   ERR_ARCHIVO   -> no se pudo abrir/leer/escribir el archivo
//   ERR_ELIMINADO -> (solo marcarEliminado) ya estaba eliminado
// ====================================================================
#ifndef PERSISTENCIA_MECANICOS_H
#define PERSISTENCIA_MECANICOS_H

#include "persistencia.h"

// Asigna el ID desde el header, eliminado = false y las marcas de
// tiempo; escribe el registro y actualiza el header.
// El ID asignado se entrega en idAsignado.
Resultado guardarMecanico(const Mecanico& nuevo, int& idAsignado);

// Ubica el registro por calculo (sin recorrer el archivo). Lo entrega
// tanto si esta activo como si esta eliminado. Si posicionLeida no es
// nullptr recibe el byte exacto donde se hizo la lectura (sirve para
// el mensaje de depuracion).
Resultado leerMecanicoPorID(int id, Mecanico& destino, long* posicionLeida = nullptr);

// Escribe el registro en su posicion original (segun m.id) y
// actualiza m.fechaUltimaModificacion.
Resultado sobrescribirMecanico(Mecanico& m);

// Borrado logico (la "baja" del mecanico): eliminado = true y registrosActivos-- en el header.
Resultado marcarMecanicoEliminado(int id);

// Cantidad de registros activos, tomada del header.
Resultado contarMecanicosActivos(int& cantidad);

#endif
