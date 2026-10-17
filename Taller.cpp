#include <iostream>
#include <cstring> // Librería obligatoria para poder usar strcmp y strcpy con char[]

using namespace std;

// ====================================================================
// 1. ESTRUCTURAS DE DATOS (EL MODELO)
// ====================================================================

struct Taller {
    char nombre[100];
    char rif[20];
    char direccion[200];
    char telefono[20];
};

struct Vehiculo {
    int id;
    char placa[10];
    char marca[50];
    char modelo[50];
    int anio;
    int kilometraje;
    char nombrePropietario[100];
    char cedulaPropietario[20];
    char telefonoPropietario[20];
    char fechaRegistro[11];
};

struct Mecanico {
    int id;
    char nombre[100];
    char cedula[20];
    char especialidad[20];
    char fechaIngreso[11];
};

struct Repuesto {
    int id;
    char codigo[20];
    char nombre[100];
    char marca[50];
    float precio;
    int stock;
    int stockMinimo;
    char fechaRegistro[11];
};

struct DetalleOrden {
    int idRepuesto;
    int cantidad;
    float precioUnitario;
};

struct OrdenServicio {
    int id;
    int idVehiculo;
    int idMecanico;
    char fechaApertura[11];
    char fechaCierre[11];
    char estado[12];
    char descripcionFalla[200];
    int kilometrajeIngreso;
    float manoObra;
    
    // ATENCIÓN: Memoria dinámica anidada. Cada orden tiene su propio arreglo de detalles.
    DetalleOrden* detalles; 
    int numDetalles;
    int capacidadDetalles;
};

struct SistemaTaller {
    Taller taller; // Variable normal, usamos punto (.)
    
    // Arreglos dinámicos del sistema
    Vehiculo* vehiculos; 
    int numVehiculos;
    int capacidadVehiculos;
    
    Mecanico* mecanicos; 
    int numMecanicos;
    int capacidadMecanicos;
    
    Repuesto* repuestos; 
    int numRepuestos;
    int capacidadRepuestos;
    
    OrdenServicio* ordenes; 
    int numOrdenes;
    int capacidadOrdenes;
    
    // Contadores para los IDs autoincrementales
    int siguienteIdVehiculo;
    int siguienteIdMecanico;
    int siguienteIdRepuesto;
    int siguienteIdOrden;
};

// ====================================================================
// 2. FUNCIONES DE LA CAPA DE LÓGICA
// (Regla estricta: ¡Cero cin y cero cout en esta sección!)
// ====================================================================

// --- INICIALIZAR EL SISTEMA ---
void inicializarSistema(SistemaTaller* s, Taller datosTaller) {
    // Usamos la flecha (->) porque 's' es un puntero
    s->taller = datosTaller;
    
    // 1. Empezamos con el taller vacío (contadores en 0)
    s->numVehiculos = 0;
    s->numMecanicos = 0;
    s->numRepuestos = 0;
    s->numOrdenes = 0;
    
    // 2. Definimos la capacidad inicial exigida de 4
    s->capacidadVehiculos = 4;
    s->capacidadMecanicos = 4;
    s->capacidadRepuestos = 4;
    s->capacidadOrdenes = 4;
    
    // 3. ALQUILAMOS la memoria RAM con 'new'
    s->vehiculos = new Vehiculo[s->capacidadVehiculos];
    s->mecanicos = new Mecanico[s->capacidadMecanicos];
    s->repuestos = new Repuesto[s->capacidadRepuestos];
    s->ordenes = new OrdenServicio[s->capacidadOrdenes];
    
    // 4. Los IDs comienzan en 1
    s->siguienteIdVehiculo = 1;
    s->siguienteIdMecanico = 1;
    s->siguienteIdRepuesto = 1;
    s->siguienteIdOrden = 1;
}


// --- AGREGAR UN VEHÍCULO ---
Vehiculo* agregarVehiculo(SistemaTaller* s, Vehiculo datos) {
    
    // 1. Validaciones de entrada (usamos punto '.' porque 'datos' NO es puntero)
    if (datos.anio < 1950 || datos.anio > 2027) {
        return nullptr; // Falla por año inválido
    }
    
    if (datos.kilometraje <= 0) {
        return nullptr; // Falla por kilometraje inválido
    }
    
    // 2. Validar placa duplicada (usando strcmp)
    for (int i = 0; i < s->numVehiculos; i++) {
        // strcmp devuelve 0 si las cadenas de caracteres son exactamente iguales
        if (strcmp(datos.placa, s->vehiculos[i].placa) == 0) {
            return nullptr; // Falla porque la placa ya existe
        }
    }
    
    // 3. REDIMENSIONAMIENTO AUTOMÁTICO (Si el estacionamiento se llenó)
    if (s->numVehiculos == s->capacidadVehiculos) {
        // a. Calculamos el doble de capacidad
        int nuevaCapacidad = s->capacidadVehiculos * 2;
        
        // b. Alquilamos un arreglo nuevo más grande
        Vehiculo* nuevoArreglo = new Vehiculo[nuevaCapacidad];
        
        // c. Mudamos los carros viejos al arreglo nuevo
        for (int i = 0; i < s->numVehiculos; i++) {
            nuevoArreglo[i] = s->vehiculos[i];
        }
        
        // d. ¡IMPORTANTE! Demolemos el arreglo viejo para evitar fugas de memoria
        delete[] s->vehiculos;
        
        // e. Actualizamos el sistema para que use el arreglo nuevo
        s->vehiculos = nuevoArreglo;
        s->capacidadVehiculos = nuevaCapacidad;
    }
    
    // 4. Guardamos el carro en la posición vacía
    int posicion = s->numVehiculos;
    s->vehiculos[posicion] = datos;
    
    // 5. Asignamos los datos generados por el sistema (ID y Fecha)
    s->vehiculos[posicion].id = s->siguienteIdVehiculo;
    strcpy(s->vehiculos[posicion].fechaRegistro, "2026-10-16"); // strcpy copia el texto
    
    // 6. Actualizamos los contadores para el próximo carro
    s->numVehiculos++;
    s->siguienteIdVehiculo++;
    
    // 7. Retornamos la dirección de memoria del carro creado usando '&'
    return &(s->vehiculos[posicion]);
}


// --- LIBERAR LA MEMORIA (CERRAR EL SISTEMA) ---
void liberarSistema(SistemaTaller* s) {
    // 1. Devolvemos los arreglos básicos que alquilamos con 'new'
    delete[] s->vehiculos;
    delete[] s->mecanicos;
    delete[] s->repuestos;
    
    // 2. ATENCIÓN: Memoria anidada. Borramos los detalles de CADA orden primero.
    for (int i = 0; i < s->numOrdenes; i++) {
        delete[] s->ordenes[i].detalles;
    }
    
    // 3. Finalmente, borramos el arreglo principal de órdenes
    delete[] s->ordenes; 
}

// Para probar tu código, luego crearemos el "int main()" que pertenece a la Capa de Presentación

