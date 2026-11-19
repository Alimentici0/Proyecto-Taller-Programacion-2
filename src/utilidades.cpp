#include "utilidades.h"

#include <cstring>
#include <cctype>
#include <cstdio>
#include <ctime>

using namespace std;

void copiarTexto(char* destino, size_t capacidad, const char* origen) {
    memset(destino, 0, capacidad);
    strncpy(destino, origen, capacidad - 1);
}

void aMayusculas(char* texto) {
    for (size_t i = 0; texto[i] != '\0'; i++) {
        texto[i] = static_cast<char>(toupper(static_cast<unsigned char>(texto[i])));
    }
}

void recortarEspacios(char* texto) {
    size_t inicio = 0;
    while (texto[inicio] != '\0' && isspace(static_cast<unsigned char>(texto[inicio]))) inicio++;

    size_t fin = strlen(texto);
    while (fin > inicio && isspace(static_cast<unsigned char>(texto[fin - 1]))) fin--;

    size_t largo = fin - inicio;
    memmove(texto, texto + inicio, largo);
    texto[largo] = '\0';
}

bool textoVacio(const char* texto) {
    for (size_t i = 0; texto[i] != '\0'; i++) {
        if (!isspace(static_cast<unsigned char>(texto[i]))) return false;
    }
    return true;
}

bool igualesIgnorandoMayusculas(const char* a, const char* b) {
    size_t i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (toupper(static_cast<unsigned char>(a[i])) != toupper(static_cast<unsigned char>(b[i]))) return false;
        i++;
    }
    return a[i] == b[i];       // ambas terminaron a la vez
}

bool contieneIgnorandoMayusculas(const char* texto, const char* patron) {
    size_t largoPatron = strlen(patron);
    size_t largoTexto = strlen(texto);
    if (largoPatron == 0) return true;
    if (largoPatron > largoTexto) return false;

    for (size_t i = 0; i + largoPatron <= largoTexto; i++) {
        size_t j = 0;
        while (j < largoPatron &&
               toupper(static_cast<unsigned char>(texto[i + j])) ==
               toupper(static_cast<unsigned char>(patron[j]))) {
            j++;
        }
        if (j == largoPatron) return true;
    }
    return false;
}

bool fechaValida(const char* fecha) {
    if (strlen(fecha) != 10) return false;
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) {
            if (fecha[i] != '-') return false;
        } else if (!isdigit(static_cast<unsigned char>(fecha[i]))) {
            return false;
        }
    }
    int anio = (fecha[0] - '0') * 1000 + (fecha[1] - '0') * 100 + (fecha[2] - '0') * 10 + (fecha[3] - '0');
    int mes  = (fecha[5] - '0') * 10 + (fecha[6] - '0');
    int dia  = (fecha[8] - '0') * 10 + (fecha[9] - '0');
    if (anio < 1900 || mes < 1 || mes > 12 || dia < 1) return false;

    int diasPorMes[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    bool bisiesto = (anio % 4 == 0 && anio % 100 != 0) || (anio % 400 == 0);
    int maximo = diasPorMes[mes - 1] + ((mes == 2 && bisiesto) ? 1 : 0);
    return dia <= maximo;
}

void obtenerFechaActual(char* fecha) {
    time_t ahora = time(nullptr);
    tm* local = localtime(&ahora);
    strftime(fecha, 11, "%Y-%m-%d", local);
}

void obtenerMarcaTiempoBackup(char* marca, size_t capacidad) {
    time_t ahora = time(nullptr);
    tm* local = localtime(&ahora);
    strftime(marca, capacidad, "%Y-%m-%d_%H-%M", local);
}
