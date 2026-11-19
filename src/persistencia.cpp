// ====================================================================
// persistencia.cpp
// Operaciones comunes a todos los archivos: utilidades, header y
// calculo de posiciones. Los registros de cada entidad estan en
// persistencia_<entidad>.cpp.
// ====================================================================
#include "persistencia.h"

#include <fstream>
#include <filesystem>

using namespace std;

// --------------------------------------------------------------------
// Utilidades de archivos
// --------------------------------------------------------------------
bool existeArchivo(const char* ruta) {
    ifstream f(ruta, ios::binary);
    return f.good();
}

Resultado crearDirectorio(const char* ruta) {
    error_code ec;
    filesystem::create_directories(ruta, ec);   // no es error si ya existe
    return ec ? ERR_ARCHIVO : EXITO;
}

// Copia byte a byte (en bloques de 4 KB) usando solo ifstream / ofstream
Resultado copiarArchivoBinario(const char* origen, const char* destino) {
    ifstream in(origen, ios::binary);
    if (!in) return ERR_ARCHIVO;
    ofstream out(destino, ios::binary | ios::trunc);
    if (!out) return ERR_ARCHIVO;

    char bloque[4096];
    while (in.read(bloque, sizeof(bloque)) || in.gcount() > 0) {
        out.write(bloque, in.gcount());
        if (!out) return ERR_ARCHIVO;
    }
    out.flush();
    return out ? EXITO : ERR_ARCHIVO;
}

// --------------------------------------------------------------------
// Header
// --------------------------------------------------------------------
Resultado leerHeader(const char* ruta, ArchivoHeader& header) {
    ifstream f(ruta, ios::binary);
    if (!f) return ERR_ARCHIVO;

    f.seekg(0);
    f.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
    if (!f) return ERR_ARCHIVO;           // archivo mas corto que un header
    return EXITO;
}

Resultado actualizarHeader(const char* ruta, const ArchivoHeader& header) {
    // in | out para NO truncar el archivo: solo se pisan los primeros bytes
    fstream f(ruta, ios::in | ios::out | ios::binary);
    if (!f) return ERR_ARCHIVO;

    f.seekp(0);
    f.write(reinterpret_cast<const char*>(&header), sizeof(ArchivoHeader));
    f.flush();
    return f ? EXITO : ERR_ARCHIVO;
}

Resultado inicializarArchivo(const char* ruta) {
    if (!existeArchivo(ruta)) {
        ofstream out(ruta, ios::binary);
        if (!out) return ERR_ARCHIVO;

        ArchivoHeader inicial;
        inicial.cantidadRegistros = 0;
        inicial.proximoID = 1;
        inicial.registrosActivos = 0;
        inicial.version = 1;

        out.write(reinterpret_cast<const char*>(&inicial), sizeof(ArchivoHeader));
        out.flush();
        if (!out) return ERR_ARCHIVO;
    }

    // Exista de antes o recien creado, debe poder leerse su header
    ArchivoHeader comprobacion;
    return leerHeader(ruta, comprobacion);
}

Resultado inicializarTodosLosArchivos(const char*& rutaConProblema) {
    rutaConProblema = CARPETA_DATOS;
    if (crearDirectorio(CARPETA_DATOS) != EXITO) return ERR_ARCHIVO;

    const char* entidades[] = { RUTA_VEHICULOS, RUTA_MECANICOS, RUTA_REPUESTOS, RUTA_ORDENES };
    for (const char* ruta : entidades) {
        rutaConProblema = ruta;
        if (inicializarArchivo(ruta) != EXITO) return ERR_ARCHIVO;
    }
    rutaConProblema = nullptr;
    return EXITO;
}

// --------------------------------------------------------------------
// Posiciones
// --------------------------------------------------------------------
long posicionRegistro(int indiceFisico, size_t tamRegistro) {
    return static_cast<long>(sizeof(ArchivoHeader))
         + static_cast<long>(indiceFisico) * static_cast<long>(tamRegistro);
}

long posicionPorID(int id, size_t tamRegistro) {
    return posicionRegistro(id - 1, tamRegistro);
}

// --------------------------------------------------------------------
// Mantenimiento
// --------------------------------------------------------------------
Resultado contarRegistrosEnDisco(const char* ruta, size_t tamRegistro,
                                 int& cantidad, long* bytesSobrantes) {
    ifstream f(ruta, ios::binary | ios::ate);     // ate: empieza al final
    if (!f) return ERR_ARCHIVO;

    long tamanoTotal = static_cast<long>(f.tellg());
    long tamanoHeader = static_cast<long>(sizeof(ArchivoHeader));
    if (tamanoTotal < tamanoHeader) return ERR_ARCHIVO;

    long bytesDeRegistros = tamanoTotal - tamanoHeader;
    cantidad = static_cast<int>(bytesDeRegistros / static_cast<long>(tamRegistro));
    if (bytesSobrantes) *bytesSobrantes = bytesDeRegistros % static_cast<long>(tamRegistro);
    return EXITO;
}

Resultado leerRegistroFisico(const char* ruta, size_t tamRegistro,
                             int indiceFisico, void* destino) {
    ifstream f(ruta, ios::binary);
    if (!f) return ERR_ARCHIVO;

    f.seekg(posicionRegistro(indiceFisico, tamRegistro));
    f.read(static_cast<char*>(destino), static_cast<streamsize>(tamRegistro));
    return f ? EXITO : ERR_ARCHIVO;
}
