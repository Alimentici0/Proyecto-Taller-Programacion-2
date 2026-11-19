# Sistema de Gestión de Taller — Proyecto 2 (persistencia en archivos binarios)

Programación 2 · 2026-C

El sistema del Proyecto 1 guardaba todo en arreglos dinámicos en RAM. En esta versión los datos viven en
`datos/*.bin`, sobreviven entre ejecuciones y cada operación lee o escribe **solo los registros que necesita**,
de uno en uno. No existe ningún arreglo del sistema en memoria.

## Índice

1. [Arquitectura y archivos](#1-arquitectura-y-archivos)
2. [Compilación y ejecución](#2-compilación-y-ejecución)
3. [Estructuras: `sizeof` y fórmula de posición](#3-estructuras-sizeof-y-fórmula-de-posición)
4. [Relación entre ID e índice físico](#4-relación-entre-id-e-índice-físico)
5. [Diseño del detalle de la orden](#5-diseño-del-detalle-de-la-orden)
6. [Firmas de la capa de persistencia](#6-firmas-de-la-capa-de-persistencia)
7. [Firmas de la capa de lógica](#7-firmas-de-la-capa-de-lógica)
8. [Decisiones de diseño y reglas de negocio](#8-decisiones-de-diseño-y-reglas-de-negocio)
9. [Datos pre-poblados](#9-datos-pre-poblados)
10. [Casos de uso y pruebas](#10-casos-de-uso-y-pruebas)
11. [Preguntas de análisis](#11-preguntas-de-análisis)
12. [Limitaciones conocidas](#12-limitaciones-conocidas)

---

## 1. Arquitectura y archivos

Tres capas. Cada una solo llama a la inmediatamente inferior:

```
 PRESENTACION   (cin / cout)        presentacion_*.cpp
      │  llama a
      ▼
 LOGICA         (reglas de negocio) logica_*.cpp, mantenimiento.cpp, reportes.cpp
      │  llama a
      ▼
 PERSISTENCIA   (bytes en archivos) persistencia*.cpp   ── <fstream> ──►  datos/*.bin
```

* La **presentación** no abre archivos ni valida reglas: llama a la lógica y traduce cada código de resultado a un mensaje.
* La **lógica** no usa `cin`/`cout`: valida, decide y combina operaciones de persistencia.
* La **persistencia** no usa `cin`/`cout` ni aplica reglas de negocio: calcula posiciones y lee/escribe bytes.

| Carpeta / archivo | Contenido |
|---|---|
| `src/estructuras.h` | Estructuras de tamaño fijo para disco y constantes del sistema |
| `src/resultado.h` | `enum Resultado`: éxito o causa del fallo |
| `src/persistencia.h/.cpp` | Header, rutas, posiciones, utilidades de archivo, herramientas de mantenimiento |
| `src/persistencia_{vehiculos,mecanicos,repuestos,ordenes,taller}.*` | Las operaciones de cada archivo |
| `src/logica_{vehiculos,mecanicos,repuestos,ordenes,taller}.*` | Reglas de negocio de cada módulo |
| `src/mantenimiento.*` | `verificarIntegridad()` y `crearBackup()` |
| `src/reportes.*` | Los tres reportes |
| `src/presentacion*.cpp/.h` | Menús y entrada/salida |
| `src/utilidades.*` | Texto (`char[]`) y fechas |
| `datos/` | Archivos `.bin` pre-poblados |
| `pruebas/` | Entradas y salidas de los casos de uso + script que los ejecuta |
| `herramientas/generar_datos.cpp` | Genera los `.bin` de ejemplo usando la capa de lógica |
| `experimentos/` | Experimento de la pregunta 1 |
| `etapa1/` | Código original del Proyecto 1 (solo referencia; no se compila con el proyecto) |

## 2. Compilación y ejecución

Requiere un compilador con **C++17** (se usa `std::filesystem` para crear carpetas).

**Linux / macOS / MSYS2 / MinGW con `make`:**

```bash
make            # compila y genera el ejecutable ./taller
./taller        # ejecuta (usa la carpeta datos/ del directorio actual)
```

**Sin `make`** (por ejemplo, Windows con MinGW):

```bash
g++ -std=c++17 -Wall -Wextra -o taller src/*.cpp                       # bash / MSYS2
g++ -std=c++17 -Wall -Wextra -o taller.exe (Get-ChildItem src\*.cpp)   # PowerShell
```

Otros objetivos del `Makefile`:

| Comando | Efecto |
|---|---|
| `make datos` | Regenera `datos/*.bin` con los datos de ejemplo (**borra los actuales**) |
| `make experimento_p1` | Compila el experimento de la pregunta 1 |
| `make clean` | Borra ejecutables y objetos |
| `./pruebas/ejecutar_pruebas.sh` | Ejecuta todos los casos de uso y regenera las salidas |

El programa usa rutas relativas (`datos/...`): **ejecútelo desde la raíz del proyecto**. Si `datos/` no existe, o falta
algún archivo, se crea con su header inicial; si falta `taller.bin`, el programa pide los datos del taller (primer arranque).
La opción `./taller --eco` repite en pantalla lo que se lee de la entrada (se usa en las pruebas con archivos).

> **Portabilidad de los `.bin`:** los archivos se escriben como copia directa de las estructuras en memoria, por lo que
> dependen del compilador y la plataforma (relleno/padding y tamaño de `time_t`). Los `.bin` de `datos/` se generaron con
> g++ 64 bits en Linux. Si en otra máquina los tamaños de la sección 3 no coinciden, ejecute `make datos`.

## 3. Estructuras: `sizeof` y fórmula de posición

Medido con g++ 13 (64 bits), `time_t` = 8 bytes:

| Estructura | `sizeof` | Archivo |
|---|---:|---|
| `ArchivoHeader` | **16** | primeros 16 bytes de cada archivo de entidades |
| `Taller` | **360** | `taller.bin` (un solo registro, sin header) |
| `Vehiculo` | **288** | `vehiculos.bin` |
| `Mecanico` | **176** | `mecanicos.bin` |
| `Repuesto` | **208** | `repuestos.bin` |
| `LineaDetalle` | **12** | (dentro de `OrdenServicio`) |
| `OrdenServicio` | **520** | `ordenes.bin` |

La suma de los campos no coincide con `sizeof` por el **relleno (padding)** del compilador. Ejemplo, `Vehiculo`:
`id`(4) + `placa`(10) + `marca`(50) + `modelo`(50) = 114 → 2 bytes de relleno para alinear `anio` (int) → `anio`(4) + `kilometraje`(4) +
3 cadenas (100+20+20) = 264 → `eliminado`(1) → 7 bytes de relleno para alinear `time_t` → 2 × `time_t`(8) = **288**.
Por eso el código siempre usa `sizeof(Estructura)` y nunca una suma manual. Antes de escribir, los registros se ponen en
ceros (`memset`/`copiarTexto`) para que el relleno y las cadenas no guarden basura en el disco.

**Fórmula de posición** del registro con índice físico `i` (0, 1, 2…):

```
posicion = sizeof(ArchivoHeader) + i * sizeof(Estructura)
         = 16 + i * sizeof(Estructura)
```

Tamaño de un archivo con `n` registros: `16 + n * sizeof(Estructura)`. Comprobación con `datos/`: `vehiculos.bin` tiene
9 registros → 16 + 9·288 = **2608** bytes; `ordenes.bin` tiene 10 → 16 + 10·520 = **5216** bytes (coincide con `ls -l`).

Ejemplo del mensaje de depuración (Caso 2): el vehículo ID 5 se lee en el byte `16 + (5-1)·288 = 1168`.

## 4. Relación entre ID e índice físico

```
indiceFisico = id - 1          posicion = sizeof(ArchivoHeader) + (id - 1) * sizeof(Estructura)
```

Los IDs se asignan en orden de inserción a partir de 1 (el header guarda `proximoID`) y cada registro se agrega al final,
así que el registro con ID `n` es el `n`-ésimo del archivo. Un ID es válido si `1 <= id <= cantidadRegistros` (que cuenta
también los eliminados lógicamente). Se usa en **todas** las búsquedas por ID, sin recorrer el archivo.

**Qué cambio dejaría de hacerla válida** (cualquiera de estos):

* **Borrado físico** de un registro (desplaza o deja un hueco: el ID `n` ya no estaría en la posición `n-1`). Por eso el borrado es lógico.
* **Compactar o reordenar** el archivo (por ejemplo, ordenarlo por nombre o "limpiar" los eliminados).
* **Insertar en el medio** o **reutilizar IDs** de registros eliminados.
* Asignar IDs **no consecutivos** (aleatorios, saltos, o `proximoID` que no avance de 1 en 1).
* Registros de **tamaño variable**: el cálculo `i * sizeof(...)` supone que todos miden lo mismo.

`verificarIntegridad()` comprueba justamente esto: que el registro físico `i` tenga `id == i + 1`.

## 5. Diseño del detalle de la orden

### Estructura elegida

Cada orden lleva sus líneas **dentro de su propio registro**, como un arreglo de tamaño fijo más un contador:

```cpp
struct LineaDetalle {            // sizeof = 12 bytes
    int   idRepuesto;
    int   cantidad;
    float precioUnitario;        // precio histórico (el vigente al agregar la línea)
};

struct OrdenServicio {           // sizeof = 520 bytes
    ...campos de la orden...
    int          numLineas;                       // líneas en uso (0..20), offset 256
    LineaDetalle lineas[MAX_LINEAS_ORDEN];        // MAX_LINEAS_ORDEN = 20, offset 260
    bool eliminado; time_t fechaCreacion; time_t fechaUltimaModificacion;
};
```

Las primeras `numLineas` posiciones del arreglo están en uso; el resto es espacio reservado.

### Cuántos bytes ocupa una orden en disco

Todos los registros de `ordenes.bin` miden `sizeof(OrdenServicio)` = **520 bytes**, tenga las líneas que tenga:

| Orden con | Bytes del registro en disco | Bytes de líneas en uso | Bytes de líneas desperdiciados |
|---|---:|---:|---:|
| 1 línea | **520** | 12 | 228 (de los 240 reservados) |
| 15 líneas | **520** | 180 | 60 |
| 20 líneas (máximo) | **520** | 240 | 0 |

En la práctica una orden con 1 línea ocupa lo mismo que una con 15: el costo es desperdiciar espacio en las órdenes
cortas; la ventaja es que no hace falta nada más para guardarlas. `ordenes.bin` con `n` órdenes pesa `16 + 520·n`.

### Cumplimiento de los 5 requisitos

1. **Conserva las líneas entre ejecuciones** con repuesto, cantidad y precio histórico: las líneas son bytes del mismo registro (Casos 3 y 4).
2. **Al menos 15 líneas**: admite 20 (`MAX_LINEAS_ORDEN`). Al intentar la 21 se rechaza con un mensaje y la línea no se agrega (Caso 10).
3. **Registros del mismo tamaño**: sí, todos miden 520 bytes, así que el acceso por cálculo sigue valiendo.
4. **Consultar las líneas no carga otras órdenes**: leer una orden con sus líneas es **una** lectura de 520 bytes en su posición.
5. **Reglas del Proyecto 1**: acumular, quitar la línea completa devolviendo su stock y conservar el precio histórico (detalles abajo).

### Alternativa considerada y por qué se descartó

**Un archivo aparte `detalles.bin`** con un registro por línea (`idOrden`, `idRepuesto`, `cantidad`, `precio`, `eliminado`).
Es más flexible (sin tope de líneas, sin espacio desperdiciado), pero se descartó porque:

* Para listar las líneas de una orden hay que **recorrer** `detalles.bin` filtrando por `idOrden`, o mantener una lista enlazada
  (`primeraLinea` en la orden y `siguiente` en cada línea). Con ambas se pierde la ventaja del acceso directo por cálculo.
* Agregar o quitar una línea pasa a tocar **tres** archivos (`detalles.bin`, `repuestos.bin` y a veces `ordenes.bin`),
  con más posibilidades de quedar inconsistentes si el programa se interrumpe.
* Habría que añadir un sexto archivo con su header, su inicialización, su backup y sus propias reglas de integridad.

Con un máximo de 20 líneas y una orden de 520 bytes, el desperdicio de espacio es pequeño comparado con esa complejidad.

Otra alternativa descartada fue guardar las líneas **a continuación** de la orden con tamaño variable: rompe el requisito 3 (registros del mismo tamaño).

### Cómo se resuelve "quitar una línea"

`quitarRepuestoDeOrden` busca la línea por `idRepuesto`; si no está, devuelve `ERR_LINEA_NO_EXISTE`. Si está:

1. Guarda sus unidades para devolverlas.
2. **Corre una posición hacia arriba** todas las líneas siguientes (`lineas[i] = lineas[i+1]`), de modo que las líneas en uso
   quedan siempre contiguas al principio.
3. Pone en ceros la última posición que quedó libre y hace `numLineas--`.
4. Escribe la orden y **después** devuelve las unidades al stock del repuesto (ver sección 11, pregunta 2).

No hace falta una marca de "línea eliminada": el arreglo es pequeño y está en memoria mientras se edita la orden, así que correr las líneas es barato
y evita que queden huecos que el resto del código tendría que saltar.

**Acumular:** si el repuesto ya está en la orden, `agregarRepuestoAOrden` suma las unidades a **su** línea, que conserva el precio histórico
original (aunque el precio del repuesto haya cambiado después).

## 6. Firmas de la capa de persistencia

### Cómo informa cada función éxito, fallo y causa

Todas las operaciones que pueden fallar devuelven un **`Resultado`** (`resultado.h`): `EXITO` o el código de la causa.
Los datos de salida (registro leído, ID asignado, posición) viajan en **parámetros por referencia**.

* **Por qué no `bool`:** `false` no distingue "el ID no existe" de "no se pudo abrir el archivo" de "el registro estaba eliminado",
  y el enunciado exige que la presentación indique cuál fue el caso.
* **Por qué no punteros:** están prohibidos como retorno; además un puntero a un registro leído del disco sería una dirección
  de una variable local o dinámica que alguien tendría que liberar. Aquí cada lectura produce una **copia** en la variable del llamador.
* **Por qué un solo `enum`:** los tres capas usan los mismos códigos (`ERR_NO_EXISTE`, `ERR_ARCHIVO`…), la lógica añade los suyos
  (`ERR_DUPLICADO`, `ERR_STOCK_INSUFICIENTE`…) y la presentación los traduce en `mensajeResultado()`.

En persistencia solo aparecen tres valores: `EXITO`, `ERR_NO_EXISTE` (ID fuera de `1..cantidadRegistros`) y `ERR_ARCHIVO`
(no se pudo abrir, leer o escribir). `ERR_ELIMINADO` aparece solo en `marcar...Eliminado` cuando el registro ya lo estaba.

### Operaciones de header y utilidades (`persistencia.h`)

| Función | Retorno | Informa |
|---|---|---|
| `Resultado inicializarArchivo(const char* ruta)` | `EXITO` si el archivo quedó listo (existía o se creó, y su header se puede leer) | `ERR_ARCHIVO` si no se pudo crear o leer. No modifica un archivo existente |
| `Resultado leerHeader(const char* ruta, ArchivoHeader& header)` | `Resultado` | Lee solo los primeros 16 bytes |
| `Resultado actualizarHeader(const char* ruta, const ArchivoHeader& header)` | `Resultado` | Abre en `in\|out` (sin truncar) y pisa solo la posición 0 |
| `Resultado inicializarTodosLosArchivos(const char*& rutaConProblema)` | `Resultado` | Si falla, `rutaConProblema` dice cuál archivo causó el error |
| `long posicionRegistro(int indiceFisico, size_t tam)`, `long posicionPorID(int id, size_t tam)` | posición en bytes | Cálculo puro, sin archivos |
| `Resultado contarRegistrosEnDisco(...)`, `Resultado leerRegistroFisico(...)` | `Resultado` | Para la verificación de integridad: miran el archivo sin fiarse del header |
| `Resultado crearDirectorio(...)`, `Resultado copiarArchivoBinario(...)`, `bool existeArchivo(...)` | | `existeArchivo` es `bool` porque no necesita informar una causa |

### Cinco operaciones por entidad (vehículos, mecánicos, repuestos y órdenes)

Se repiten por entidad (en el Proyecto 3 se unificarán). Ejemplo con vehículos; el patrón es idéntico en
`Mecanico`, `Repuesto` y `OrdenServicio` (`guardarOrden`, `leerOrdenPorID`, `sobrescribirOrden`, `marcarOrdenEliminada`, `contarOrdenesActivas`):

| Función | Retorno y parámetros | Cómo informa |
|---|---|---|
| Crear | `Resultado guardarVehiculo(const Vehiculo& nuevo, int& idAsignado)` | `idAsignado` es el ID que asignó. Escribe el registro y **después** el header |
| Leer por ID | `Resultado leerVehiculoPorID(int id, Vehiculo& destino, long* posicionLeida = nullptr)` | `ERR_NO_EXISTE` si `id` no está en `1..cantidadRegistros`. Entrega el registro **esté activo o eliminado**; `posicionLeida` sirve para el mensaje de depuración |
| Sobrescribir | `Resultado sobrescribirVehiculo(Vehiculo& v)` | Escribe en la posición de `v.id` y actualiza `v.fechaUltimaModificacion` (por eso recibe referencia no constante) |
| Borrado lógico | `Resultado marcarVehiculoEliminado(int id)` | `ERR_ELIMINADO` si ya estaba eliminado (evita restar dos veces en el header) |
| Contar activos | `Resultado contarVehiculosActivos(int& cantidad)` | Lee `registrosActivos` del header |

**Por qué la lectura por ID entrega también registros eliminados:** decidir si un registro eliminado "sirve" es una regla de negocio
(un mecánico dado de baja debe seguir mostrándose en órdenes cerradas, pero no recibir nuevas) y le corresponde a la lógica.
La persistencia solo informa qué hay en esa posición.

`taller.bin` (`persistencia_taller.h`): `bool existeTallerGuardado()`, `Resultado leerTaller(Taller&)`, `Resultado guardarTaller(const Taller&)`.

## 7. Firmas de la capa de lógica

Convenciones: ningún retorno es un puntero; el registro sale como **copia** por referencia. `obtener...PorID` devuelve
`EXITO` (activo), `ERR_ELIMINADO` (existe pero eliminado; el registro **sí** se entrega, para poder decir "fue eliminado: placa X") o
`ERR_NO_EXISTE`. Las búsquedas con varios resultados reciben un arreglo de tamaño `MAX_RESULTADOS_BUSQUEDA` (20) y devuelven
`cantidad` y `hayMas` (si había más coincidencias, la presentación lo avisa). Las operaciones que modifican varios archivos
**validan todo antes de escribir**.

### Vehículos (`logica_vehiculos.h`)

| Función | Qué informa |
|---|---|
| `Resultado registrarVehiculo(const Vehiculo& datos, int& idAsignado)` | `ERR_DATOS_INVALIDOS`, `ERR_ANIO_INVALIDO`, `ERR_KILOMETRAJE_INVALIDO`, `ERR_DUPLICADO` (placa), `ERR_ARCHIVO` |
| `Resultado obtenerVehiculoPorID(int id, Vehiculo& resultado, long* posicion = nullptr)` | `EXITO` / `ERR_ELIMINADO` / `ERR_NO_EXISTE` |
| `Resultado obtenerVehiculoPorPlaca(const char* placa, Vehiculo& resultado)` | `EXITO` / `ERR_NO_EXISTE` (solo activos) |
| `Resultado buscarVehiculosPorPropietario(const char* texto, Vehiculo resultados[], int& cantidad, bool& hayMas)` | máx. 20 resultados |
| `Resultado actualizarVehiculo(int id, const Vehiculo& nuevosDatos)` | no cambia placa, kilometraje ni ID |
| `Resultado eliminarVehiculo(int id)` | `ERR_TIENE_ORDENES` si tiene órdenes en cualquier estado |
| `cantidadVehiculosRegistrados(int&)`, `cantidadVehiculosActivos(int&)` | para que la presentación recorra por ID `1..total` |

### Mecánicos (`logica_mecanicos.h`)

| Función | Qué informa |
|---|---|
| `Resultado registrarMecanico(const Mecanico& datos, int& idAsignado)` | `ERR_DUPLICADO` (cédula), `ERR_ESPECIALIDAD_INVALIDA`, `ERR_FECHA_INVALIDA`, `ERR_DATOS_INVALIDOS` |
| `Resultado obtenerMecanicoPorID(int id, Mecanico& resultado)` | `EXITO` (activo) / `ERR_ELIMINADO` (**de baja**, con el registro cargado) / `ERR_NO_EXISTE` |
| `Resultado contarOrdenesAbiertasDeMecanico(int idMecanico, int& cantidad)` | recorre `ordenes.bin` con una orden en memoria |
| `Resultado darDeBajaMecanico(int id)` | `ERR_TIENE_ORDENES_ABIERTAS`, `ERR_ELIMINADO` (ya estaba de baja), `ERR_NO_EXISTE` |

### Repuestos (`logica_repuestos.h`)

| Función | Qué informa |
|---|---|
| `Resultado registrarRepuesto(const Repuesto& datos, int& idAsignado)` | `ERR_DUPLICADO` (código), `ERR_PRECIO_INVALIDO`, `ERR_STOCK_INVALIDO` |
| `obtenerRepuestoPorID`, `obtenerRepuestoPorCodigo(const char*, Repuesto&)` | igual convención que vehículos |
| `Resultado buscarRepuestosPorNombre(const char* texto, Repuesto resultados[], int& cantidad, bool& hayMas)` | máx. 20 |
| `Resultado registrarEntradaRepuesto(int id, int cantidad)` | `ERR_CANTIDAD_INVALIDA` |
| `Resultado cambiarPrecioRepuesto(int id, float nuevoPrecio)` | `ERR_PRECIO_INVALIDO` |
| `Resultado eliminarRepuesto(int id)` | `ERR_REPUESTO_EN_USO` si aparece en el detalle de **cualquier** orden |
| `bool repuestoBajoStock(const Repuesto&)` | `stock <= stockMinimo` (función pura, no puede fallar → `bool`) |

### Órdenes (`logica_ordenes.h`)

| Función | Qué informa |
|---|---|
| `Resultado abrirOrden(int idVehiculo, int idMecanico, int kilometrajeIngreso, const char* descripcion, int& idOrden)` | `ERR_VEHICULO_NO_EXISTE/ELIMINADO`, `ERR_MECANICO_NO_EXISTE/ELIMINADO`, `ERR_VEHICULO_CON_ORDEN_ABIERTA`, `ERR_MECANICO_LIMITE_ORDENES`, `ERR_KILOMETRAJE_INVALIDO` |
| `Resultado obtenerOrdenPorID(int id, OrdenServicio& resultado)` | `ERR_ORDEN_NO_EXISTE` (las órdenes no se eliminan) |
| `Resultado obtenerVistaOrden(int id, VistaOrden& vista)` | la orden **más** placa, marca/modelo, nombre del mecánico (y si está de baja) y totales: así la presentación nunca muestra IDs sueltos |
| `Resultado agregarRepuestoAOrden(int idOrden, int idRepuesto, int cantidad)` | `ERR_ESTADO_INCORRECTO`, `ERR_STOCK_INSUFICIENTE`, `ERR_LIMITE_LINEAS`, `ERR_REPUESTO_NO_EXISTE/ELIMINADO`, `ERR_CANTIDAD_INVALIDA` |
| `Resultado quitarRepuestoDeOrden(int idOrden, int idRepuesto)` | `ERR_LINEA_NO_EXISTE`, `ERR_ESTADO_INCORRECTO` |
| `Resultado cerrarOrden(int idOrden, float manoObra)` | `ERR_MANO_OBRA_INVALIDA`, `ERR_ORDEN_VACIA`, `ERR_ESTADO_INCORRECTO` |
| `Resultado anularOrden(int idOrden)` | `ERR_ESTADO_INCORRECTO` (solo ABIERTA o CERRADA) |
| `float calcularTotalOrden(const OrdenServicio&)`, `calcularTotalRepuestos(...)` | funciones puras |
| `Resultado buscarOrdenesDeVehiculo(int idVehiculo, int ids[], int maximo, int& cantidad, bool& hayMas)` | para el historial |

Cuando falla por el estado (`ERR_ESTADO_INCORRECTO`), la presentación vuelve a leer la orden y muestra **el estado actual** en el mensaje.

### Mantenimiento y reportes

| Función | Qué informa |
|---|---|
| `Resultado verificarIntegridad(ReporteIntegridad& reporte)` | `EXITO` si pudo verificar (aunque encuentre problemas, que quedan en `reporte`: contadores y mensajes). La lógica no imprime |
| `Resultado crearBackup(char* rutaBackup, size_t capacidad)` | entrega el nombre de la carpeta creada; `ERR_ARCHIVO` si alguna copia falla |
| `Resultado reporteRepuestosMasUsados(FilaRepuestoMasUsado top[], int& cantidad)` | hasta 10 filas |
| `Resultado reporteFacturacionMecanico(int idMecanico, FilaFacturacionMecanico& fila)` | una fila por mecánico (incluye los de baja) |
| `Resultado totalFacturadoVehiculo(int idVehiculo, float& total)` | total de órdenes CERRADAS del vehículo |

## 8. Decisiones de diseño y reglas de negocio

* **Placa y código** se guardan en mayúsculas y se comparan sin distinguir mayúsculas. Son únicos **entre activos**: la placa de un vehículo eliminado puede reutilizarse.
* **Cédula del mecánico** es única entre **todos** los mecánicos, incluidos los de baja: es la misma persona y su historial queda asociado a ella.
* **Fechas de apertura y cierre** las pone el sistema (fecha actual); el usuario no las escribe. Las fechas del historial pre-poblado las fija `generar_datos`.
* **Kilometraje:** se acepta `>= 0` al registrar (en el Proyecto 1 se exigía `> 0`) y al abrir una orden no puede ser menor al del vehículo; al abrirla, el vehículo adopta ese kilometraje.
* **Órdenes** siempre se conservan: se **anulan**, nunca se eliminan. Anular devuelve el stock de todas las líneas pero las líneas quedan como historial.
* **Reportes** solo suman órdenes `CERRADA`. Las `ABIERTA` y las `ANULADA` no suman en ningún total.
* **Búsquedas por otros campos** (placa, nombre, código, propietario) recorren el archivo leyendo **un registro a la vez**; la presentación recorre los listados por ID `1..total` con el mismo criterio.
* **Escribir "al final":** un registro nuevo se escribe en el índice `cantidadRegistros` (calculado), que es el final del archivo. Se calcula en lugar de usar `seekp(0, ios::end)` para que, si un cierre abrupto dejó un registro huérfano
  (escrito pero con el header sin actualizar), el siguiente guardado lo pise en vez de desalinear los IDs.
* **Cada operación abre y cierra el archivo.** Es más lento que mantenerlo abierto, pero garantiza que lo escrito está en disco al terminar la operación y mantiene la persistencia sin estado.
* **Entrada de datos** con `getline` + `strtol`/`strtod`: se acabaron los problemas de mezclar `cin >>` con `cin.getline`, y un texto en un campo numérico simplemente se vuelve a pedir.

## 9. Datos pre-poblados

`datos/` contiene un estado de ejemplo generado con `make datos` (`herramientas/generar_datos.cpp`, que usa la capa de lógica para que
stock y precios históricos sean coherentes):

| Archivo | Registros | Detalle |
|---|---:|---|
| `taller.bin` | 1 | Autoservicio El Lago C.A. |
| `vehiculos.bin` | 9 | 8 activos + 1 eliminado (ID 9, sin órdenes) |
| `mecanicos.bin` | 4 | 3 activos + 1 **dado de baja** (ID 3, Luis Fuenmayor, con 2 órdenes cerradas) |
| `repuestos.bin` | 13 | 12 activos + 1 eliminado (ID 13); el ID 10 (bomba de agua) y el ID 11 (alternador) terminan en BAJO STOCK |
| `ordenes.bin` | 10 | **6 CERRADAS**, **2 ABIERTAS** y **2 ANULADAS**; las órdenes 1, 4 y 6 tienen 4, 4 y 5 líneas |

Distribución de órdenes: #1, #2, #3, #4, #6, #7 cerradas · #8, #9 abiertas · #5 (cerrada y luego anulada), #10 (abierta y luego anulada).

## 10. Casos de uso y pruebas

Todo está en `pruebas/`. `./pruebas/ejecutar_pruebas.sh` ejecuta cada caso en `pruebas/trabajo/casoN/` (una copia de `datos/` o una carpeta vacía),
**sin tocar** `datos/`, y escribe las salidas. Los archivos están numerados por ejecución (`entrada_3a.txt`, `entrada_3b.txt`…): las ejecuciones `a`, `b`, `c`
de un caso comparten carpeta, que es lo que demuestra que los datos sobreviven al reiniciar.

| Caso | Archivos | Qué demuestra | Dónde mirar en la salida |
|---|---|---|---|
| 1. Persistencia | `1a`, `1b` | 3 vehículos, 2 mecánicos y 4 repuestos; tras reiniciar, mismos datos y mismos IDs | `salida_1b.txt`: listados idénticos, IDs 1..3 / 1..2 / 1..4 |
| 2. Acceso directo | `2a` | 6 vehículos, eliminar el ID 3, buscar el 5 (mensaje con el **byte 1168**) y buscar el 3 (informa que fue **ELIMINADO**) | `salida_2a.txt`, líneas `[DEPURACION]` |
| 3. La orden sobrevive | `3a`, `3b`, `3c` | Abrir orden + 3 repuestos; reiniciar; 4.º repuesto y +2 unidades de uno existente (acumula 2+2 = 4); quitar uno (su stock vuelve); cerrar con mano de obra; reiniciar y mostrar | `salida_3c.txt`: 3 líneas, precios históricos y total $141.00 = $86.00 + $55.00 |
| 4. 15 líneas | `4a`, `4b` | Orden con 15 repuestos distintos; tras reiniciar siguen las 15 | `salida_4b.txt` |
| 5. Anulación | `5a`, `5b` | Anular la orden #6 (CERRADA, 5 líneas); tras reiniciar, el stock de cada repuesto volvió a su valor previo | comparar el 1.er listado de `5a` con el de `5b`: +1, +1, +4, +4, +1 solo en los 5 repuestos de la orden |
| 6. Baja de mecánico | `6a`, `6b` | Baja rechazada (orden abierta) → cerrar la orden → baja aceptada → sus órdenes cerradas siguen mostrando su nombre → nueva orden rechazada | `salida_6a.txt` / `6b` |
| 7. Integridad | `7a`, `7b` | Backup, nuevo vehículo + orden, reemplazo manual de `vehiculos.bin` por el backup, verificación | `salida_7b.txt`: `ROTO: Orden #11 apunta al vehiculo ID 10 (no existe en vehiculos.bin)` |
| 8. Reportes | `8a` | 10 órdenes entre 4 mecánicos; los 3 reportes; las ABIERTAS y ANULADAS no suman | `salida_8a.txt` |
| 9 (extra) | `9a` | Validaciones y mensajes de error de todas las secciones | `salida_9a.txt` |
| 10 (extra) | `10a`, `10b` | Límite de 20 líneas: la 21 se rechaza y no descuenta stock | `salida_10a.txt` |

El **paso 3 del Caso 7** (reemplazar `datos/vehiculos.bin` por la copia del backup) es manual; el script lo hace entre `entrada_7a` y `entrada_7b`
con `cp backup_*/vehiculos.bin datos/vehiculos.bin`. Si prueba a mano, haga lo mismo con el programa cerrado.

**Verificación independiente de los reportes:** los totales del Reporte 2 y 3 se comprobaron leyendo los `.bin` con un script
aparte en Python (sin usar el código C++) y coinciden: Jose Pérez $335.90, Ana Villalobos $74.00, Luis Fuenmayor $380.00, Marta Soto $128.20; total del taller $918.10.

## 11. Preguntas de análisis

### 1. Escribir una `OrdenServicio` del Proyecto 1 con su `DetalleOrden*`, reiniciar y leerla

El experimento está en `experimentos/experimento_p1.cpp` (`make experimento_p1`; salida guardada en `experimentos/salida_experimento_p1.txt`).
Se escribe la orden con `write` y se lee desde **otra** ejecución.

**Qué se observó:** los campos simples se recuperan bien (`id`, `estado`, `descripcionFalla`, `numDetalles = 2`), y `detalles` muestra la **misma dirección**
que tenía al escribirse (`0x5599ec40c2b0`). Pero al intentar acceder a `detalles[0]` el programa termina con *Segmentation fault* (código de salida 139).
Además, la dirección que se guarda cambia en cada ejecución del escritor (se vieron `0x5606…`, `0x5576…`, `0x5596…`).
(El resultado exacto depende del sistema operativo: en otros puede imprimir basura o cerrarse con otro error, pero siempre es comportamiento indefinido.)

**Por qué ocurre:** `detalles` es un puntero, es decir, un número que es una **dirección de memoria de ese proceso**. `write(&orden, sizeof(OrdenServicio))`
copia los bytes de la estructura (272 en el Proyecto 1) y entre ellos están los 8 bytes de la dirección, **no** las dos líneas, que viven en otra zona del *heap*.
Cuando el programa termina, el sistema operativo libera toda su memoria; en la nueva ejecución esa dirección no apunta a nada del programa (y con la
aleatorización de direcciones ni siquiera sería la misma zona), así que acceder a ella es leer memoria inválida. Los datos simples sobreviven porque están *dentro* de la estructura; las líneas no, porque estaban *fuera*, detrás de un puntero.
Por eso en el Proyecto 2 el detalle tiene que estar **dentro** del registro (un arreglo de tamaño fijo) o en otro archivo, referenciado por IDs y no por direcciones.

### 2. Interrupción entre la escritura del detalle y la del stock

Agregar un repuesto escribe en dos archivos (`ordenes.bin` con la línea y `repuestos.bin` con el stock) y no hay forma de hacerlo de manera atómica.
Según el orden, el daño es distinto:

| Orden de escritura | Si se interrumpe entre las dos | Consecuencia |
|---|---|---|
| Orden y **luego** stock | La línea está en la orden pero el stock **no** se descontó | Stock de **más**: el sistema cree que hay unidades que ya se asignaron (se podría prometer repuesto que no existe). Si el usuario, al no saber si se completó, reintenta, la línea **acumula** y se **cobra doble** al cliente. Para corregirlo haría falta restar stock, y el sistema no tiene una operación de ajuste negativo |
| **Stock y luego orden** *(el que se implementó)* | El stock se descontó pero la línea no llegó a la orden | Stock de **menos**: unidades "perdidas" en el sistema que siguen físicamente en el estante. Si el usuario reintenta, solo baja de nuevo el stock (de menos otra vez); **nunca se cobra de más** al cliente. Se corrige con la operación que ya existe, *Entrada de mercancía* |

**Elegí "stock primero, orden después"** siguiendo el principio de que *el inventario nunca debe sobreestimar lo que hay*: el stock se descuenta **antes**
de que la orden reclame las unidades. Para `quitarRepuestoDeOrden` y `anularOrden` es al revés (la orden libera las unidades primero y el stock las recupera **después**),
por la misma razón: una interrupción siempre deja el stock de menos, nunca de más. En los tres casos el error queda en el lado conservador, no afecta lo que se factura al cliente y se arregla con una entrada.
(Fue una corrección posterior: la primera versión de `agregarRepuestoAOrden` escribía primero la orden, y se cambió al analizar esta pregunta; está en el historial de commits.)

**Límite honesto:** ninguno de los dos órdenes es detectable por `verificarIntegridad()`, porque el sistema no guarda un historial de movimientos de inventario con el que contrastar el stock;
se detectaría en un conteo físico. Resolverlo de verdad requiere una transacción o un registro previo de la operación (*journal*), que va más allá de este proyecto.
Lo que sí se hace es **validar todo antes de la primera escritura**, para que los fallos de reglas de negocio nunca dejen una operación a medias; el único riesgo que queda es la interrupción real del proceso.

### 3. Eliminación física de un mecánico con órdenes

Las órdenes guardan el mecánico como un número (`idMecanico`). Si se borrara físicamente su registro, pasaría una de dos cosas:

* **Se compacta el archivo** (los siguientes mecánicos suben una posición): el ID `n` ya no estaría en la posición `n-1`, rompiendo el acceso directo (sección 4), y peor: las órdenes antiguas con `idMecanico = 3` pasarían a apuntar **a otra persona**. El historial atribuiría trabajos a quien nunca los hizo.
* **Se deja un hueco**: las órdenes quedan apuntando a un mecánico que ya no existe. No se podría mostrar quién hizo cada trabajo, el Reporte 3 (facturación por mecánico) perdería lo facturado por él, y la verificación de integridad marcaría cada una de esas órdenes como rota.

En ambos casos se pierde el historial. La **baja lógica** lo resuelve porque el registro **nunca se mueve ni se borra**: solo se marca `eliminado = true`. El ID sigue en su posición, así que las órdenes
cerradas y anuladas siguen encontrando su nombre (Caso 6) y el reporte lo sigue incluyendo (marcado "baja"); y como `abrirOrden` rechaza a los mecánicos de baja y `darDeBajaMecanico` exige que no tengan
órdenes abiertas, la baja tampoco deja órdenes en curso sin responsable. Es la misma razón por la que vehículos y repuestos no se pueden eliminar si tienen órdenes asociadas.

## 12. Limitaciones conocidas

* **No hay transacciones.** Una interrupción real entre dos escrituras puede dejar el stock de menos (ver pregunta 2). La verificación de integridad no puede detectarlo.
* **Un solo usuario.** No hay bloqueo de archivos: dos instancias del programa escribiendo a la vez podrían pisarse.
* **Los `.bin` no son portables** entre compiladores/plataformas con distinto relleno o `time_t` (sección 2).
* **Rendimiento:** cada lectura reabre el archivo, y las búsquedas por campo y los reportes recorren los archivos completos (un registro a la vez). Es correcto para el tamaño del taller; con
  muchos miles de registros convendría mantener el archivo abierto o añadir índices.
* **Backup:** el nombre de la carpeta usa solo hasta el minuto (`backup_AAAA-MM-DD_HH-MM`); dos backups en el mismo minuto se sobrescriben. Restaurar es manual (copiar los `.bin` con el programa cerrado).
* Los textos del programa están en ASCII (sin tildes) para verse igual en cualquier consola.
