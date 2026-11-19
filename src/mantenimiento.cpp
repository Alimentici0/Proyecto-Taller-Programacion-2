#include "mantenimiento.h"
#include "persistencia_vehiculos.h"
#include "persistencia_mecanicos.h"
#include "persistencia_repuestos.h"
#include "persistencia_ordenes.h"
#include "utilidades.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cstddef>

using namespace std;

// --------------------------------------------------------------------
// Auxiliares
// --------------------------------------------------------------------

// Anota un problema (con formato tipo printf). Cuenta todos, pero solo
// conserva el texto de los primeros MAX_PROBLEMAS_REPORTADOS.
static void registrarProblema(ReporteIntegridad& rep, const char* formato, ...) {
    rep.totalProblemas++;
    if (rep.mensajesGuardados >= MAX_PROBLEMAS_REPORTADOS) return;

    va_list args;
    va_start(args, formato);
    vsnprintf(rep.mensajes[rep.mensajesGuardados], LARGO_MENSAJE_PROBLEMA, formato, args);
    va_end(args);
    rep.mensajesGuardados++;
}

static const char* nombreDeArchivo(const char* ruta) {
    const char* barra = strrchr(ruta, '/');
    return barra ? barra + 1 : ruta;
}

// Funcion que se ejecuta sobre cada registro fisico (para las reglas propias de cada archivo)
typedef void (*VerificadorRegistro)(ReporteIntegridad& rep, const void* registro);

// Verificacion comun a los cuatro archivos de entidades:
//   - header vs contenido real (cantidad de registros y de activos)
//   - bytes sobrantes al final
//   - id == indice fisico + 1
// y luego llama al verificador propio de cada entidad.
static void verificarArchivo(ReporteIntegridad& rep, const char* ruta, size_t tamRegistro,
                             size_t offsetEliminado, int& verificados,
                             VerificadorRegistro verificador) {
    const char* nombre = nombreDeArchivo(ruta);

    ArchivoHeader h;
    if (leerHeader(ruta, h) != EXITO) {
        registrarProblema(rep, "ROTO: No se pudo leer el header de %s", nombre);
        return;
    }

    int real = 0;
    long sobrantes = 0;
    if (contarRegistrosEnDisco(ruta, tamRegistro, real, &sobrantes) != EXITO) {
        registrarProblema(rep, "ROTO: No se pudo medir el contenido de %s", nombre);
        return;
    }
    verificados = real;

    if (sobrantes != 0) {
        registrarProblema(rep, "ROTO: %s tiene %ld bytes sobrantes que no forman un registro completo",
                          nombre, sobrantes);
    }
    if (h.cantidadRegistros != real) {
        registrarProblema(rep, "ROTO: Header de %s indica %d registros; el archivo contiene %d",
                          nombre, h.cantidadRegistros, real);
    }

    alignas(8) char buffer[sizeof(OrdenServicio)];     // cabe el registro mas grande
    int activosContados = 0;
    for (int i = 0; i < real; i++) {
        if (leerRegistroFisico(ruta, tamRegistro, i, buffer) != EXITO) {
            registrarProblema(rep, "ROTO: No se pudo leer el registro fisico %d de %s", i, nombre);
            continue;
        }

        int idGuardado;
        memcpy(&idGuardado, buffer, sizeof(int));          // 'id' es el primer campo de las 4 estructuras
        if (idGuardado != i + 1) {
            registrarProblema(rep, "ROTO: El registro fisico %d de %s tiene ID %d (se esperaba %d)",
                              i, nombre, idGuardado, i + 1);
        }

        bool eliminado;
        memcpy(&eliminado, buffer + offsetEliminado, sizeof(bool));
        if (!eliminado) activosContados++;

        if (verificador) verificador(rep, buffer);
    }

    if (h.registrosActivos != activosContados) {
        registrarProblema(rep, "ROTO: Header de %s indica %d registros activos; el archivo contiene %d",
                          nombre, h.registrosActivos, activosContados);
    }
}

// --------------------------------------------------------------------
// Reglas propias de cada archivo
// --------------------------------------------------------------------

// d) repuestos con stock negativo
static void verificarRepuesto(ReporteIntegridad& rep, const void* registro) {
    const Repuesto& r = *static_cast<const Repuesto*>(registro);
    if (r.stock < 0) {
        registrarProblema(rep, "ROTO: Repuesto ID %d (%s) tiene stock negativo (%d)", r.id, r.codigo, r.stock);
    }
}

