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

// includes, project
extern void read_data (char *, TYPE_DATA, TYPE_DATA, type_data *, char);
extern char *extract_name (char *);
extern void exam_data (char *, TYPE_DATA *, TYPE_DATA *, char );
//auxiliar functions
extern void show_data (type_data *, unsigned int, unsigned int);
extern void show_data_file (type_data *m, type_data *c, unsigned int c_numbers, unsigned int rows, unsigned int columns, char *output);

#endif /* COMMON_H_ */
