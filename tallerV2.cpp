#include <iostream>
#include <cstring>

using namespace std;

// ====================================================================
// 1. ESTRUCTURAS DE DATOS (EL MODELO)
// ====================================================================
struct Taller { char nombre[100]; char rif[20]; char direccion[200]; char telefono[20]; };
struct Vehiculo { int id; char placa[10]; char marca[50]; char modelo[50]; int anio; int kilometraje; char nombrePropietario[100]; char cedulaPropietario[20]; char telefonoPropietario[20]; char fechaRegistro[11]; };
struct Mecanico { int id; char nombre[100]; char cedula[20]; char especialidad[20]; char fechaIngreso[11]; };
struct Repuesto { int id; char codigo[20]; char nombre[100]; char marca[50]; float precio; int stock; int stockMinimo; char fechaRegistro[11]; };
struct DetalleOrden { int idRepuesto; int cantidad; float precioUnitario; };
struct OrdenServicio { int id; int idVehiculo; int idMecanico; char fechaApertura[11]; char fechaCierre[11]; char estado[12]; char descripcionFalla[200]; int kilometrajeIngreso; float manoObra; DetalleOrden* detalles; int numDetalles; int capacidadDetalles; };
struct SistemaTaller { Taller taller; Vehiculo* vehiculos; int numVehiculos; int capacidadVehiculos; Mecanico* mecanicos; int numMecanicos; int capacidadMecanicos; Repuesto* repuestos; int numRepuestos; int capacidadRepuestos; OrdenServicio* ordenes; int numOrdenes; int capacidadOrdenes; int siguienteIdVehiculo; int siguienteIdMecanico; int siguienteIdRepuesto; int siguienteIdOrden; };

// ====================================================================
// 2. FUNCIONES DE LA CAPA DE LÓGICA (Cero cin/cout)
// ====================================================================

void inicializarSistema(SistemaTaller* s, Taller datosTaller) {
    s->taller = datosTaller;
    s->numVehiculos = 0; s->numMecanicos = 0; s->numRepuestos = 0; s->numOrdenes = 0;
    s->capacidadVehiculos = 4; s->capacidadMecanicos = 4; s->capacidadRepuestos = 4; s->capacidadOrdenes = 4;
    s->vehiculos = new Vehiculo[s->capacidadVehiculos];
    s->mecanicos = new Mecanico[s->capacidadMecanicos];
    s->repuestos = new Repuesto[s->capacidadRepuestos];
    s->ordenes = new OrdenServicio[s->capacidadOrdenes];
    s->siguienteIdVehiculo = 1; s->siguienteIdMecanico = 1; s->siguienteIdRepuesto = 1; s->siguienteIdOrden = 1;
}

void liberarSistema(SistemaTaller* s) {
    delete[] s->vehiculos; delete[] s->mecanicos; delete[] s->repuestos;
    for (int i = 0; i < s->numOrdenes; i++) { delete[] s->ordenes[i].detalles; }
    delete[] s->ordenes; 
}

Vehiculo* agregarVehiculo(SistemaTaller* s, Vehiculo datos) {
    if (datos.anio < 1950 || datos.anio > 2027 || datos.kilometraje <= 0) return nullptr;
    for (int i = 0; i < s->numVehiculos; i++) {
        if (strcmp(datos.placa, s->vehiculos[i].placa) == 0) return nullptr;
    }
    if (s->numVehiculos == s->capacidadVehiculos) {
        s->capacidadVehiculos *= 2;
        Vehiculo* nuevoArreglo = new Vehiculo[s->capacidadVehiculos];
        for (int i = 0; i < s->numVehiculos; i++) nuevoArreglo[i] = s->vehiculos[i];
        delete[] s->vehiculos;
        s->vehiculos = nuevoArreglo;
    }
    datos.id = s->siguienteIdVehiculo++; strcpy(datos.fechaRegistro, "2026-10-16");
    s->vehiculos[s->numVehiculos] = datos; s->numVehiculos++;
    return &(s->vehiculos[s->numVehiculos - 1]);
}

