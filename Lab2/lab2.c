#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#define MAX_NUMBERS 30
typedef struct 
{
    int row,col;
    int data[MAX_NUMBERS][MAX_NUMBERS];
}matrix_t;

typedef struct
{
    matrix_t *mat1;
    matrix_t *mat2;
    matrix_t *result;
    int row_start;
    int row_end;
} thread_data_t;

matrix_t read_matrix_from_file(char *mat);
void write_matrix_to_file(matrix_t *mat,char* name);
matrix_t perMatrix(matrix_t mat_1,matrix_t mat_2);
void perElement(matrix_t *mat1, matrix_t *mat2, matrix_t *result);
void perRow(matrix_t *mat1, matrix_t *mat2, matrix_t *result);
void *multiply_element(void *arg);
void *multiply_row(void *arg);
char *string_concat(char *str1, char *str2);


int main(int argc,char* argv[])
{   
    
    //insizlize names
    char* infile1 = malloc(64);
    char* infile2 = malloc(64);
    char* out_file = malloc(64);    
    //case of ./MatMul inMartix1 inMartix2 outMatrix
    if(argc == 4)
    {	
    	strcpy(infile1, argv[1]);
    	strcpy(infile2, argv[2]);
    	strcpy(out_file, argv[3]);

    }
    //case of ./MatMul 
    else if(argc == 1)
    {	//defult values of files a b c
        strcpy(infile1, "a");
    	strcpy(infile2, "b");
    	strcpy(out_file, "c");
        
    }
    //False case
    else
    {
        printf("unknown input format\nUsage: ./matMultp Mat1 Mat2 MatOut\nor ./matMultp set input to a b c");
        exit(EXIT_FAILURE);//exit of program
    }
    
    matrix_t mat1,mat2,resultMat;
    struct timeval stop, start;
    mat1 = read_matrix_from_file(string_concat(infile1,".txt"));
    mat2 = read_matrix_from_file(string_concat(infile2,".txt"));
    
    gettimeofday(&start, NULL); //start checking time
    
    resultMat = perMatrix(mat1,mat2);//running method 1
    write_matrix_to_file(&resultMat ,string_concat(out_file,"_per_matrix.txt"));
    
    gettimeofday(&stop, NULL); //end checking time
    printf("for thread per matrix\n");
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);

    gettimeofday(&start, NULL); //start checking time

    perRow(&mat1,&mat2,&resultMat);
    write_matrix_to_file(&resultMat ,string_concat(out_file,"_per_row.txt"));
    	
    gettimeofday(&stop, NULL); //end checking time
    printf("for thread per row\n");
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);

    gettimeofday(&start, NULL); //start checking time
    perElement(&mat1,&mat2,&resultMat);
    write_matrix_to_file(&resultMat ,string_concat(out_file,"_per_element.txt"));
    	
    gettimeofday(&stop, NULL); //end checking time
    printf("for thread per element\n");
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);

    // Free dynamically allocated memory
    free(infile1);
    free(infile2);
    free(out_file);
    
    return 0;
}

matrix_t read_matrix_from_file(char *mat)
{
    matrix_t matrix;
    FILE *file = fopen(mat,"r");
    if(file == NULL)
    {
        printf("Error opening file %s", mat);
        exit(EXIT_FAILURE);
    }
    fscanf(file, "row=%d col=%d", &matrix.row, &matrix.col);
    for(int i = 0; i < matrix.row; i++)
    {
        for(int j = 0; j < matrix.col; j++)
        {
            if (fscanf(file, "%d", &matrix.data[i][j]) != 1) {
                printf("Error: Insufficient data in the file.\n");
                exit(EXIT_FAILURE);//exit of program
            }
        }
    }
    fclose(file);

    return matrix;
}


void write_matrix_to_file(matrix_t *mat,char* name)
{
    FILE* file = fopen(name,"w");
    if(file == NULL)
    {
        printf("couldn't make the file\n");
    }
    fprintf(file,"row=%d\tcol=%d\n",mat->row,mat->col);
    for(int i = 0;i < mat->row;i++)
    {
        for(int j = 0;j < mat->col;j++)
        {
            fprintf(file,"%d\t",mat->data[i][j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

//normal matrix multiplication 
matrix_t perMatrix(matrix_t mat_1,matrix_t mat_2)
{
//rows of first must equal to coloumns of second 
if(mat_1.col != mat_2.row)
{
	printf("Uncomapatble dimensions\n");
	exit(EXIT_FAILURE);
}

matrix_t result_matrix;
int common = mat_1.col;// the common dimenstion of matrix
//dimensions of resulted matrix
result_matrix.row = mat_1.row;
result_matrix.col = mat_2.col;

	for(int i = 0;i < result_matrix.row;i++)
	{
		for(int j = 0;j < result_matrix.col;j++)
		{
		result_matrix.data[i][j] = 0;
			for(int k = 0;k < common;k++)
			{
				result_matrix.data[i][j] += mat_1.data[i][k] * mat_2.data[k][j];
			}
		}
	}
	return result_matrix;
}
//to allocate memory for concatinating file names
char *string_concat(char *str1, char *str2) {
    char *result = (char *)malloc(strlen(str1) + strlen(str2) + 1);
    strcpy(result, str1);
    strcat(result, str2);

    return result;
}

void perRow(matrix_t *mat1, matrix_t *mat2, matrix_t *result)
{
    pthread_t threads[MAX_NUMBERS];
    thread_data_t data[MAX_NUMBERS];

    for (int i = 0; i < mat1->row; i++)
    {
        data[i].mat1 = mat1;
        data[i].mat2 = mat2;
        data[i].result = result;
        data[i].row_start = i;
        data[i].row_end = i + 1;

        pthread_create(&threads[i], NULL, multiply_row, (void *)&data[i]);
    }

    // Join threads
    for (int i = 0; i < mat1->row; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

void *multiply_row(void *arg)
{
    thread_data_t *data = (thread_data_t *)arg;

    for (int i = data->row_start; i < data->row_end; i++)
    {
        for (int j = 0; j < data->mat2->col; j++)
        {
            int sum = 0;
            for (int k = 0; k < data->mat1->col; k++)
            {
                sum += data->mat1->data[i][k] * data->mat2->data[k][j];
            }
            data->result->data[i][j] = sum;
        }
    }

    pthread_exit(NULL);
}

void perElement(matrix_t *mat1, matrix_t *mat2, matrix_t *result)
{
    pthread_t threads[MAX_NUMBERS][MAX_NUMBERS];
    thread_data_t data[MAX_NUMBERS][MAX_NUMBERS];

    for (int i = 0; i < mat1->row; i++)
    {
        for (int j = 0; j < mat2->col; j++)
        {
            data[i][j].mat1 = mat1;
            data[i][j].mat2 = mat2;
            data[i][j].result = result;
            data[i][j].row_start = i;
            data[i][j].row_end = i + 1;

            pthread_create(&threads[i][j], NULL, multiply_element, (void *)&data[i][j]);
        }
    }

    // Join threads
    for (int i = 0; i < mat1->row; i++)
    {
        for (int j = 0; j < mat2->col; j++)
        {
            pthread_join(threads[i][j], NULL);
        }
    }
}

void *multiply_element(void *arg)
{
    thread_data_t *data = (thread_data_t *)arg;

    int i = data->row_start;
    int j = data->row_end - 1;

    int sum = 0;
    for (int k = 0; k < data->mat1->col; k++)
    {
        sum += data->mat1->data[i][k] * data->mat2->data[k][j];
    }
    data->result->data[i][j] = sum;

    pthread_exit(NULL);
}

