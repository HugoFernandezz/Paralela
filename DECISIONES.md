# Registro de decisiones de diseño

Práctica final de Programación Paralela 2026/27: cálculo del gradiente y del Jacobiano de funciones polinómicas, en versión secuencial, OpenMP y CUDA.

Aquí queda cada decisión de diseño: qué había que decidir, qué opciones había, cuál se recomendó y cuál se eligió. El documento se actualiza al cerrar las decisiones de cada fase.

## Entorno

- Repositorio en luna: `/home/g15/Paralela` (usuario `g15`). Se compila y se ejecuta en `practicasPP-26-27/`.
- Comprobado en luna (fase A0): nvcc 12.6 (V12.6.85) en `/usr/local/cuda/bin/nvcc`, driver 535.288.01 y GPU NVIDIA GeForce RTX 2080 Ti.
- La plantilla compila con `-O2 -Wall`: sale solo con avisos, sin errores, y se ejecuta en los modos 0 y 1.

## Resumen

| # | Fase | Decisión | Elegida |
|---|------|----------|---------|
| 1 | A0 | Nivel de optimización al compilar | B: `-O2` en g++ y nvcc |
| 2 | A0 | Idioma de los identificadores nuevos | A: español |
| 3 | A1 | Formato de los ficheros de entrada | B: numérico estructurado |
| 4 | A1 | Estructura del polinomio en memoria | B: struct de arrays densos |
| 5 | A1 | Tipo numérico (`type_data`) | A: `float` (repetir con `double` al final) |
| 6 | A1 | Tamaño del problema | C: P puntos × N variables × M monomios |
| 7 | A1 | Sistemas de ecuaciones | A: diseño para K funciones desde el principio |
| 8 | A2 | Cómo se guarda el gradiente o Jacobiano (pasos 1 y 2) | A: explícito, en otro `Polinomios` con K·N polinomios |

---

## Decisión 1. Nivel de optimización al compilar (fase A0)

**Contexto.** La plantilla compila con `-O0`. En nvcc, `-O` solo afecta al código de la CPU: los kernels de la GPU se optimizan siempre. Por eso, con `-O0` se compara una CPU sin optimizar contra una GPU optimizada, y el resultado queda sesgado a favor de la GPU.

**Opciones.**
- **A. Mantener `-O0`, como la plantilla.** Es lo que dio el profesor, pero la CPU va artificialmente lenta y las conclusiones son discutibles.
- **B. `-O2` en g++ y nvcc.** Es la optimización estándar: tiempos realistas y comparación justa. En gcc 11, `-O2` no vectoriza automáticamente, así que el código hace lo que se lee.
- **C. `-O3 -march=native`.** Añade la vectorización con AVX2. Da el máximo rendimiento de la CPU, pero introduce un efecto más (SIMD) que hay que saber explicar.

**Recomendada:** B. **Elegida: B.**

**Además:** se sustituye `-w` (oculta todos los avisos) por `-Wall`, para ver los avisos del compilador. nvcc no admite `-Wall` directamente, así que en el `.cu` se pasa al compilador de la CPU con `-Xcompiler -Wall`.

**Aplicado en el Makefile (fase A0).** Ojo: las reglas del Makefile no dependen de los `.h`. Si se cambia una cabecera (por ejemplo `type.h`), hay que hacer `make clean` antes de `make`.

---

## Decisión 2. Idioma de los identificadores nuevos (fase A0)

**Opciones.**
- **A. Español** (`evaluar_gradiente`, `num_monomios`). Encaja con los `ejecucion_CPU/OMP/GPU` de la plantilla.
- **B. Inglés.**

**Recomendada:** A. **Elegida: A.** Los comentarios van en español en cualquier caso.

---

## Decisión 3. Formato de los ficheros de entrada (fase A1)

**Opciones.**
- **A. Texto como el del enunciado** (`3*x*y + 4*y*z + 2*x + 5`).
  - A favor: se lee y se escribe a mano fácilmente.
  - En contra: hace falta un analizador de texto de unas 100 a 150 líneas, ajeno a la asignatura, y un polinomio grande en una sola línea supera el `MAX_LINE` de la plantilla.
