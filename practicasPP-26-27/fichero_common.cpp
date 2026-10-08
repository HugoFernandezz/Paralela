#include "definitions.h"
#include "fichero_common.h"
#include "wtime.h"

using namespace std;

type_data extract_number (char *buffer, unsigned int *pos, char separator)
{	/*unsigned int k = 0;
	char c;
	c = buffer[*pos];
	char temp[100];
	//type_data number;
	while (buffer[*pos] == ' ') (*pos)++;
        c = buffer[*pos];
	while ((c != separator) && (c != '\n'))
	{
		temp[k] = buffer[*pos];
		k++;(*pos)++;
		c = buffer[*pos];
	}	
	(*pos)++;
	temp[k]='\0';
	if (c == '\n') temp[k-1] = '\0';
	//printf("%s\n",temp);
	if (temp[0] == 'N') return (FLT_MAX);
	return ((type_data)atof(temp));*/
}

unsigned int extract_columns (char *buffer, char separator)
{
/*        unsigned int c_t = 1, i = 0;
        unsigned int len = strlen(buffer);
	 while (buffer[i] == separator) i++;
        char c = buffer[i];
        while ((c != '\0') && (i < len))
        {
                if (c == separator)
                {
                        while (c == separator)
                        {
                                i++;
                                c = buffer[i];
                        }
                        c_t++;
                }
                else
                {
                        i++;
                        c = buffer[i];
                }
        }
        return c_t;
*/
}

extern void exam_data (char *input, TYPE_DATA *rows, TYPE_DATA *columns, char separator) {

        /*
	TYPE_DATA r_t = 1;
        char *buffer = (char *)malloc(sizeof(char)*MAX_LINE);

        FILE *fp;
        if ((fp =  fopen(input,"r")) == NULL) {
                printf("ReadInput(): Can't open file \"%s\"\n", input);
                exit(1);
        }
	fgets(buffer,MAX_LINE,fp);
	*columns = extract_columns(buffer,separator);
	
	while(fgets(buffer,MAX_LINE,fp) != NULL ) r_t++;
        *rows = r_t;
	printf("%lu %lu %lu\n",*rows,*columns,*rows * *columns);
	free(buffer);
	*/
}

extern void read_data (char *input, TYPE_DATA rows, TYPE_DATA columns, type_data *m, char separator)
{
	  /*unsigned int pos = 0,i = 0, j = 0;

	  char buffer[MAX_LINE+1];// = (char *)malloc(sizeof(char)*MAX_LINE);
	  FILE *fp;
	  if ((fp =  fopen(input,"r")) == NULL) {
	  	printf("ReadInput(): Can't open file \"%s\"\n", input);
		exit(1);
	  }
	  while((fgets(buffer,MAX_LINE,fp) != NULL) && (i < rows))
	  {
	  	for (j = 0;j < columns;j++)
		{
			m[i*columns + j] = extract_number(buffer,&pos,separator);
			//printf("%f \n",m[i*columns + j]);
		}
		pos = 0;
		i++;
		//printf("%u %u\n",i,j);
	  }
	  fclose(fp);
	  */
}


