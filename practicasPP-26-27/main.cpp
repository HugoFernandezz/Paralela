#include "definitions.h"
#include "wtime.h"
#include "fichero_common.h"
#include "fichero_cpu.h"
#include "fichero_omp.h"
#include "fichero_gpu.h"


using namespace std;

static void mostrar_uso(const char *programa)
{
	printf("Uso: %s -f FICHERO [-m MODO] [-t HILOS] [-b BLOQUE] [-d] [-v] [-h]\n", programa);
	printf("\t<Params>\n");
	printf("\t\t-f FICHERO\tfichero de entrada con las funciones y los puntos\n");
	printf("\t\t-m MODO\t\t0 secuencial (por defecto), 1 OpenMP, 2 GPU\n");
	printf("\t\t-t HILOS\thilos de OpenMP (por defecto %d)\n", HILOS_POR_DEFECTO);
	printf("\t\t-b BLOQUE\thilos por bloque en la GPU (por defecto %d)\n", TAM_BLOQUE_POR_DEFECTO);
	printf("\t\t-d\t\tmuestra las funciones y los puntos leídos\n");
	printf("\t\t-v\t\tOutput version information and exit\n");
	printf("\t\t-h\t\tmuestra esta ayuda y termina\n");
}

int main (int argc, char **argv){

	char c;
	int i, data_print = 0;
	TYPE_DATA mode, blockSize, th;
	const char *fichero = NULL;
	Polinomios funciones, derivadas;
	Puntos puntos;
	double time_init, time_end, tiempo_lectura, tiempo_derivacion;

	mode = 0; //SEQUENTIAL MODE
	th = HILOS_POR_DEFECTO;
	blockSize = TAM_BLOQUE_POR_DEFECTO;
	// Cada opción es una letra precedida de '-'. Las opciones -f, -m, -t y -b
	// llevan su valor en el argumento siguiente.
	for (i = 1; i < argc; i++) {
		c = (argv[i][0] == '-' && strlen(argv[i]) == 2) ? argv[i][1] : 0;
		if (c == 'f' || c == 'm' || c == 't' || c == 'b') {
			if (i + 1 >= argc) {
				fprintf(stderr, "Falta el valor de la opción %s\n", argv[i]);
				return 1;
			}
			i++;
		}
		switch (c) {
			case 'f':
				fichero = argv[i];
				break;
			case 'm':
				mode = (TYPE_DATA) atol(argv[i]);
				break;
			case 't':
				th = (TYPE_DATA) atol(argv[i]);
				break;
			case 'b':
				blockSize = (TYPE_DATA) atol(argv[i]);
				break;
			case 'd':
				data_print = 1;
				break;
			case 'v':
				printf("algorithm Calculation v.1.0\n\n");
				return 0;
			case 'h':
			case '?':
				mostrar_uso(argv[0]);
				return 0;
			default:
				fprintf(stderr, "Opción desconocida: %s\n", argv[i]);
				mostrar_uso(argv[0]);
				return 1;
		}
	}
	if (fichero == NULL) {
		fprintf(stderr, "Falta el fichero de entrada (-f)\n");
		mostrar_uso(argv[0]);
		return 1;
	}

	// La lectura se mide aparte: no forma parte del cálculo que se paraleliza
	time_init = wtime();
	leer_entrada(fichero, &funciones, &puntos);
	tiempo_lectura = wtime() - time_init;

	// PASOS 1 y 2: derivación simbólica. Se hace una sola vez, en código común a
	// las tres versiones, y se mide aparte: es la parte que no se paraleliza.
	time_init = wtime();
	derivar(&funciones, &derivadas);
	tiempo_derivacion = wtime() - time_init;

	printf("DATA. %lu variables, %lu funciones con %lu monomios en total (%.2f MB), %lu puntos\n",
	       funciones.num_variables, funciones.num_polinomios, funciones.num_monomios,
	       (double) bytes_polinomios(&funciones) / BYTES_POR_MB, puntos.num_puntos);
	printf("DERIVADAS. %lu polinomios, %lu monomios en total (%.2f MB)\n",
	       derivadas.num_polinomios, derivadas.num_monomios, (double) bytes_polinomios(&derivadas) / BYTES_POR_MB);
	printf("Tiempo de lectura: %f seg\n", tiempo_lectura);
	printf("Tiempo de derivacion: %f seg\n", tiempo_derivacion);
	if (data_print) {
		printf("\n");
		imprimir_funciones(&funciones);
		imprimir_puntos(&puntos);
		printf("\n");
		imprimir_paso1(&funciones, &derivadas);
		printf("\n");
		imprimir_paso2(&funciones, &derivadas);
	}

	switch (mode) {
		case 0:
			printf("\nEJECUCION ALGORITHM SEQUENTIAL MODE...\n");
                        printf("********************************************************\n");
			time_init = wtime();
			//resultao = ejecucion_CPU(.....);
			time_end = wtime() - time_init;
                        break;
		case 1:
			printf("\nEJECUCION ALGORITHM OPEN-MP MODE (%lu hilos)...\n", th);
			printf("*************************************\n");
			//omp_set_num_threads(th); //aqui o dentro de la funcion
			time_init = wtime();
			//resultado = ejecucion_OMP(....th);
			time_end = wtime() - time_init;
			break;
		case 2:
			printf("\n EJECUCION AlGORITHM GPU MODE (bloques de %lu hilos)...\n", blockSize);
			printf("*************************************\n");
			time_init = wtime();
			//resultado = ejecucion_GPU(datos, blocksize..........
			time_end = wtime() - time_init;
			break;
	  	default:
      			fprintf(stderr, "Wrong mode type: %lu.  Use -h for help.\n", mode);
			liberar_polinomios(&funciones);
			liberar_polinomios(&derivadas);
			liberar_puntos(&puntos);
			return 1;
	}
	printf("\nTime: %f seg\n",time_end);

	liberar_polinomios(&funciones);
	liberar_polinomios(&derivadas);
	liberar_puntos(&puntos);
	return 0;

}