// a), b) y c) sobre cada orden
static void verificarOrden(ReporteIntegridad& rep, const void* registro) {
    const OrdenServicio& o = *static_cast<const OrdenServicio*>(registro);
    bool abierta = (strcmp(o.estado, ESTADO_ABIERTA) == 0);

    if (!abierta && strcmp(o.estado, ESTADO_CERRADA) != 0 && strcmp(o.estado, ESTADO_ANULADA) != 0) {
        registrarProblema(rep, "ROTO: Orden #%d tiene un estado desconocido ('%s')", o.id, o.estado);
    }

    // Vehiculo
    Vehiculo veh;
    Resultado r = leerVehiculoPorID(o.idVehiculo, veh);
    if (r == ERR_NO_EXISTE) {
        registrarProblema(rep, "ROTO: Orden #%d apunta al vehiculo ID %d (no existe en vehiculos.bin)",
                          o.id, o.idVehiculo);
    } else if (r != EXITO) {
        registrarProblema(rep, "ROTO: No se pudo leer vehiculos.bin al verificar la orden #%d", o.id);
    } else if (abierta && veh.eliminado) {
        registrarProblema(rep, "ROTO: Orden #%d esta ABIERTA pero su vehiculo ID %d (%s) esta eliminado",
                          o.id, o.idVehiculo, veh.placa);
    }

    // Mecanico (en ordenes CERRADAS o ANULADAS estar de baja es valido)
    Mecanico mec;
    r = leerMecanicoPorID(o.idMecanico, mec);
    if (r == ERR_NO_EXISTE) {
        registrarProblema(rep, "ROTO: Orden #%d apunta al mecanico ID %d (no existe en mecanicos.bin)",
                          o.id, o.idMecanico);
    } else if (r != EXITO) {
        registrarProblema(rep, "ROTO: No se pudo leer mecanicos.bin al verificar la orden #%d", o.id);
    } else if (abierta && mec.eliminado) {
        registrarProblema(rep, "ROTO: Orden #%d esta ABIERTA pero su mecanico ID %d (%s) esta dado de baja",
                          o.id, o.idMecanico, mec.nombre);
    }

    // Lineas de detalle
    if (o.numLineas < 0 || o.numLineas > MAX_LINEAS_ORDEN) {
        registrarProblema(rep, "ROTO: Orden #%d indica %d lineas de detalle (el maximo es %d)",
                          o.id, o.numLineas, MAX_LINEAS_ORDEN);
        return;
    }
    for (int i = 0; i < o.numLineas; i++) {
        Repuesto repuesto;
        r = leerRepuestoPorID(o.lineas[i].idRepuesto, repuesto);
        if (r == ERR_NO_EXISTE) {
            registrarProblema(rep, "ROTO: Orden #%d, linea %d: el repuesto ID %d no existe en repuestos.bin",
                              o.id, i + 1, o.lineas[i].idRepuesto);
        } else if (r != EXITO) {
            registrarProblema(rep, "ROTO: No se pudo leer repuestos.bin al verificar la orden #%d", o.id);
        }
    }
}

// --------------------------------------------------------------------
// Verificacion de integridad
// --------------------------------------------------------------------
Resultado verificarIntegridad(ReporteIntegridad& reporte) {
    memset(&reporte, 0, sizeof(ReporteIntegridad));

    verificarArchivo(reporte, RUTA_VEHICULOS, sizeof(Vehiculo), offsetof(Vehiculo, eliminado),
                     reporte.vehiculosVerificados, nullptr);
    verificarArchivo(reporte, RUTA_MECANICOS, sizeof(Mecanico), offsetof(Mecanico, eliminado),
                     reporte.mecanicosVerificados, nullptr);
    verificarArchivo(reporte, RUTA_REPUESTOS, sizeof(Repuesto), offsetof(Repuesto, eliminado),
                     reporte.repuestosVerificados, verificarRepuesto);
    verificarArchivo(reporte, RUTA_ORDENES, sizeof(OrdenServicio), offsetof(OrdenServicio, eliminado),
                     reporte.ordenesVerificadas, verificarOrden);
    return EXITO;
}

// --------------------------------------------------------------------
// Respaldo
// --------------------------------------------------------------------
Resultado crearBackup(char* rutaBackup, size_t capacidad) {
    char marca[32];
    obtenerMarcaTiempoBackup(marca, sizeof(marca));
    snprintf(rutaBackup, capacidad, "backup_%s", marca);

    if (crearDirectorio(rutaBackup) != EXITO) return ERR_ARCHIVO;

    for (int i = 0; i < CANTIDAD_ARCHIVOS_BIN; i++) {
        char destino[300];
        snprintf(destino, sizeof(destino), "%s/%s", rutaBackup, nombreDeArchivo(ARCHIVOS_BIN[i]));
        if (copiarArchivoBinario(ARCHIVOS_BIN[i], destino) != EXITO) return ERR_ARCHIVO;
    }
    return EXITO;
}
