// ====================================================================
// logica_repuestos.h
// CAPA DE LOGICA - REPUESTOS / INVENTARIO (convenciones: ver logica_vehiculos.h)
// ====================================================================
#ifndef LOGICA_REPUESTOS_H
#define LOGICA_REPUESTOS_H

#include "persistencia_repuestos.h"

// Codigo unico entre los activos (se guarda en mayusculas). Precio > 0,
// stock >= 0, stock minimo >= 0.
// Errores: ERR_DATOS_INVALIDOS, ERR_PRECIO_INVALIDO, ERR_STOCK_INVALIDO,
//          ERR_DUPLICADO, ERR_ARCHIVO.
Resultado registrarRepuesto(const Repuesto& datos, int& idAsignado);

// EXITO (activo) | ERR_ELIMINADO | ERR_NO_EXISTE | ERR_ARCHIVO
Resultado obtenerRepuestoPorID(int id, Repuesto& resultado);

// Codigo exacto (sin distinguir mayusculas). Solo activos.
Resultado obtenerRepuestoPorCodigo(const char* codigo, Repuesto& resultado);

// Nombre parcial (sin distinguir mayusculas). Solo activos.
// 'resultados' debe tener espacio para MAX_RESULTADOS_BUSQUEDA.
Resultado buscarRepuestosPorNombre(const char* texto, Repuesto resultados[],
                                   int& cantidad, bool& hayMas);

// Suma unidades al stock. ERR_CANTIDAD_INVALIDA si cantidad <= 0.
Resultado registrarEntradaRepuesto(int id, int cantidad);

// Nuevo precio > 0. No afecta a las ordenes ya registradas, porque cada
// linea guarda su propio precio historico.
Resultado cambiarPrecioRepuesto(int id, float nuevoPrecio);

// Borrado logico. ERR_REPUESTO_EN_USO si aparece en el detalle de
// cualquier orden (abierta, cerrada o anulada).
Resultado eliminarRepuesto(int id);

// true si stock <= stockMinimo (se muestra como BAJO STOCK)
bool repuestoBajoStock(const Repuesto& repuesto);

Resultado cantidadRepuestosRegistrados(int& total);
Resultado cantidadRepuestosActivos(int& activos);

#endif
