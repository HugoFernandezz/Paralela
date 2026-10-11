#include "definitions.h"
#include "fichero_common.h"
#include "wtime.h"
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>

using namespace std;

// Con hasta 3 variables se llaman x, y, z, como en el enunciado; con más, x1, x2, ...
static const char LETRAS_VARIABLES[] = "xyz";
static const TYPE_DATA MAX_VARIABLES_CON_LETRA = sizeof(LETRAS_VARIABLES) - 1;

/* ------------------------------------------------------------------ */
/* Memoria                                                             */
/* ------------------------------------------------------------------ */

// malloc con comprobación: sin memoria el programa no puede seguir, así que
// dice qué estaba reservando y termina.
void *reservar_memoria(size_t bytes, const char *que)
{
	void *p = malloc(bytes);
	if (p == NULL) {
		fprintf(stderr, "Error: no hay memoria para %s (%zu bytes)\n", que, bytes);
		exit(EXIT_FAILURE);
	}
	return p;
}

void liberar_polinomios(Polinomios *p)
{
	free(p->inicio);
	free(p->coef);
	free(p->exp);
}

void liberar_puntos(Puntos *p)
{
	free(p->coord);
}

/* ------------------------------------------------------------------ */
/* Lectura de la entrada                                               */
/* ------------------------------------------------------------------ */