bool validarEspecialidad(const char* especialidad) {
    return (strcmp(especialidad, "MOTOR") == 0 || strcmp(especialidad, "FRENOS") == 0 || strcmp(especialidad, "ELECTRICIDAD") == 0 || strcmp(especialidad, "SUSPENSION") == 0 || strcmp(especialidad, "GENERAL") == 0);
}

int contarOrdenesAbiertasMecanico(SistemaTaller s, int idMecanico) {
    int contador = 0;
    for (int i = 0; i < s.numOrdenes; i++) {
        if (s.ordenes[i].idMecanico == idMecanico && strcmp(s.ordenes[i].estado, "ABIERTA") == 0) contador++;
    }
    return contador;
}

Mecanico* buscarMecanicoPorCedula(SistemaTaller s, const char* cedula) {
    for (int i = 0; i < s.numMecanicos; i++) {
        if (strcmp(s.mecanicos[i].cedula, cedula) == 0) return &s.mecanicos[i];
    }
    return nullptr;
}

Mecanico* agregarMecanico(SistemaTaller* s, Mecanico datos) {
    if (buscarMecanicoPorCedula(*s, datos.cedula) != nullptr || !validarEspecialidad(datos.especialidad)) return nullptr; 
    if (s->numMecanicos >= s->capacidadMecanicos) {
        s->capacidadMecanicos *= 2;
        Mecanico* nuevoArreglo = new Mecanico[s->capacidadMecanicos];
        for (int i = 0; i < s->numMecanicos; i++) nuevoArreglo[i] = s->mecanicos[i];
        delete[] s->mecanicos; s->mecanicos = nuevoArreglo;
    }
    datos.id = s->siguienteIdMecanico++; s->mecanicos[s->numMecanicos] = datos; s->numMecanicos++;
    return &s->mecanicos[s->numMecanicos - 1];
}

Mecanico** listarMecanicos(SistemaTaller s, int* cantidad) {
    *cantidad = s.numMecanicos;
    if (*cantidad == 0) return nullptr;
    Mecanico** lista = new Mecanico*[*cantidad];
    for (int i = 0; i < *cantidad; i++) lista[i] = &s.mecanicos[i];
    return lista;
}

Repuesto* buscarRepuestoPorCodigo(SistemaTaller s, const char* codigo) {
    for (int i = 0; i < s.numRepuestos; i++) {
        if (strcmp(s.repuestos[i].codigo, codigo) == 0) return &s.repuestos[i];
    }
    return nullptr;
}

Repuesto* buscarRepuestoPorID(SistemaTaller s, int id) {
    for (int i = 0; i < s.numRepuestos; i++) {
        if (s.repuestos[i].id == id) return &s.repuestos[i];
    }
    return nullptr;
}

Repuesto* agregarRepuesto(SistemaTaller* s, Repuesto datos) {
    if (datos.precio <= 0 || datos.stock < 0 || datos.stockMinimo < 0 || buscarRepuestoPorCodigo(*s, datos.codigo) != nullptr) return nullptr;
    if (s->numRepuestos >= s->capacidadRepuestos) {
        s->capacidadRepuestos *= 2;
        Repuesto* nuevoArreglo = new Repuesto[s->capacidadRepuestos];
        for (int i = 0; i < s->numRepuestos; i++) nuevoArreglo[i] = s->repuestos[i];
        delete[] s->repuestos; s->repuestos = nuevoArreglo;
    }
    datos.id = s->siguienteIdRepuesto++; s->repuestos[s->numRepuestos] = datos; s->numRepuestos++;
    return &s->repuestos[s->numRepuestos - 1];
}

bool registrarEntradaRepuesto(SistemaTaller* s, int idRepuesto, int cantidadAgregar) {
    if (cantidadAgregar <= 0) return false;
    Repuesto* rep = buscarRepuestoPorID(*s, idRepuesto);
    if (rep != nullptr) { rep->stock += cantidadAgregar; return true; }
    return false;
}

Repuesto** listarRepuestos(SistemaTaller s, int* cantidad) {
    *cantidad = s.numRepuestos;
    if (*cantidad == 0) return nullptr;
    Repuesto** lista = new Repuesto*[*cantidad];
    for (int i = 0; i < *cantidad; i++) lista[i] = &s.repuestos[i];
    return lista;
}

