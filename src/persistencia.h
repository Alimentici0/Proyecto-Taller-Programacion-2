// ====================================================================
// persistencia.h
// CAPA DE PERSISTENCIA: lee y escribe bytes en archivos binarios.
//
// Reglas de esta capa:
//   - NO usa cin / cout (los mensajes son cosa de la presentacion).
//   - NO aplica reglas de negocio ni filtra registros eliminados.
//   - Usa solo <fstream> (ifstream, ofstream, fstream, seekg, seekp,
//     read, write). Nada de fopen / fclose.
// ====================================================================
#ifndef PERSISTENCIA_H
#define PERSISTENCIA_H

#include <cstddef>
#include "estructuras.h"
#include "resultado.h"

// --------------------------------------------------------------------
// Rutas de los archivos
// --------------------------------------------------------------------
const char* const CARPETA_DATOS   = "datos";
const char* const RUTA_TALLER     = "datos/taller.bin";
const char* const RUTA_VEHICULOS  = "datos/vehiculos.bin";
const char* const RUTA_MECANICOS  = "datos/mecanicos.bin";
const char* const RUTA_REPUESTOS  = "datos/repuestos.bin";
const char* const RUTA_ORDENES    = "datos/ordenes.bin";

// Lista de todos los .bin del sistema (para backup y verificacion)
const int CANTIDAD_ARCHIVOS_BIN = 5;
const char* const ARCHIVOS_BIN[CANTIDAD_ARCHIVOS_BIN] = {
    RUTA_TALLER, RUTA_VEHICULOS, RUTA_MECANICOS, RUTA_REPUESTOS, RUTA_ORDENES
};

// --------------------------------------------------------------------
// Utilidades de archivos
// --------------------------------------------------------------------
bool      existeArchivo(const char* ruta);
Resultado crearDirectorio(const char* ruta);
Resultado copiarArchivoBinario(const char* origen, const char* destino);

// --------------------------------------------------------------------
// 4.1 Operaciones sobre el HEADER (sirven para cualquier archivo de
//     entidades, porque todos usan el mismo ArchivoHeader)
// --------------------------------------------------------------------

// Si el archivo no existe lo crea con el header inicial
// (0 registros, proximoID = 1, 0 activos, version 1). Si ya existe no
// lo modifica. Devuelve EXITO solo si el archivo quedo listo para usarse.
Resultado inicializarArchivo(const char* ruta);

// Lee unicamente el header (posicion 0), sin leer ningun registro.
Resultado leerHeader(const char* ruta, ArchivoHeader& header);

// Sobrescribe unicamente el header (posicion 0), sin tocar los registros.
Resultado actualizarHeader(const char* ruta, const ArchivoHeader& header);

// Crea la carpeta datos/ y los cuatro archivos de entidades que falten.
// Si falla, rutaConProblema apunta al archivo que causo el error.
Resultado inicializarTodosLosArchivos(const char*& rutaConProblema);

// --------------------------------------------------------------------
// 4.2 Calculo de posiciones (acceso aleatorio)
//
//   posicion = sizeof(ArchivoHeader) + indiceFisico * sizeof(Estructura)
//   indiceFisico = id - 1      (los IDs son 1, 2, 3... sin huecos)
// --------------------------------------------------------------------
long posicionRegistro(int indiceFisico, size_t tamRegistro);
long posicionPorID(int id, size_t tamRegistro);

// --------------------------------------------------------------------
// Herramientas de bajo nivel para MANTENIMIENTO (verificarIntegridad).
// Miran el archivo "tal como esta en disco", sin fiarse del header.
// --------------------------------------------------------------------

// Cuenta cuantos registros caben realmente en el archivo segun su
// tamano en bytes. bytesSobrantes (opcional) recibe los bytes que no
// forman un registro completo (deberia ser 0).
Resultado contarRegistrosEnDisco(const char* ruta, size_t tamRegistro,
                                 int& cantidad, long* bytesSobrantes = nullptr);

// Lee el registro del indice fisico dado (0, 1, 2...) dentro de 'destino'.
Resultado leerRegistroFisico(const char* ruta, size_t tamRegistro,
                             int indiceFisico, void* destino);

#endif
