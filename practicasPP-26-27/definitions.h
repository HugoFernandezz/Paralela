#ifndef _DEFINITIONS_H
#define _DEFINITIONS_H
#include "type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <iostream>
#include <iomanip>
#include <cfloat>
#include <limits>


//#define BLOCK_SIZE 512
//#define BLOCK_64 64
//#define BLOCK_SM 64
//#define BLOCK_128 128
//#define BLOCK_256 256

// Conjunto de polinomios en N variables, guardado como struct de arrays densos
// (decisiones 4 y 7 de DECISIONES.md). Guarda las K funciones de entrada y,
// desde A2, también sus derivadas parciales.
// Los monomios de todos los polinomios van seguidos en memoria: el polinomio k
// ocupa los monomios inicio[k] .. inicio[k+1]-1. Así el conjunto entero pasa a
// la GPU con tres copias, sin punteros anidados.
typedef struct {
	TYPE_DATA num_variables;	// N
	TYPE_DATA num_polinomios;	// K
	TYPE_DATA num_monomios;		// total de monomios de todos los polinomios
	TYPE_DATA *inicio;		// [num_polinomios + 1]
	type_data *coef;		// [num_monomios]
	int *exp;			// [num_monomios * N]: exp[i*N + j] = exponente de la variable j en el monomio i
} Polinomios;

// Puntos donde se evalúa el gradiente: P filas de N coordenadas.
typedef struct {
	TYPE_DATA num_puntos;		// P
	TYPE_DATA num_variables;	// N
	type_data *coord;		// [num_puntos * N]: coord[p*N + j] = valor de la variable j en el punto p
} Puntos;

/*typedef struct prop_devices
{
	bool busy;
	TYPE_DATA id;
}GPU;*/

#define MAX_CADENA 1024
#define MAX_LINE 16384

// Valores por defecto de las opciones -t y -b de main.cpp
#define HILOS_POR_DEFECTO 1
#define TAM_BLOQUE_POR_DEFECTO 256

#define BYTES_POR_MB (1024.0 * 1024.0)
#endif
