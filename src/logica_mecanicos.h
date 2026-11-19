// ====================================================================
// logica_mecanicos.h
// CAPA DE LOGICA - MECANICOS (convenciones: ver logica_vehiculos.h)
// ====================================================================
#ifndef LOGICA_MECANICOS_H
#define LOGICA_MECANICOS_H

#include "persistencia_mecanicos.h"

bool especialidadValida(const char* especialidad);     // MOTOR, FRENOS, ELECTRICIDAD, SUSPENSION, GENERAL

// Cedula unica entre TODOS los mecanicos (incluidos los dados de baja:
// es la misma persona y conserva su historial). Especialidad solo entre
// los valores permitidos (se guarda en mayusculas). Fecha YYYY-MM-DD.
// Errores: ERR_DATOS_INVALIDOS, ERR_ESPECIALIDAD_INVALIDA,
//          ERR_FECHA_INVALIDA, ERR_DUPLICADO, ERR_ARCHIVO.
Resultado registrarMecanico(const Mecanico& datos, int& idAsignado);

// EXITO (activo) | ERR_ELIMINADO (de baja; el registro SI se entrega,
// porque las ordenes cerradas siguen mostrando su nombre) |
// ERR_NO_EXISTE | ERR_ARCHIVO.
Resultado obtenerMecanicoPorID(int id, Mecanico& resultado);

// Cuenta las ordenes ABIERTAS asignadas a un mecanico (recorre
// ordenes.bin con una sola orden en memoria).
Resultado contarOrdenesAbiertasDeMecanico(int idMecanico, int& cantidad);

// Baja logica. ERR_NO_EXISTE | ERR_ELIMINADO (ya estaba de baja) |
// ERR_TIENE_ORDENES_ABIERTAS.
Resultado darDeBajaMecanico(int id);

// Para recorrer por ID (1..total) desde la presentacion.
Resultado cantidadMecanicosRegistrados(int& total);
Resultado cantidadMecanicosActivos(int& activos);

#endif
