// ====================================================================
// logica_taller.h
// CAPA DE LOGICA - TALLER (taller.bin: un unico registro)
// ====================================================================
#ifndef LOGICA_TALLER_H
#define LOGICA_TALLER_H

#include "persistencia_taller.h"

// true si taller.bin ya existe (si no, es el primer arranque)
bool tallerRegistrado();

// Registra los datos del taller (nombre y RIF obligatorios).
// Errores: ERR_DATOS_INVALIDOS, ERR_ARCHIVO.
Resultado registrarTaller(const Taller& datos);

Resultado cargarTaller(Taller& resultado);

#endif
