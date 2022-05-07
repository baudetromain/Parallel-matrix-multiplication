#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>


// square matrix structure definition
typedef struct
{
	int size;
	int* data;
} SquareMatrix;

// column vector structure definition
typedef struct
{
	int size;
	int* data;
} ColumnVector;


// methods headers
SquareMatrix* allocate_square_matrix(int size);

void free_square_matrix(SquareMatrix* matrix);

SquareMatrix* create_square_matrix_from_file(char* file_name);

void print_square_matrix(SquareMatrix* matrix);

ColumnVector* allocate_column_vector(int size);

void free_column_vector(ColumnVector* matrix);

ColumnVector* create_column_vector_from_file(char* file_name);

void print_column_vector(ColumnVector* matrix);


// variables for the rank and the ampount of processors
int rank, procs;

// main function
int main(int argc, char** argv)
{
	// MPI initialization
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &procs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// read matrix and column vector from input
	if (rank == 0)
	{
		SquareMatrix* matrix;
		ColumnVector* column_vector;

		// if bad command usage, print error message and exit
		if (argc != 3)
		{
			printf("Usage: mpirun ./matrix_multiplication_better <matrix_file> <vector_file>\n");

			// exit program
			MPI_Finalize();
			return 1;
		}

		// read matrix and column vector from file
		matrix = create_square_matrix_from_file(argv[1]);
		column_vector = create_column_vector_from_file(argv[2]);

		// print matrix and column vector
		print_square_matrix(matrix);
		print_column_vector(column_vector);
	}

	MPI_Finalize();
	return 0;
}

// function to allocate a square matrix of size size
SquareMatrix* allocate_square_matrix(int size)
{
	SquareMatrix* matrix = (SquareMatrix*) malloc(sizeof(SquareMatrix));
	matrix->size = size;
	matrix->data = (int*) malloc((size * size) * sizeof(int));
	return matrix;
}

// method to free a matrix
void free_square_matrix(SquareMatrix* matrix)
{
	free(matrix->data);
	free(matrix);
}

// function read_matrix_from_file(char** filename) to read a matrix from a file
SquareMatrix* create_square_matrix_from_file(char* filename)
{
	FILE* file = fopen(filename, "r");
	int c;
	int counter = 0;

	//we first read one single line to find the size of the matrix
	while ((c = fgetc(file)) != EOF)
	{
		//printf("%c ",c);
		if (c == ' ')
		{
			counter++;
		}
		if (c == '\n')
		{
			break;
		}
	}

	//Now read the file to build the matrix
	SquareMatrix* matrix = allocate_square_matrix(counter);

	fseek(file, 0, SEEK_SET);
	int val = 0;

	for (int i = 0; i < (matrix->size) * (matrix->size); i++)
	{
		fscanf(file, "%d", &val);
		matrix->data[i] = val;
	}

	return matrix;
}

// function to print a matrix
void print_square_matrix(SquareMatrix* matrix)
{
	for (int i = 0; i < matrix->size; i++)
	{
		for (int j = 0; j < matrix->size; j++)
		{
			printf("%d ", matrix->data[i * matrix->size + j]);
		}
		printf("\n");
	}
}

ColumnVector* allocate_column_vector(int size)
{
	ColumnVector* vector = (ColumnVector*) malloc(sizeof(ColumnVector));
	vector->size = size;
	vector->data = (int*) malloc(size * sizeof(int));
	return vector;
}

void free_column_vector(ColumnVector* vector)
{
	free(vector->data);
	free(vector);
}

ColumnVector* create_column_vector_from_file(char* filename)
{
	FILE* file = fopen(filename, "r");
	int c;
	int counter = 0;

	//we first read one single line to find the size of the matrix
	while ((c = fgetc(file)) != EOF)
	{
		if (c == ' ')
		{
			counter++;
		}
		if (c == '\n')
		{
			break;
		}
	}

	//Now read the file to build the matrix
	ColumnVector* vector = allocate_column_vector(counter);

	fseek(file, 0, SEEK_SET);
	int val = 0;

	for (int i = 0; i < vector->size; i++)
	{
		fscanf(file, "%d", &val);
		vector->data[i] = val;
	}

	return vector;
}

void print_column_vector(ColumnVector* vector)
{
	for (int i = 0; i < vector->size; i++)
	{
		printf("%d ", vector->data[i]);
	}
	printf("\n");
}
