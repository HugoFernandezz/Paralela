# Registro de decisiones de diseño

Práctica final de Programación Paralela 2026/27: cálculo del gradiente y del Jacobiano de funciones polinómicas, en versión secuencial, OpenMP y CUDA.

Aquí queda cada decisión de diseño: qué había que decidir, qué opciones había, cuál se recomendó y cuál se eligió. El documento se actualiza al cerrar las decisiones de cada fase.

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

---

## Decisión 1. Nivel de optimización al compilar (fase A0)

**Contexto.** La plantilla compila con `-O0`. En nvcc, `-O` solo afecta al código de la CPU: los kernels de la GPU se optimizan siempre. Por eso, con `-O0` se compara una CPU sin optimizar contra una GPU optimizada, y el resultado queda sesgado a favor de la GPU.

**Opciones.**
- **A. Mantener `-O0`, como la plantilla.** Es lo que dio el profesor, pero la CPU va artificialmente lenta y las conclusiones son discutibles.
- **B. `-O2` en g++ y nvcc.** Es la optimización estándar: tiempos realistas y comparación justa. En gcc 11, `-O2` no vectoriza automáticamente, así que el código hace lo que se lee.
- **C. `-O3 -march=native`.** Añade la vectorización con AVX2. Da el máximo rendimiento de la CPU, pero introduce un efecto más (SIMD) que hay que saber explicar.

**Recomendada:** B. **Elegida: B.**

**Además:** se sustituye `-w` (oculta todos los avisos) por `-Wall`, para ver los avisos del compilador.

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
- El formato exacto (también el del fichero de puntos) se fija al implementar A1 y se añadirá aquí.

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

**Consecuencia, junto con la decisión 7 (se confirma al implementar A1).** Todas las funciones del sistema se guardan juntas:
- `coef[M_total]`;
- `exp[M_total * N]`;
- `inicio[K + 1]`, que indica dónde empieza cada función.

Así el sistema completo pasa a la GPU con tres copias, sin punteros anidados.

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

## Pendientes

- [ ] Recompilar con `type_data` = `double` y repetir las mediciones, para comparar con `float` (decisión 5).
- [ ] Aplicar en el Makefile `-O2` (decisión 1) y `-Wall` en lugar de `-w`.
