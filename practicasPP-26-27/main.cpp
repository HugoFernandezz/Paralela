#include "definitions.h"
#include "wtime.h"
//#include "fichero.h"
#include "fichero_cpu.h"
#include "fichero_omp.h"
#include "fichero_gpu.h"


using namespace std;

int main (int argc, char **argv){

	char c;
	int device, data_print = 0, len;
	TYPE_DATA max_rows = 0, max_columns = 0, num_series = 0, mode, iterations, i, blockSize, th;
	//type_data *data_original....,
	double time_init,time_end;

	//sep = (char *)malloc(sizeof(char)*5);
	//separator = ';';
	mode = 1; //SEQUENTIAL MODE
	i = 0;
	char buffer[100];
         while (i < argc) {
	      strcpy(buffer,argv[i]);
	      //printf("%s\n",buffer);
              if ((strcmp(buffer,"-t") == 0) || (strcmp(buffer,"-v") == 0) || (strcmp(buffer,"-m") == 0) || (strcmp(buffer,"-o") == 0) || (strcmp(buffer,"-s") == 0) || (strcmp(buffer,"-b") == 0))
                 {
	          c = buffer[1];
        	  i++;	       
		  switch (c) {
		    	case 'v':
	     			printf("algorithm Calculation v.1.0\n\n");
				return 0;
			 case 'm':
                                mode = atoi(argv[i]);
                                break;
			case 'o':
				//strcpy(dir_source,argv[i]);
				//printf("%s\n",dir_source);
				break;
			case 's':
				//strcpy(sep,argv[i]);
				//separator = sep[0];
				break;
			 case 'b':
                            	blockSize = (TYPE_DATA) atoi(argv[i]);
				break;
			case 't':
				//th = atoi(argv[i]);
				break;
			case 'h':
			case '?':
		      		printf("Usage:\tejecucion.... [-s \"char_separator\" -m MODE] [-n DEVICE] [-h | -? HELP] \n");
				printf("\t<Params>\n");
			    	printf("\t\t-v\t\tOutput version information and exit\n");		    	
	    			return 0;
    	  		}
		}
	      i++;
 	}	
	device = 0;//ngpu;
  	//CONFIGURATION SIMULATION
	//data_original = read... (....rows,columns,num_series,separator);
	//for (i = 0; i < num_series; i++)
	//	show_data(data_original...._rows,columns);
	
	printf("DATA. rows %d columns %d \n\n",max_rows,max_columns); 	
	switch (mode) {
		case 0:
			printf("\nEJECUCION ALGORITHM SEQUENTIAL MODE...\n");
                        printf("********************************************************\n");
			//omp_set_num_threads(1);
			time_init = wtime();
			//resultao = ejecucion_CPU(.....);
			time_end = wtime() - time_init;
                        break; 
		case 1:
			printf("\nEJECUCION ALGORITHM OPEN-MP MODE...\n");
			printf("*************************************\n");			
			//omp_set_num_threads(th); //aqui o dentro de la funcion
			time_init = wtime();
			//resultado = ejecucion_OMP(....th);
			time_end = wtime() - time_init;
			break;			
		case 2:
			printf("\n EJECUCION AlGORITHM GPU MODE...\n");
			printf("*************************************\n");
			time_init = wtime();
			//resultado = ejecucion_GPU(datos, blocksize..........
			time_end = wtime() - time_init;
			break;
	  	default:
      			printf("Wrong mode type: %d.  Use -h for help.\n", mode);
			return 0;
	}
	printf("\nTime: %f seg\n",time_end);

	return 0;	

}

