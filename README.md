# Proyecto-Taller-Programacion-2
# Sistema de Gestión para Taller Mecánico

Sistema de consola en C++ para administrar inventario, personal y operaciones de un taller automotriz. Creado para la materia Taller de Programación 2 de la Universidad Rafael Urdaneta.

## 🏗️ Arquitectura y Restricciones Técnicas

El diseño aplica las reglas de gestión de memoria de la asignatura:

* **Separación de Capas:** Aísla la Capa de Lógica (datos, punteros) de la Capa de Presentación (menús). La capa lógica omite comandos `cin` y `cout`.
* **Memoria Dinámica:** Usa arreglos dinámicos para Vehículos, Mecánicos, Repuestos y Órdenes. Evita contenedores estándar como `std::vector`. Los arreglos duplican su capacidad al llenarse. Emplea `new` y `delete` para limpiar fugas de memoria.
* **Memoria Dinámica Anidada:** Cada Orden de Servicio crea un sub-arreglo dinámico para registrar repuestos.
* **C-Strings:** Trabaja con arreglos nativos (`char[]`) y rechaza `std::string`. Valida cadenas con la librería `<cstring>` (`strcmp`, `strcpy`).

## ⚙️ Módulos del Sistema

1. **Vehículos:** Registra datos del cliente. Valida placas únicas, año (1950-2027) y kilometraje.
2. **Mecánicos:** Administra técnicos. Bloquea órdenes nuevas para mecánicos con 3 tickets "ABIERTOS".
3. **Repuestos:** Agrega piezas, define el stock mínimo, lanza alertas de escasez y registra mercancía nueva.
4. **Órdenes de Servicio:** Abre tickets, asocia piezas, resta stock del inventario y cierra la orden sumando la mano de obra al precio histórico de cada repuesto.

## 🚀 Compilación y Ejecución

Compila el código con `g++` (MinGW) en Windows.

**1. Compilar:**
Abre la terminal en la carpeta del proyecto y ejecuta:
> `g++ Taller.cpp -o Taller.exe`

**2. Ejecutar:**
Inicia el programa con:
> `./Taller.exe`

## 👥 Autores
* Ali Barrientos, CI 32.666.898
* Gabriel Zambrano, CI. 32477775
