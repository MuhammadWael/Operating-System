#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
/*
      c_per_row.txt
      c_per_element.txt
     */
#define MAX_NUMBERS 20
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
    //case of ./MatMul inMartix1 inMartix2 outMatrix
    matrix_t mat1,mat2,resultMat;
    /*set to defult values
    char* out_file = "c";
    char* infile1 = "a";
    char* infile2 = "b";*/
    if(argc == 4)
    {
    	/*char *infile1 = (char *)malloc(sizeof(argv[1])/sizeof(char));
    	strcpy(result, str1);
    	in_file1 = argv[1];
    	in_file2 = argv[2];*/
        mat1 = read_matrix_from_file(string_concat(argv[1],".txt"));
        mat2 = read_matrix_from_file(string_concat(argv[2],".txt"));
        //for first method 
        resultMat = perMatrix(mat1,mat2);
        write_matrix_to_file(&resultMat ,string_concat(argv[3],"_per_matrix.txt"));
        
        perRow(&mat1,&mat2,&resultMat);
        write_matrix_to_file(&resultMat ,string_concat(argv[3],"_per_row.txt"));
        
        perElement(&mat1,&mat2,&resultMat);
        write_matrix_to_file(&resultMat ,string_concat(argv[3],"_per_element.txt"));
    }
    //case of ./MatMul 
    else if(argc == 1)
    {
        mat1 = read_matrix_from_file("a.txt");
        mat2 = read_matrix_from_file("b.txt");
        struct timeval stop, start;

    	gettimeofday(&start, NULL); //start checking time
    	
        resultMat = perMatrix(mat1,mat2);
        write_matrix_to_file(&resultMat ,string_concat("c","_per_matrix.txt"));
    	
    	gettimeofday(&stop, NULL); //end checking time
	printf("for method 1\n");
    	printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    	printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
        
	gettimeofday(&start, NULL); //start checking time
       
        perRow(&mat1,&mat2,&resultMat);
        write_matrix_to_file(&resultMat ,string_concat("c","_per_row.txt"));
            	
    	gettimeofday(&stop, NULL); //end checking time
	printf("for method 2\n");
    	printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    	printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    	
    	gettimeofday(&start, NULL); //start checking time
        perElement(&mat1,&mat2,&resultMat);
        write_matrix_to_file(&resultMat ,string_concat("c","_per_element.txt"));
            	
    	gettimeofday(&stop, NULL); //end checking time
	printf("for method 3\n");
    	printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    	printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    }
    //False case
    else
    {
        printf("unknown input format\nUsage: ./matMultp Mat1 Mat2 MatOut");
    }

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
                exit(EXIT_FAILURE);
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
	printf("Uncomapatble dimensions");
	exit(1);
}
//the result of multiplication should be  

matrix_t result_matrix;

int common = mat_1.col;
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


