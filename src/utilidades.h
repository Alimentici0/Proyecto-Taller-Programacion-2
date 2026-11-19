// ====================================================================
// utilidades.h
// Funciones auxiliares de texto (char[]) y fechas. Sin cin/cout y sin
// archivos: se pueden usar desde cualquier capa.
// ====================================================================
#ifndef UTILIDADES_H
#define UTILIDADES_H

#include <cstddef>

// Copia 'origen' en 'destino' sin desbordarlo: deja el resto del
// arreglo en ceros (asi los bytes que se escriben a disco son
// deterministas) y siempre termina en '\0'.
void copiarTexto(char* destino, size_t capacidad, const char* origen);

void aMayusculas(char* texto);
void recortarEspacios(char* texto);                  // quita espacios al inicio y al final
bool textoVacio(const char* texto);                  // "" o solo espacios

bool igualesIgnorandoMayusculas(const char* a, const char* b);
bool contieneIgnorandoMayusculas(const char* texto, const char* patron);   // coincidencia parcial

// Fechas en formato YYYY-MM-DD (11 chars con el '\0')
bool fechaValida(const char* fecha);
void obtenerFechaActual(char* fecha);                // fecha debe tener >= 11 bytes

// "2026-11-03_14-32" (para el nombre de la carpeta de respaldo)
void obtenerMarcaTiempoBackup(char* marca, size_t capacidad);

#endif
