#!/usr/bin/env bash
# ====================================================================
# ejecutar_pruebas.sh
# Ejecuta los casos de uso del enunciado y guarda las salidas.
#
#   entrada_NX.txt  -> lo que se "teclea" en la ejecucion X del caso N
#   salida_NX.txt   -> lo que el programa mostro (generado por este script)
#
# Cada caso corre en su propia carpeta pruebas/trabajo/casoN/, asi las
# pruebas NO tocan la carpeta datos/ del proyecto. Las ejecuciones a, b, c
# de un caso comparten esa carpeta: es lo que permite comprobar que los
# datos sobreviven al cerrar y volver a abrir el programa.
#
# Uso (desde cualquier carpeta):   ./pruebas/ejecutar_pruebas.sh
# Requiere: make y g++ (compila el programa si hace falta).
# ====================================================================
set -u
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
PRUEBAS="$RAIZ/pruebas"
TRABAJO="$PRUEBAS/trabajo"

make -C "$RAIZ" taller > /dev/null || { echo "No se pudo compilar el programa"; exit 1; }
EXE="$RAIZ/taller"

# Caso que parte de cero (el programa pide los datos del taller al arrancar)
preparar_vacio()   { rm -rf "$TRABAJO/caso$1"; mkdir -p "$TRABAJO/caso$1"; }
# Caso que parte de los datos pre-poblados de datos/
preparar_poblado() { preparar_vacio "$1"; cp -r "$RAIZ/datos" "$TRABAJO/caso$1/datos"; }

# ejecutar <caso> <letra>: una ejecucion del programa con su archivo de entrada
ejecutar() {
    ( cd "$TRABAJO/caso$1" && "$EXE" --eco < "$PRUEBAS/entrada_$1$2.txt" > "$PRUEBAS/salida_$1$2.txt" 2>&1 )
    echo "  entrada_$1$2.txt -> salida_$1$2.txt"
}

echo "Caso 1: persistencia entre ejecuciones";           preparar_vacio 1;   ejecutar 1 a; ejecutar 1 b
echo "Caso 2: acceso directo por ID";                    preparar_vacio 2;   ejecutar 2 a
echo "Caso 3: la orden sobrevive con su detalle";        preparar_poblado 3; ejecutar 3 a; ejecutar 3 b; ejecutar 3 c
echo "Caso 4: orden con 15 lineas";                      preparar_vacio 4;   ejecutar 4 a; ejecutar 4 b
echo "Caso 5: anulacion con reversion en disco";         preparar_poblado 5; ejecutar 5 a; ejecutar 5 b
echo "Caso 6: baja de mecanico e historial";             preparar_poblado 6; ejecutar 6 a; ejecutar 6 b

echo "Caso 7: integridad tras una restauracion parcial"; preparar_poblado 7; ejecutar 7 a
# Paso 3 del caso: reemplazar MANUALMENTE vehiculos.bin por la copia del backup
echo "  (manual) cp backup_*/vehiculos.bin datos/vehiculos.bin"
cp "$TRABAJO"/caso7/backup_*/vehiculos.bin "$TRABAJO/caso7/datos/vehiculos.bin"
ejecutar 7 b

echo "Caso 8: reportes";                                 preparar_poblado 8; ejecutar 8 a
echo "Caso 9 (extra): validaciones y mensajes de error"; preparar_poblado 9; ejecutar 9 a
echo "Caso 10 (extra): limite de lineas de la orden";    preparar_vacio 10;  ejecutar 10 a; ejecutar 10 b

echo "Listo. Las salidas quedaron en $PRUEBAS/"
