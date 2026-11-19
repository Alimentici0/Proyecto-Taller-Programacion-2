// ====================================================================
// logica_vehiculos.h
// CAPA DE LOGICA - VEHICULOS. Reglas de negocio; sin cin/cout.
//
// Convenciones de esta capa (iguales en los cuatro modulos):
//  - Ninguna funcion devuelve punteros: el registro leido del disco se
//    entrega como COPIA en un parametro por referencia.
//  - El valor de retorno es un Resultado (EXITO o la causa del fallo).
//  - obtener...PorID entrega el registro siempre que EXISTA. Si esta
//    eliminado devuelve ERR_ELIMINADO (con el registro cargado), de modo
//    que la presentacion pueda decir "fue eliminado" y no "no existe".
//  - Las busquedas con varios resultados entregan como maximo
//    MAX_RESULTADOS_BUSQUEDA y activan 'hayMas' si habia mas.
// ====================================================================
#ifndef LOGICA_VEHICULOS_H
#define LOGICA_VEHICULOS_H

#include "persistencia_vehiculos.h"

// Placa unica entre activos (sin distinguir mayusculas; se guarda en
// mayusculas), anio 1950-2027, kilometraje >= 0, campos obligatorios.
// Errores: ERR_DATOS_INVALIDOS, ERR_ANIO_INVALIDO, ERR_KILOMETRAJE_INVALIDO,
//          ERR_DUPLICADO, ERR_ARCHIVO.
Resultado registrarVehiculo(const Vehiculo& datos, int& idAsignado);

// EXITO (activo) | ERR_ELIMINADO | ERR_NO_EXISTE | ERR_ARCHIVO.
// Lectura directa por calculo de posicion; posicion (opcional) recibe
// el byte donde se leyo.
Resultado obtenerVehiculoPorID(int id, Vehiculo& resultado, long* posicion = nullptr);

// Placa exacta (sin distinguir mayusculas). Recorre el archivo con un
// solo registro en memoria. Solo activos. EXITO | ERR_NO_EXISTE | ERR_ARCHIVO.
Resultado obtenerVehiculoPorPlaca(const char* placa, Vehiculo& resultado);

// Coincidencia parcial del nombre del propietario (sin distinguir
// mayusculas). 'resultados' debe tener espacio para MAX_RESULTADOS_BUSQUEDA.
Resultado buscarVehiculosPorPropietario(const char* texto, Vehiculo resultados[],
                                        int& cantidad, bool& hayMas);

// Cambia marca, modelo, anio y datos del propietario. NO cambia placa,
// kilometraje ni ID (se ignoran en 'nuevosDatos').
Resultado actualizarVehiculo(int id, const Vehiculo& nuevosDatos);

// Borrado logico. ERR_TIENE_ORDENES si tiene ordenes en cualquier estado.
Resultado eliminarVehiculo(int id);

// Para que la presentacion recorra el archivo por ID (1..total) sin
// cargarlo completo: total historico (incluye eliminados) y activos.
Resultado cantidadVehiculosRegistrados(int& total);
Resultado cantidadVehiculosActivos(int& activos);

#endif
