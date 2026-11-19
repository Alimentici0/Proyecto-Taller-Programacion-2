// ====================================================================
// reportes.h
// CAPA DE LOGICA - REPORTES. Todos leen directamente de los archivos
// (un registro a la vez) y solo cuentan ordenes CERRADAS: las ABIERTAS y
// las ANULADAS no suman en ningun total.
// El Reporte 1 (historial de un vehiculo) se arma con
// obtenerVehiculoPorPlaca + buscarOrdenesDeVehiculo + obtenerVistaOrden
// + totalFacturadoVehiculo; aqui estan las partes que agregan datos.
// ====================================================================
#ifndef REPORTES_H
#define REPORTES_H

#include "persistencia.h"

const int TOP_REPUESTOS = 10;

// Reporte 1 (parte numerica): total facturado en ordenes CERRADAS del vehiculo
Resultado totalFacturadoVehiculo(int idVehiculo, float& total);

// Reporte 2: los repuestos con mas unidades consumidas en ordenes CERRADAS
struct FilaRepuestoMasUsado {
    int  idRepuesto;
    char codigo[20];
    char nombre[100];
    int  unidades;
};
// 'top' debe tener espacio para TOP_REPUESTOS. Entrega 'cantidad' filas
// (puede ser menos de 10), de mayor a menor; en empate va primero el ID menor.
Resultado reporteRepuestosMasUsados(FilaRepuestoMasUsado top[], int& cantidad);

// Reporte 3: facturacion de UN mecanico (la presentacion repite por cada mecanico)
struct FilaFacturacionMecanico {
    int   idMecanico;
    char  nombre[100];
    bool  deBaja;
    int   ordenesCerradas;
    float totalManoObra;
    float totalRepuestos;
};
Resultado reporteFacturacionMecanico(int idMecanico, FilaFacturacionMecanico& fila);

#endif