OrdenServicio* abrirOrdenServicio(SistemaTaller* s, int idVehiculo, int idMecanico, const char* fechaApertura, const char* descripcionFalla) {
    if (contarOrdenesAbiertasMecanico(*s, idMecanico) >= 3) return nullptr;
    if (s->numOrdenes >= s->capacidadOrdenes) {
        s->capacidadOrdenes *= 2;
        OrdenServicio* nuevoArreglo = new OrdenServicio[s->capacidadOrdenes];
        for (int i = 0; i < s->numOrdenes; i++) nuevoArreglo[i] = s->ordenes[i];
        delete[] s->ordenes; s->ordenes = nuevoArreglo;
    }
    OrdenServicio nuevaOrden;
    nuevaOrden.id = s->siguienteIdOrden++; nuevaOrden.idVehiculo = idVehiculo; nuevaOrden.idMecanico = idMecanico;
    strcpy(nuevaOrden.fechaApertura, fechaApertura); strcpy(nuevaOrden.fechaCierre, "");
    strcpy(nuevaOrden.estado, "ABIERTA"); strcpy(nuevaOrden.descripcionFalla, descripcionFalla);
    nuevaOrden.kilometrajeIngreso = 0; nuevaOrden.manoObra = 0.0f;
    nuevaOrden.capacidadDetalles = 2; nuevaOrden.numDetalles = 0;
    nuevaOrden.detalles = new DetalleOrden[nuevaOrden.capacidadDetalles];
    s->ordenes[s->numOrdenes] = nuevaOrden; s->numOrdenes++;
    return &s->ordenes[s->numOrdenes - 1];
}

OrdenServicio* buscarOrdenPorID(SistemaTaller s, int idOrden) {
    for (int i = 0; i < s.numOrdenes; i++) {
        if (s.ordenes[i].id == idOrden) return &s.ordenes[i];
    }
    return nullptr;
}

bool agregarRepuestoAOrden(SistemaTaller* s, int idOrden, int idRepuesto, int cantidad) {
    OrdenServicio* orden = buscarOrdenPorID(*s, idOrden);
    if (orden == nullptr || strcmp(orden->estado, "ABIERTA") != 0) return false;
    Repuesto* rep = buscarRepuestoPorID(*s, idRepuesto);
    if (rep == nullptr || rep->stock < cantidad || cantidad <= 0) return false;
    if (orden->numDetalles >= orden->capacidadDetalles) {
        orden->capacidadDetalles *= 2;
        DetalleOrden* nuevosDetalles = new DetalleOrden[orden->capacidadDetalles];
        for (int i = 0; i < orden->numDetalles; i++) nuevosDetalles[i] = orden->detalles[i];
        delete[] orden->detalles; orden->detalles = nuevosDetalles;
    }
    rep->stock -= cantidad;
    DetalleOrden nuevoDetalle; nuevoDetalle.idRepuesto = idRepuesto; nuevoDetalle.cantidad = cantidad; nuevoDetalle.precioUnitario = rep->precio;
    orden->detalles[orden->numDetalles] = nuevoDetalle; orden->numDetalles++;
    return true;
}

bool cerrarOrdenServicio(SistemaTaller* s, int idOrden, float cobroManoObra, const char* fechaCierre) {
    OrdenServicio* orden = buscarOrdenPorID(*s, idOrden);
    if (orden == nullptr || strcmp(orden->estado, "ABIERTA") != 0 || cobroManoObra < 0) return false;
    orden->manoObra = cobroManoObra; strcpy(orden->fechaCierre, fechaCierre); strcpy(orden->estado, "CERRADA");
    return true;
}

float calcularTotalOrden(SistemaTaller s, int idOrden) {
    OrdenServicio* orden = buscarOrdenPorID(s, idOrden);
    if (orden == nullptr) return 0.0f;
    float totalRepuestos = 0.0f;
    for (int i = 0; i < orden->numDetalles; i++) totalRepuestos += (orden->detalles[i].cantidad * orden->detalles[i].precioUnitario);
    return orden->manoObra + totalRepuestos;
}