// Termina el programa diciendo qué dato del fichero falta o es incorrecto.
static void error_lectura(const char *fichero, const char *formato, ...)
{
	va_list args;
	fprintf(stderr, "Error en el fichero %s: ", fichero);
	va_start(args, formato);
	vfprintf(stderr, formato, args);
	va_end(args);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

// Avanza hasta el siguiente dato saltando espacios, saltos de línea y
// comentarios (desde '#' hasta el final de la línea).
static void saltar_comentarios(FILE *fp)
{
	int c;
	while ((c = fgetc(fp)) != EOF) {
		if (c == '#') {
			while ((c = fgetc(fp)) != EOF && c != '\n')
				;
		} else if (!isspace(c)) {
			ungetc(c, fp);
			return;
		}
	}
}

// Leen el siguiente dato del fichero. Devuelven 1 si lo han podido leer.
// Se lee dato a dato con fscanf, sin búfer de línea, así que no hay límite
// de longitud de línea.
static int leer_entero(FILE *fp, long *valor)
{
	saltar_comentarios(fp);
	return fscanf(fp, "%ld", valor) == 1;
}

static int leer_real(FILE *fp, type_data *valor)
{
	double leido;	// se lee como double para que funcione igual con type_data float o double
	saltar_comentarios(fp);
	if (fscanf(fp, "%lf", &leido) != 1)
		return 0;
	*valor = (type_data) leido;
	return 1;
}

// Cabecera: N, K y el número de monomios de cada función.
// inicio[] guarda las sumas acumuladas de esos números: así la función k
// ocupa los monomios inicio[k] .. inicio[k+1]-1 y el total es inicio[K].
static void leer_cabecera(FILE *fp, const char *fichero, Polinomios *f)
{
	long valor;
	TYPE_DATA k;

	if (!leer_entero(fp, &valor) || valor < 1)
		error_lectura(fichero, "se esperaba el número de variables N (entero >= 1)");
	f->num_variables = (TYPE_DATA) valor;
	if (!leer_entero(fp, &valor) || valor < 1)
		error_lectura(fichero, "se esperaba el número de funciones K (entero >= 1)");
	f->num_polinomios = (TYPE_DATA) valor;

	f->inicio = (TYPE_DATA *) reservar_memoria((f->num_polinomios + 1) * sizeof(TYPE_DATA), "el inicio de cada función");
	f->inicio[0] = 0;
	for (k = 0; k < f->num_polinomios; k++) {
		if (!leer_entero(fp, &valor) || valor < 1)
			error_lectura(fichero, "se esperaba el número de monomios de la función %lu (entero >= 1)", k + 1);
		f->inicio[k + 1] = f->inicio[k] + (TYPE_DATA) valor;
	}
	f->num_monomios = f->inicio[f->num_polinomios];
}

// Un monomio por fila: el coeficiente y los N exponentes.
static void leer_monomios(FILE *fp, const char *fichero, Polinomios *f)
{
	TYPE_DATA N = f->num_variables, k, i, j;
	long valor;

	f->coef = (type_data *) reservar_memoria(f->num_monomios * sizeof(type_data), "los coeficientes");
	f->exp = (int *) reservar_memoria(f->num_monomios * N * sizeof(int), "los exponentes");
	for (k = 0; k < f->num_polinomios; k++) {
		for (i = f->inicio[k]; i < f->inicio[k + 1]; i++) {
			TYPE_DATA monomio = i - f->inicio[k] + 1;	// posición dentro de su función, para los mensajes
			if (!leer_real(fp, &f->coef[i]))
				error_lectura(fichero, "se esperaba el coeficiente del monomio %lu de la función %lu", monomio, k + 1);
			for (j = 0; j < N; j++) {
				if (!leer_entero(fp, &valor) || valor < 0)
					error_lectura(fichero, "se esperaba el exponente %lu del monomio %lu de la función %lu (entero >= 0)", j + 1, monomio, k + 1);
				f->exp[i * N + j] = (int) valor;
			}
		}
	}
}

// P y después una fila de N coordenadas por punto.
static void leer_puntos(FILE *fp, const char *fichero, TYPE_DATA num_variables, Puntos *pts)
{
	TYPE_DATA N = num_variables, p, j;
	long valor;

	if (!leer_entero(fp, &valor) || valor < 1)
		error_lectura(fichero, "se esperaba el número de puntos P (entero >= 1)");
	pts->num_puntos = (TYPE_DATA) valor;
	pts->num_variables = N;
	pts->coord = (type_data *) reservar_memoria(pts->num_puntos * N * sizeof(type_data), "las coordenadas de los puntos");
	for (p = 0; p < pts->num_puntos; p++)
		for (j = 0; j < N; j++)
			if (!leer_real(fp, &pts->coord[p * N + j]))
				error_lectura(fichero, "se esperaba la coordenada %lu del punto %lu", j + 1, p + 1);
}

// Lee el fichero de entrada completo. El formato está descrito en
// DECISIONES.md (decisión 3) y con un ejemplo en casos/enunciado.txt.
void leer_entrada(const char *fichero, Polinomios *funciones, Puntos *puntos)
{
	FILE *fp = fopen(fichero, "r");
	if (fp == NULL)
		error_lectura(fichero, "no se puede abrir (%s)", strerror(errno));

	leer_cabecera(fp, fichero, funciones);
	leer_monomios(fp, fichero, funciones);
	leer_puntos(fp, fichero, funciones->num_variables, puntos);

	// Si sobran datos, algún recuento (de monomios o de puntos) no cuadra con lo escrito
	saltar_comentarios(fp);
	if (fgetc(fp) != EOF)
		error_lectura(fichero, "sobran datos al final: revisa el número de monomios de cada función y el de puntos");
	fclose(fp);
}

/* ------------------------------------------------------------------ */
/* Impresión legible                                                   */
/* ------------------------------------------------------------------ */

static void imprimir_variable(TYPE_DATA j, TYPE_DATA num_variables)
{
	if (num_variables <= MAX_VARIABLES_CON_LETRA)
		printf("%c", LETRAS_VARIABLES[j]);
	else
		printf("x%lu", j + 1);
}

// Con una sola función se llama f, como en el enunciado; con varias, f1, f2, ...
static void imprimir_nombre_funcion(TYPE_DATA k, TYPE_DATA num_funciones)
{
	if (num_funciones == 1)
		printf("f");
	else
		printf("f%lu", k + 1);
}

// Escribe el monomio i en la misma notación que se usaría a mano: 3*x*y, x^2*z, 5.
// El signo hace de separador (" + " o " - "), salvo en el primer monomio.
static void imprimir_monomio(const Polinomios *p, TYPE_DATA i, int es_primero)
{
	TYPE_DATA N = p->num_variables, j;
	const int *e = &p->exp[i * N];
	double valor = fabs((double) p->coef[i]);
	int hay_variables = 0, hay_algo_escrito = 0;

	for (j = 0; j < N; j++)
		if (e[j] > 0)
			hay_variables = 1;

	if (es_primero) {
		if (p->coef[i] < 0)
			printf("-");
	} else {
		printf("%s", p->coef[i] < 0 ? " - " : " + ");
	}
	// El coeficiente 1 no se escribe delante de variables: x*y en lugar de 1*x*y
	if (valor != 1.0 || !hay_variables) {
		printf("%g", valor);
		hay_algo_escrito = 1;
	}
	for (j = 0; j < N; j++) {
		if (e[j] == 0)
			continue;
		if (hay_algo_escrito)
			printf("*");
		imprimir_variable(j, N);
		if (e[j] > 1)
			printf("^%d", e[j]);
		hay_algo_escrito = 1;
	}
}

// Escribe el polinomio k del conjunto, sin nombre ni salto de línea.
void imprimir_polinomio(const Polinomios *p, TYPE_DATA k)
{
	TYPE_DATA i;
	for (i = p->inicio[k]; i < p->inicio[k + 1]; i++)
		imprimir_monomio(p, i, i == p->inicio[k]);
}

// Las K funciones de entrada, una por línea: f = 3*x*y + 4*y*z + 2*x + 5
void imprimir_funciones(const Polinomios *f)
{
	TYPE_DATA k;
	for (k = 0; k < f->num_polinomios; k++) {
		imprimir_nombre_funcion(k, f->num_polinomios);
		printf(" = ");
		imprimir_polinomio(f, k);
		printf("\n");
	}
}

void imprimir_puntos(const Puntos *pts)
{
	TYPE_DATA N = pts->num_variables, p, j;
	for (p = 0; p < pts->num_puntos; p++) {
		printf("Punto %lu: (", p + 1);
		for (j = 0; j < N; j++)
			printf("%s%g", j == 0 ? "" : ", ", (double) pts->coord[p * N + j]);
		printf(")\n");
	}
}
