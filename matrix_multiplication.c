#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <time.h>

typedef struct matrix
{
	int** values;
	int height;
	int width;
} Matrix;

Matrix* allocate_matrix(int, int);
void free_matrix(Matrix*);
int get_matrix_value(Matrix*, int, int);
void set_matrix_value(Matrix*, int, int, int);
void fill_matrix_with_random_values(Matrix*, int, int);
void print_matrix(Matrix*);

void print_message_header();

int RANK, NUMPROCS;

int main(int argc, char** argv)
{
	srand(time(0));

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &NUMPROCS); // to learn how many proc we are
	MPI_Comm_rank(MPI_COMM_WORLD, &RANK); // to learn what is my rank

	if (RANK == 0)
	{
		Matrix* matrix = allocate_matrix(4, 4);
		fill_matrix_with_random_values(matrix, 0, 10);

		print_message_header();
		printf("Just created the following matrix :\n");
		print_matrix(matrix);
	}

	MPI_Finalize();
	return 0;
}

Matrix* allocate_matrix(int height, int width)
{
	Matrix* matrix = (Matrix*) malloc(sizeof(Matrix));
	matrix->values = (int**) malloc(sizeof(int*) * height);
	matrix->height = height;
	matrix->width = width;

	for (int i = 0; i < height; i++)
	{
		*(matrix->values + i) = (int*) malloc(sizeof(int) * width);
	}

	return matrix;
}

int get_matrix_value(Matrix* matrix, int height, int width)
{
	if (height >= matrix->height)
	{
		print_message_header();
		printf("Warning : attempting to read a value out of the matrix bound (reading value at coords (%d, %d) but the matrix's height is %d. Exiting.", height, width, matrix->height);
		exit(1);
	}

	if (width >= matrix->width)
	{
		print_message_header();
		printf("Warning : attempting to read a value out of the matrix bound (reading value at coords (%d, %d) but the matrix's width is %d. Exiting.", height, width, matrix->width);
		exit(1);
	}

	return matrix->values[height][width];
	//return *(*(matrix->values + height) + width);
}

void set_matrix_value(Matrix* matrix, int height, int width, int value)
{
	if (height >= matrix->height)
	{
		print_message_header();
		printf("Warning : attempting to write a value out of the matrix bound (writing value at coords (%d, %d) but the matrix's height is %d. Exiting.", height, width, matrix->height);
		exit(1);
	}

	if (width >= matrix->width)
	{
		print_message_header();
		printf("Warning : attempting to write a value out of the matrix bound (writing value at coords (%d, %d) but the matrix's width is %d. Exiting.", height, width, matrix->width);
		exit(1);
	}

	* ( * ( matrix->values + height ) + width ) = value;
}

void fill_matrix_with_random_values(Matrix* matrix, int lower_bound, int upper_bound)
{
	for(int i = 0; i < matrix->height; i++)
	{
		for(int j = 0; j < matrix->width; j++)
		{
			set_matrix_value(matrix, i, j, (rand() % (upper_bound - lower_bound)) + lower_bound);
		}
	}
}

void print_matrix(Matrix* matrix)
{
	for(int i = 0; i < matrix->height; i++)
	{
		printf("[matrix printing] ");

		for (int j = 0; j < matrix->width; j++)
		{
			printf("%d ", get_matrix_value(matrix, i, j));
		}

		printf("\n");
	}

	printf("\n");
}

void print_message_header()
{
	printf("[process n°%d] ", RANK);
}
