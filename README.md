

```markdown
# Sistema de Gestión para Taller Mecánico

Sistema de consola en C++ diseñado para administrar el inventario, el personal y las operaciones diarias de un taller automotriz. 

Integrantes:
* [Ali Barrientos / 32666898]
* [Gabriel Zambrano / 32477775]

## Instrucciones de Compilación y Ejecución

**Compilar el código:**
Ejecuta el compilador MinGW desde la terminal en el directorio raíz del proyecto:
> `g++ Taller.cpp -o Taller.exe`

**Ejecutar el sistema (Modo interactivo):**
> `./Taller.exe`

**Ejecutar el sistema (Modo automatizado con archivos):**
Redirige los archivos de entrada y salida estándar:
> `cmd /c "Taller.exe < pruebas\entrada.txt > pruebas\salida.txt"`

## Diagrama de Estructuras y Relaciones

El sistema centraliza los datos en `SistemaTaller`, el cual administra cuatro arreglos dinámicos principales. Las órdenes de servicio implementan memoria dinámica anidada.

```text
SistemaTaller
 │
 ├──> vehiculos (Vehiculo[])
 ├──> mecanicos (Mecanico[])
 ├──> repuestos (Repuesto[])
 │
 └──> ordenes (OrdenServicio[])
       │
       └──> detalles (DetalleOrden[])  <-- Relación Orden -> Detalles

```

## Separación de Capas

El proyecto divide el código en dos bloques funcionales estrictos:

* **Capa de Lógica:** Contiene las reglas de negocio, procesa las validaciones (como las búsqueda de cédulas duplicadas) y gestiona la memoria dinámica (`new`/`delete`). No utiliza comandos de entrada/salida (`cin`/`cout`).
* **Capa de Presentación:** Muestra los menús, captura las entradas del usuario por teclado y llama a las funciones lógicas correspondientes. Muestra mensajes de éxito o error basándose en los valores retornados por la lógica.

## Firmas Diseñadas y Tipos de Retorno

**Módulo de Mecánicos:**

* `Mecanico* agregarMecanico(SistemaTaller* s, Mecanico datos)`
*Justificación:* Retorna un puntero al mecánico recién creado para confirmar el éxito de la operación. Retorna `nullptr` si la validación falla (cédula duplicada).
* `Mecanico** listarMecanicos(SistemaTaller s, int* cantidad)`
*Justificación:* Retorna un arreglo de punteros (`**`). Evita duplicar estructuras pesadas en memoria al enviar la lista a la capa de presentación.

**Módulo de Repuestos:**

* `Repuesto* agregarRepuesto(SistemaTaller* s, Repuesto datos)`
*Justificación:* Retorna un puntero al repuesto creado. Permite a la presentación acceder al ID generado automáticamente para mostrarlo en pantalla.
* `bool registrarEntradaRepuesto(SistemaTaller* s, int idRepuesto, int cantidadAgregar)`
*Justificación:* Retorna un valor booleano. La presentación solo necesita confirmar si la suma al stock se ejecutó (`true`) o falló por ID inexistente (`false`).

**Módulo de Órdenes:**

* `OrdenServicio* abrirOrdenServicio(...)`
*Justificación:* Retorna un puntero para confirmar la creación y exponer el ID generado. Retorna `nullptr` si el mecánico supera el límite de 3 órdenes abiertas.
* `bool agregarRepuestoAOrden(SistemaTaller* s, int idOrden, int idRepuesto, int cantidad)`
*Justificación:* Retorna un booleano para validar el éxito. Falla (`false`) si el stock es insuficiente o la orden ya está cerrada.

## Preguntas de Análisis

**1. `agregarVehiculo` retorna un puntero a un elemento de `s->vehiculos`. Si guardan ese puntero en una variable, registran varios vehículos más y luego lo usan, ¿qué puede ocurrir y por qué?**

Ocurre una lectura de memoria inválida (dangling pointer). Al registrar más vehículos, el arreglo dinámico alcanza su capacidad máxima y activa la función de redimensionamiento. El sistema solicita un nuevo bloque de memoria en la RAM, copia los carros y ejecuta `delete[]` sobre el arreglo original. El puntero guardado en la variable seguirá apuntando a la dirección de memoria vieja, la cual ya fue destruida y devuelta al sistema operativo.

**2. ¿Por qué `DetalleOrden` guarda `precioUnitario` en lugar de consultar el precio del repuesto cada vez que se calcula el total?**

Conserva la integridad histórica de la facturación. Si una orden de servicio consulta el precio dinámicamente desde el inventario maestro, cualquier actualización futura en el precio de la pieza alteraría retroactivamente el total de las facturas cerradas meses atrás. Guardar el `precioUnitario` dentro de `DetalleOrden` congela el valor exacto del momento de la compra.

**3. Cuando se redimensiona el arreglo de órdenes, ¿qué ocurre con el arreglo de detalles de cada orden? ¿Se copia, se comparte o se pierde? Justifiquen con su implementación.**

Se comparte temporalmente y se conserva intacto. En nuestra implementación, el redimensionamiento ejecuta una copia superficial de las estructuras (`nuevoArreglo[i] = s->ordenes[i]`). Esto significa que el puntero interno `detalles` de la nueva orden apunta a la misma dirección de RAM que el puntero de la orden vieja. Al ejecutar `delete[] s->ordenes` para demoler el arreglo principal viejo, no destruimos los arreglos internos (no hay un bucle `delete[] detalles` en ese bloque específico). Por lo tanto, el nuevo arreglo principal hereda el acceso directo a los arreglos de detalles originales sin perder información ni duplicar la memoria.

```

```
