// ====================================================================
// mantenimiento.h
// CAPA DE LOGICA - MANTENIMIENTO: verificacion de integridad y respaldo.
// ====================================================================
#ifndef MANTENIMIENTO_H
#define MANTENIMIENTO_H

#include <cstddef>
#include "persistencia.h"

const int MAX_PROBLEMAS_REPORTADOS = 100;     // mensajes que se conservan en el reporte
const int LARGO_MENSAJE_PROBLEMA = 160;

// Resultado de verificarIntegridad. La logica no imprime: deja aqui los
// mensajes y los contadores y la presentacion los muestra.
struct ReporteIntegridad {
    int vehiculosVerificados;
    int mecanicosVerificados;
    int repuestosVerificados;
    int ordenesVerificadas;

    int  totalProblemas;                                      // todos los encontrados
    int  mensajesGuardados;                                   // los que caben en 'mensajes'
    char mensajes[MAX_PROBLEMAS_REPORTADOS][LARGO_MENSAJE_PROBLEMA];
};

// Recorre los archivos (un registro en memoria a la vez) y registra TODO
// lo que encuentre fuera de lugar:
//   a) ordenes cuyo vehiculo o mecanico no existe
//   b) ordenes ABIERTAS con vehiculo o mecanico no activo
//      (en CERRADAS / ANULADAS un mecanico de baja es valido)
//   c) lineas de detalle cuyo repuesto no existe
//   d) repuestos con stock negativo
//   e) headers que no coinciden con el contenido real del archivo
// Ademas comprueba que el registro fisico i tenga id == i + 1 (la
// relacion que hace posible el acceso directo por ID).
// Devuelve EXITO si pudo hacer la verificacion (aunque encuentre
// problemas); ERR_ARCHIVO si no pudo ni empezar.
Resultado verificarIntegridad(ReporteIntegridad& reporte);

// Crea la carpeta backup_AAAA-MM-DD_HH-MM y copia byte a byte todos los
// .bin del sistema. La ruta creada se entrega en rutaBackup.
// Si falla alguna copia devuelve ERR_ARCHIVO.
Resultado crearBackup(char* rutaBackup, size_t capacidad);

#endif