// ====================================================================
// 3. CAPA DE PRESENTACIÓN (Menús, cin, cout)
// ====================================================================

void menuRegistrarMecanico(SistemaTaller* s) {
    Mecanico m; cout << "\n--- REGISTRAR NUEVO MECANICO ---\n";
    cout << "Nombre completo: "; cin.ignore(); cin.getline(m.nombre, 100);
    cout << "Cedula: "; cin.getline(m.cedula, 20);
    cout << "Especialidad (MOTOR, FRENOS, ELECTRICIDAD, SUSPENSION, GENERAL): "; cin.getline(m.especialidad, 20);
    cout << "Fecha de ingreso (YYYY-MM-DD): "; cin.getline(m.fechaIngreso, 11);
    Mecanico* resultado = agregarMecanico(s, m);
    if (resultado != nullptr) cout << "¡Exito! Mecanico registrado con el ID #" << resultado->id << "\n";
    else cout << "ERROR: Cedula duplicada o especialidad invalida.\n";
}

void menuListarMecanicos(SistemaTaller* s) {
    int cantidad = 0; Mecanico** lista = listarMecanicos(*s, &cantidad);
    if (cantidad == 0 || lista == nullptr) { cout << "\nNo hay mecanicos registrados.\n"; return; }
    cout << "\n--- LISTA DE MECANICOS ---\n";
    for (int i = 0; i < cantidad; i++) {
        int abiertas = contarOrdenesAbiertasMecanico(*s, lista[i]->id);
        cout << "ID: " << lista[i]->id << " | Nombre: " << lista[i]->nombre << " | Cedula: " << lista[i]->cedula 
             << " | Especialidad: " << lista[i]->especialidad << " | Ordenes Abiertas: " << abiertas << "\n";
    }
    delete[] lista;
}

void menuRegistrarRepuesto(SistemaTaller* s) {
    Repuesto r; cout << "\n--- REGISTRAR NUEVO REPUESTO ---\n";
    cout << "Codigo (ej. FILT-001): "; cin.ignore(); cin.getline(r.codigo, 20);
    cout << "Nombre del repuesto: "; cin.getline(r.nombre, 100);
    cout << "Marca: "; cin.getline(r.marca, 50);
    cout << "Precio unitario ($): "; cin >> r.precio;
    cout << "Stock inicial: "; cin >> r.stock;
    cout << "Stock minimo (alerta): "; cin >> r.stockMinimo;
    cin.ignore(); cout << "Fecha de registro (YYYY-MM-DD): "; cin.getline(r.fechaRegistro, 11);
    Repuesto* res = agregarRepuesto(s, r);
    if (res != nullptr) cout << "¡Repuesto registrado exitosamente con ID #" << res->id << "!\n";
    else cout << "ERROR: Verifique valores o si el codigo ya existe.\n";
}

void menuListarRepuestos(SistemaTaller* s) {
    int cantidad = 0; Repuesto** lista = listarRepuestos(*s, &cantidad);
    if (cantidad == 0 || lista == nullptr) { cout << "\nNo hay repuestos registrados.\n"; return; }
    cout << "\n--- INVENTARIO DE REPUESTOS ---\n";
    for (int i = 0; i < cantidad; i++) {
        cout << "ID: " << lista[i]->id << " | Codigo: " << lista[i]->codigo << " | Nombre: " << lista[i]->nombre << " | Stock: " << lista[i]->stock;
        if (lista[i]->stock <= lista[i]->stockMinimo) cout << " [ALERTA: BAJO STOCK]";
        cout << "\n";
    }
    delete[] lista;
}

void menuEntradaMercancia(SistemaTaller* s) {
    int idRepuesto, cantidad; cout << "\n--- ENTRADA DE MERCANCIA ---\n";
    cout << "Ingrese el ID del repuesto: "; cin >> idRepuesto;
    cout << "Ingrese la cantidad a sumar al stock: "; cin >> cantidad;
    if (registrarEntradaRepuesto(s, idRepuesto, cantidad)) cout << "¡Stock actualizado exitosamente!\n";
    else cout << "ERROR: ID no encontrado o cantidad invalida.\n";
}

