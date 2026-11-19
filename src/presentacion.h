// ====================================================================
// presentacion.h
// CAPA DE PRESENTACION: todo lo que usa cin / cout.
// Regla: aqui NO se abre ningun archivo. Para obtener o guardar datos
// se llama a la capa de logica, que devuelve un Resultado.
// ====================================================================
#pragma once

#include <cstddef>
#include "resultado.h"
#include "estructuras.h"

// Si es true, la presentacion muestra tambien la posicion en bytes donde
// se leyo un registro buscado por ID (mensaje de depuracion del Caso 2).
const bool MODO_DEPURACION = true;

// ---- presentacion_comun.cpp: entrada, salida y mensajes ----
void activarEcoEntrada(bool activo);                       // repite lo leido (para pruebas con archivos)
bool leerLinea(const char* mensaje, char* destino, size_t capacidad);
bool leerEntero(const char* mensaje, int& valor);
bool leerDecimal(const char* mensaje, float& valor);
const char* mensajeResultado(Resultado r);
void mostrarError(Resultado r);
void imprimirLineaCaja(const char* texto);
void imprimirBordeCaja(char relleno);

// ---- presentacion_menu.cpp: arranque y menu principal ----
bool pedirYRegistrarTaller();
void ejecutarMenuPrincipal(const char* nombreTaller);

// ---- presentacion_vehiculos.cpp ----
void menuVehiculos();

// ---- presentacion_mecanicos.cpp ----
void menuMecanicos();

// ---- presentacion_repuestos.cpp ----
void menuRepuestos();
