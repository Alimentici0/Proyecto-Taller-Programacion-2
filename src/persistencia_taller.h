// ====================================================================
// persistencia_taller.h
// taller.bin: UN unico registro Taller, SIN header.
// Se crea en el primer arranque y despues solo se lee.
// ====================================================================
#ifndef PERSISTENCIA_TALLER_H
#define PERSISTENCIA_TALLER_H

#include "persistencia.h"

bool existeTallerGuardado();

// Lee el unico registro del archivo.
Resultado leerTaller(Taller& destino);

// Escribe el registro del taller. Si el archivo no existia asigna
// fechaCreacion; si existia conserva la fechaCreacion original.
// En ambos casos actualiza fechaUltimaModificacion.
Resultado guardarTaller(const Taller& datos);

#endif