- **B. Numérico estructurado.** Una cabecera con los tamaños y una línea por monomio: `coef e1 e2 … eN`.
  - A favor: se lee con `fscanf` en pocas líneas y el generador de casos es trivial.
  - En contra: con N grande no se puede leer a simple vista.

**Recomendada:** B. **Elegida: B.**

**Consecuencias.**
- El programa imprime los pasos 1 y 2 en notación legible (`∂f/∂x = 3·y + 2`). Así se cumple el requisito del enunciado de poder comprobarlos a mano.
- Por la decisión 7, la cabecera incluye el número de funciones K.

**Formato exacto (fijado en A1).** Un único fichero con las funciones y los puntos. Los datos se separan con espacios o saltos de línea, y `#` empieza un comentario hasta el final de la línea.

```
N K                  número de variables y de funciones
M_1 ... M_K          número de monomios de cada función
coef e_1 ... e_N     un monomio por línea: primero los M_1 de f1, después los de f2...
P                    número de puntos
x_1 ... x_N          un punto por línea
```

- Se lee dato a dato con `fscanf`, sin búfer de línea, así que no hay límite de longitud de línea (el `MAX_LINE` de la plantilla no se usa).
- Todos los números de monomios van en la cabecera para poder reservar la memoria de una vez, sin `realloc`.
- Si falta un dato, sobra alguno al final o un recuento es inválido, el programa termina con un mensaje que dice qué dato esperaba.
- Ejemplos: `casos/enunciado.txt` y `casos/prueba_lectura.txt`.

---

## Decisión 4. Estructura del polinomio en memoria (fase A1)

**Opciones.**
- **A. Array de structs con un `malloc` por monomio:** `struct Monomio { type_data coef; int *exp; }`.
  - A favor: es intuitiva.
  - En contra: para llevarla a la GPU hay que copiar puntero a puntero, lo que es lento y propenso a errores.
- **B. Struct de arrays densos:** `coef[M]` y `exp[M*N]`, con una fila de N exponentes por monomio, en bloques contiguos.
  - A favor: pasa a la GPU con pocos `cudaMemcpy` y los accesos son regulares.
  - En contra: también guarda los exponentes que valen 0.
- **C. Disperso, al estilo CSR:** solo los pares (variable, exponente) distintos de 0, con un índice de inicio por monomio.
  - A favor: ocupa lo justo.
  - En contra: los monomios tienen longitudes distintas, el reparto entre hilos queda desequilibrado y el código es más difícil de explicar.

**Recomendada:** B. **Elegida: B.**

**Consecuencia, junto con la decisión 7 (confirmada en A1).** Todas las funciones del sistema se guardan juntas:
- `coef[M_total]`;
- `exp[M_total * N]`;
- `inicio[K + 1]`, que indica dónde empieza cada función.

Así el sistema completo pasa a la GPU con tres copias, sin punteros anidados.

**Implementación (A1).** En `definitions.h`:
- `Polinomios`: N, K, M_total, `inicio`, `coef` y `exp` (exponentes de tipo `int`). Se llama así y no "Sistema" porque la misma estructura servirá en A2 para guardar las derivadas parciales.
- `Puntos`: P, N y `coord[P * N]`.

---

## Decisión 5. Tipo numérico, `type_data` en `type.h` (fase A1)

**Contexto.** La RTX 2080 Ti de luna calcula unas 32 veces más lento en `double` que en `float`.

**Opciones.**
- **A. `float`, como la plantilla.**
  - A favor: la GPU rinde al máximo.
  - En contra: unas 7 cifras significativas, así que con M grande se acumulan errores de redondeo. Hace falta una tolerancia de validación de alrededor de 1e-4 y valores moderados en los puntos para que las potencias no se desborden.
- **B. `double`.**
  - A favor: unas 15 cifras y validación casi exacta.
  - En contra: la GPU queda muy penalizada.

**Recomendada:** A. **Elegida: A.**

**Pendiente:** al final, recompilar con `double` y repetir las mediciones para comparar el efecto en la CPU y en la GPU. Ver la sección de pendientes.

---

## Decisión 6. Qué se hace crecer como "tamaño del problema" (fase A1)

**Contexto.** Un gradiente en un solo punto con pocas variables no da trabajo suficiente para paralelizar.

