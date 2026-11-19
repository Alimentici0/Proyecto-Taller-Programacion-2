// ====================================================================
// resultado.h
// Codigo de resultado que devuelven las operaciones que pueden fallar.
//
// Decision de diseno: en vez de devolver bool (que no dice POR QUE
// fallo) o punteros (que ya no existen en el Proyecto 2), cada
// operacion devuelve un Resultado. EXITO significa que todo salio bien;
// cualquier otro valor indica la causa. Los datos de salida (el
// registro leido, el ID asignado, etc.) viajan en parametros por
// referencia. La presentacion traduce el codigo a un mensaje.
// ====================================================================
#ifndef RESULTADO_H
#define RESULTADO_H

enum Resultado {
    EXITO = 0,

    // --- Errores de archivo / existencia ---
    ERR_ARCHIVO,                    // no se pudo abrir, leer o escribir un archivo
    ERR_NO_EXISTE,                  // el ID no existe en el archivo
    ERR_ELIMINADO,                  // el registro existe pero esta eliminado (o de baja)

    // --- Validacion de datos ---
    ERR_DATOS_INVALIDOS,            // campo obligatorio vacio
    ERR_ANIO_INVALIDO,
    ERR_KILOMETRAJE_INVALIDO,
    ERR_DUPLICADO,                  // placa, cedula o codigo ya registrados
    ERR_ESPECIALIDAD_INVALIDA,
    ERR_FECHA_INVALIDA,
    ERR_PRECIO_INVALIDO,
    ERR_STOCK_INVALIDO,
    ERR_CANTIDAD_INVALIDA,

    // --- Reglas de integridad al eliminar ---
    ERR_TIENE_ORDENES,              // vehiculo con ordenes
    ERR_TIENE_ORDENES_ABIERTAS,     // mecanico con ordenes abiertas
    ERR_REPUESTO_EN_USO,            // repuesto presente en el detalle de alguna orden

    // --- Ordenes de servicio ---
    ERR_VEHICULO_NO_EXISTE,
    ERR_VEHICULO_ELIMINADO,
    ERR_MECANICO_NO_EXISTE,
    ERR_MECANICO_ELIMINADO,
    ERR_VEHICULO_CON_ORDEN_ABIERTA,
    ERR_MECANICO_LIMITE_ORDENES,
    ERR_ORDEN_NO_EXISTE,
    ERR_ESTADO_INCORRECTO,
    ERR_REPUESTO_NO_EXISTE,
    ERR_REPUESTO_ELIMINADO,
    ERR_STOCK_INSUFICIENTE,
    ERR_LIMITE_LINEAS,
    ERR_LINEA_NO_EXISTE,
    ERR_ORDEN_VACIA,
    ERR_MANO_OBRA_INVALIDA
};

#endif
