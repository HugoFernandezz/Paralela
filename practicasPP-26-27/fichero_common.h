#ifndef _MULTISERIES_COMMON_H_
#define _MULTISERIES_COMMON_H_
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <math.h>
#include <time.h>
#include <ostream>
#include <iomanip>
#include "definitions.h"

// memoria
extern void *reservar_memoria(size_t bytes, const char *que);
extern void liberar_polinomios(Polinomios *p);
extern void liberar_puntos(Puntos *p);
// lectura de la entrada
extern void leer_entrada(const char *fichero, Polinomios *funciones, Puntos *puntos);
// impresión legible
extern void imprimir_polinomio(const Polinomios *p, TYPE_DATA k);
extern void imprimir_funciones(const Polinomios *f);
extern void imprimir_puntos(const Puntos *pts);

#endif /* COMMON_H_ */