**Opciones.**
- **A. Un punto, con N y M grandes.**
  - A favor: es fiel al enunciado y obliga a hacer reducciones sobre los monomios.
  - En contra: con un único punto la GPU tiene poco trabajo.
- **B. Muchos puntos P con un polinomio moderado.**
  - A favor: cada punto es independiente, ideal para la GPU.
  - En contra: deja poco que analizar en paralelismo anidado y en reducciones.
- **C. Los tres parámetros: P puntos, N variables y M monomios.**
  - A favor: cuesta solo un bucle más, con P = 1 equivale a A, y permite estudiar el anidamiento variables × puntos que pide el estudio de OpenMP.

**Recomendada:** C. **Elegida: C.**

---

## Decisión 7. Sistemas de ecuaciones (fase A1)

**Opciones.**
- **A. Diseñar todo desde el principio para sistemas de K funciones**, tratando una función suelta como K = 1.
  - A favor: un único camino de código desde el principio y nada que reestructurar en A5.
  - En contra: las estructuras son un poco más generales desde el primer día.
- **B. Empezar con una sola función y ampliar en A5.**
  - A favor: no se generaliza antes de tiempo, y el Jacobiano son K gradientes, que se añaden con un bucle.
  - En contra: hay que adaptar las estructuras en A5.

**Recomendada:** B. **Elegida: A.**

**Consecuencias.**
- El gradiente de una función es el caso K = 1 del Jacobiano.
- Las estructuras y el formato de entrada incluyen K desde la fase A1.

---

## Decisión 8. Cómo se guarda el gradiente o Jacobiano, resultado de los pasos 1 y 2 (fase A2)

**Contexto.** La regla del paso 1 es ∂/∂xⱼ (c·xᵃ) = (c·aⱼ)·x^(a−eⱼ) si aⱼ > 0, y 0 en otro caso. Lo que hay que decidir es si su resultado se guarda o no. De eso dependen tres cosas: cuánta memoria cuesta, qué evalúa el paso 3 y lo fiel que queda la implementación al "seguimiento estricto del método" que exige el enunciado.

Derivar nunca produce términos semejantes: si a ≠ b, entonces a−eⱼ ≠ b−eⱼ. Por eso no hace falta agrupar nada después de derivar.

Para comparar memoria se usa este ejemplo: N = 1000, M = 10⁴ y s = 5 variables por monomio de media. Los exponentes de f ocupan 40 MB.

**Opciones.**
- **A. Explícito.** Otro `Polinomios` con K·N polinomios: ∂fₖ/∂xⱼ va en la posición k·N + j, con sus propios coeficientes y exponentes.
  - A favor: es el método literal, porque el paso 3 evalúa lo que produjo el paso 2. Usa la misma estructura y la misma rutina que f. Es lo más fácil de imprimir y explicar, y pasa a la GPU con tres copias.
  - En contra: ocupa s veces lo que f (200 MB más en el ejemplo). Si cada monomio usara todas las variables (s = N), ocuparía N veces más: 40 GB.
- **B. Implícito.** No se guarda nada: el paso 3 aplica la regla al vuelo sobre los monomios de f.
  - A favor: no ocupa memoria extra y admite un algoritmo más rápido. Con productos parciales por la izquierda y por la derecha cuesta M·N por punto en lugar de s·M·N.
  - En contra: los pasos 1 y 2 solo existirían al imprimir, lo que es un riesgo frente a las instrucciones metodológicas obligatorias. Además, en la versión rápida varios monomios suman en la misma derivada, lo que causa condiciones de carrera.
- **C. Híbrido.** Por cada monomio derivado se guarda el coeficiente c·aⱼ y el índice del monomio de f del que viene.
  - A favor: ocupa poco (unos 0,6 MB en el ejemplo).
  - En contra: el gradiente queda guardado solo a medias, los accesos son indirectos e irregulares y es lo más difícil de explicar.

**Recomendada:** A. **Elegida: A.**