void menuAbrirOrden(SistemaTaller* s) {
    int idVehiculo, idMecanico; char fecha[11], falla[200];
    cout << "\n--- ABRIR ORDEN DE SERVICIO ---\n";
    cout << "ID del Vehiculo: "; cin >> idVehiculo;
    cout << "ID del Mecanico asignado: "; cin >> idMecanico;
    cout << "Fecha de Apertura (YYYY-MM-DD): "; cin >> fecha; cin.ignore();
    cout << "Descripcion de la Falla: "; cin.getline(falla, 200);
    OrdenServicio* orden = abrirOrdenServicio(s, idVehiculo, idMecanico, fecha, falla);
    if (orden != nullptr) cout << "¡Exito! Orden #" << orden->id << " abierta correctamente.\n";
    else cout << "ERROR: Mecanico excedio limite (3) de ordenes abiertas o datos invalidos.\n";
}

void menuAgregarRepuestoOrden(SistemaTaller* s) {
    int idOrden, idRepuesto, cant; cout << "\n--- AGREGAR REPUESTO A ORDEN ---\n";
    cout << "ID de la Orden: "; cin >> idOrden;
    cout << "ID del Repuesto a usar: "; cin >> idRepuesto;
    cout << "Cantidad a usar: "; cin >> cant;
    if (agregarRepuestoAOrden(s, idOrden, idRepuesto, cant)) cout << "Repuesto agregado y descontado del inventario.\n";
    else cout << "ERROR: Stock insuficiente, orden cerrada o ID invalido.\n";
}

void menuCerrarOrden(SistemaTaller* s) {
    int idOrden; float manoObra; char fecha[11];
    cout << "\n--- CERRAR ORDEN DE SERVICIO ---\n";
    cout << "ID de la Orden a cerrar: "; cin >> idOrden;
    cout << "Costo de Mano de Obra ($): "; cin >> manoObra;
    cout << "Fecha de Cierre (YYYY-MM-DD): "; cin >> fecha;
    if (cerrarOrdenServicio(s, idOrden, manoObra, fecha)) {
        float total = calcularTotalOrden(*s, idOrden);
        cout << "¡Orden CERRADA! El total a facturar al cliente es: $" << total << "\n";
    } else cout << "ERROR: No se pudo cerrar.\n";
}

// ====================================================================
// 4. FUNCIÓN PRINCIPAL (ENTRY POINT)
// ====================================================================

int main() {
    SistemaTaller miTaller;
    Taller datos = {"Taller Los Amigos", "J-12345678-9", "Maracaibo, Zulia", "0414-0000000"};
    inicializarSistema(&miTaller, datos);

    int opcion;
    do {
        cout << "\n=========================================\n";
        cout << "   SISTEMA DE GESTION: " << miTaller.taller.nombre << "\n";
        cout << "=========================================\n";
        cout << "1. Registrar Mecanico\n";
        cout << "2. Listar Mecanicos\n";
        cout << "3. Registrar Repuesto\n";
        cout << "4. Listar Inventario\n";
        cout << "5. Ingresar Mercancia (Stock)\n";
        cout << "6. Abrir Orden de Servicio\n";
        cout << "7. Agregar Repuesto a Orden\n";
        cout << "8. Cerrar Orden de Servicio\n";
        cout << "9. Salir\n";
        cout << "=========================================\n";
        cout << "Seleccione una opcion: ";
        cin >> opcion;

        switch (opcion) {
            case 1: menuRegistrarMecanico(&miTaller); break;
            case 2: menuListarMecanicos(&miTaller); break;
            case 3: menuRegistrarRepuesto(&miTaller); break;
            case 4: menuListarRepuestos(&miTaller); break;
            case 5: menuEntradaMercancia(&miTaller); break;
            case 6: menuAbrirOrden(&miTaller); break;
            case 7: menuAgregarRepuestoOrden(&miTaller); break;
            case 8: menuCerrarOrden(&miTaller); break;
            case 9: cout << "Cerrando el sistema y liberando memoria...\n"; break;
            default: cout << "Opcion invalida.\n";
        }
    } while (opcion != 9);

    liberarSistema(&miTaller);
    return 0;
}