**Consecuencias.**
- La derivación se ejecuta una sola vez, en `derivar()` de `fichero_common.cpp`, y se mide aparte. Es la parte secuencial común a las tres versiones (entra en el análisis de Amdahl).
- La estructura se construye en dos pasadas. Primero se cuentan los monomios de cada derivada y se calculan las sumas acumuladas en `inicio`. Después se rellenan los coeficientes y los exponentes. Los monomios derivados conservan el orden que tenían en f.
- La fila k del Jacobiano son los polinomios k·N … k·N + N−1. Con K = 1 es el gradiente.
- El generador de A4 tendrá que crear monomios con pocas variables cuando N sea grande. El programa imprime cuánto ocupan las derivadas para poder dimensionar los casos.
- La opción B queda como candidata para B2-opt ("replantear el problema a nivel algorítmico"), medida contra A.

---

## Detalles menores de A2

- **Impresión de los pasos 1 y 2 (con `-d`), como en el enunciado:**
  - `∂f/∂x = ∂/∂x(3*x*y + 4*y*z + 2*x + 5) = 3*y + 0 + 2 + 0 = 3*y + 2`
  - `∇f = (∂f/∂x, ∂f/∂y, ∂f/∂z) = (3*y + 2, 3*x + 4*z, 4*y)`
  - Con varias funciones se escribe `∂f1/∂x1` y hay una línea `∇fk` por cada fila del Jacobiano.
- **Términos de la derivada.** La derivada de cada monomio por separado (la parte `3*y + 0 + 2 + 0`) se imprime a partir de la estructura derivada, no se recalcula al imprimir.
- **Derivada nula.** Una derivada sin monomios se imprime como `0`.

---

## Detalles menores de A1 (sin alternativas que cambien el diseño)

- **Opciones nuevas de `main.cpp`.**
  - `-f FICHERO`: fichero de entrada.
  - `-d`: imprime las funciones y los puntos leídos. Usa la variable `data_print` de la plantilla.
- **Valores por defecto** (en `definitions.h`): modo 0 (secuencial), 1 hilo y bloques de 256 hilos.
- **Nombres en la impresión.**
  - Variables: `x`, `y`, `z` con hasta 3 variables, como en el enunciado; `x1 ... xN` con más.
  - Funciones: `f` si hay una sola; `f1 ... fK` si hay varias.
  - Notación: `3*x*y + x^2*z - 7`.
- **Errores.** Un error de lectura o de memoria termina el programa con un mensaje en `stderr`. Las reservas se comprueban todas en `reservar_memoria()`.
- **Fallos de la plantilla corregidos en `main.cpp`.**
  - `mode` empezaba en 1.
  - `-h` no se reconocía.
  - `th` y `blockSize` no estaban inicializados.
  - Los `printf` usaban `%d` con `unsigned long`.
  - El índice sin signo se comparaba con `argc`.
  - `strcpy` a un búfer de 100 caracteres se desbordaba con argumentos largos.
  - Una opción sin valor al final de la línea leía `argv[argc]`, que es NULL.
  - Se quitan `-o`, `-s` y las variables sin uso, que venían de la plantilla de CSV.
- **`fichero_common.h`**: se quitan las declaraciones de funciones que no existían.

---

## Pendientes

- [ ] Recompilar con `type_data` = `double` y repetir las mediciones, para comparar con `float` (decisión 5).
- [x] Aplicar en el Makefile `-O2` (decisión 1) y `-Wall` en lugar de `-w`.
- [x] Validar en luna que la plantilla compila y se ejecuta (fase A0).
- [x] Validar en luna que un kernel mínimo compilado con nvcc 12.6 corre con el driver 535 (fase A0).
- [x] A1: validar en luna la lectura e impresión de `casos/enunciado.txt` y `casos/prueba_lectura.txt`, y los mensajes de error.
- [ ] A1: confirmar que `make` no da avisos en `main.cpp` ni en `fichero_common.cpp` (no se pegó la salida de `make`).
- [ ] A2: validar en luna los pasos 1 y 2 con `casos/enunciado.txt` y `casos/prueba_lectura.txt`.
- [ ] A3: en `main.cpp`, comprobar el modo antes de leer el fichero (ahora `-m 7` lee el fichero antes de dar el error).
- [ ] B2-opt (opcional): probar el gradiente implícito con productos parciales (opción B de la decisión 8) y medirlo contra A.
