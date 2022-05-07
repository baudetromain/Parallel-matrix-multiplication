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
Matrix* read_matrix_from_file(char*);
Matrix* generate_random_matrix();

int* generate_column_vector(int);
void print_vector(int*, int);
int scalar_product(int*, int*, int);

void print_message_header();

#define DEFAULT_MATRIX_SIZE 8
#define DEFAULT_MATRIX_LOWEST_VALUE -10
#define DEFAULT_MATRIX_HIGHEST_VALUE 10

int RANK, NUMPROCS;
int LINES_PER_THREAD, MATRIX_SIZE;

int main(int argc, char** argv)
{
	srand(time(0));

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &NUMPROCS); // to learn how many proc we are
	MPI_Comm_rank(MPI_COMM_WORLD, &RANK); // to learn what is my rank

	Matrix* matrix;
	int* column_vector;

	if (RANK == 0)
	{
		if (argc == 2)
		{
			char* file_name = argv[1];
			matrix = read_matrix_from_file(file_name);
		}
		else
		{
			matrix = generate_random_matrix();
		}

		MATRIX_SIZE = matrix->width;

		print_message_header();
		printf(argc == 2 ? "Read the following matrix in input file :\n" : "Just created the following matrix :\n");
		print_matrix(matrix);

		LINES_PER_THREAD = MATRIX_SIZE / NUMPROCS;

		column_vector = generate_column_vector(MATRIX_SIZE);

		print_message_header();
		printf("Just generated the following column vector (line displayed) : \n");
		print_vector(column_vector, MATRIX_SIZE);

		int* results = (void*) malloc(sizeof(int) * LINES_PER_THREAD);

		for (int i = 0; i < LINES_PER_THREAD; i++)
		{
			*(results + i) = scalar_product(*(matrix->values + i), column_vector, MATRIX_SIZE);
		}

		// send the matrix size
		MPI_Send((void*) &MATRIX_SIZE,
				 1,
				 MPI_INT,
				 1,
				 0,
				 MPI_COMM_WORLD);

		print_message_header();
		printf("Just sent the matrix size (%d) to thread n°%d\n", MATRIX_SIZE, 1);

		// send the calculated values
		MPI_Send((void*) results,
				 LINES_PER_THREAD,
				 MPI_INT,
				 1,
				 1,
				 MPI_COMM_WORLD);

		print_message_header();
		printf("Just sent the following calculated results to thread n°%d : ", 1);
		print_vector(results, LINES_PER_THREAD);

		MPI_Send((void*) column_vector,
				 MATRIX_SIZE,
				 MPI_INT,
				 1,
				 2,
				 MPI_COMM_WORLD);

		print_message_header();
		printf("Just sent the following column matrix to thread n°%d : ", 1);
		print_vector(column_vector, MATRIX_SIZE);

		// send the matrix lines
		for (int i = LINES_PER_THREAD; i < MATRIX_SIZE; i++)
		{
			MPI_Send((void*) *(matrix->values + i),
					 MATRIX_SIZE,
					 MPI_INT,
					 1,
					 3 + i - LINES_PER_THREAD,
					 MPI_COMM_WORLD);

			print_message_header();
			printf("Just sent the following line to thread n°%d : ", 1);
			print_vector(*(matrix->values + i), MATRIX_SIZE);
		}

	}

	else
	{
		MPI_Recv((void*) &MATRIX_SIZE,
				 1,
				 MPI_INT,
				 RANK - 1,
				 0,
				 MPI_COMM_WORLD,
				 NULL);

		LINES_PER_THREAD = MATRIX_SIZE / NUMPROCS;

		print_message_header();
		printf("Just received the matrix size (%d) from thread n°%d\n", MATRIX_SIZE, RANK - 1);

		int* results = malloc(sizeof(int) * (LINES_PER_THREAD * RANK + LINES_PER_THREAD));

		MPI_Recv((void*) results,
				 LINES_PER_THREAD * RANK,
				 MPI_INT,
				 RANK - 1,
				 1,
				 MPI_COMM_WORLD,
				 NULL);

		print_message_header();
		printf("Just received the following calculated results from thread n°%d : ", RANK - 1);
		print_vector(results, LINES_PER_THREAD);

		int* column_vector = malloc(sizeof(int) * MATRIX_SIZE);

		MPI_Recv((void*) column_vector,
				 MATRIX_SIZE,
				 MPI_INT,
				 RANK - 1,
				 2,
				 MPI_COMM_WORLD,
				 NULL);

		print_message_header();
		printf("Just received the following column matrix from thread n°%d : ", RANK - 1);
		print_vector(column_vector, MATRIX_SIZE);

		Matrix* lines = allocate_matrix(MATRIX_SIZE - LINES_PER_THREAD * RANK, MATRIX_SIZE);

		for (int i = LINES_PER_THREAD * RANK; i < MATRIX_SIZE; i++)
		{
			MPI_Recv((void*) *(lines->values + i - LINES_PER_THREAD * RANK),
					 MATRIX_SIZE,
					 MPI_INT,
					 RANK - 1,
					 3 + i - LINES_PER_THREAD * RANK,
					 MPI_COMM_WORLD,
					 NULL);

			print_message_header();
			printf("Just received the following line to thread n°%d : ", 1);
			print_vector(*(lines->values + i - LINES_PER_THREAD * RANK), MATRIX_SIZE);
		}

		for (int i = 0; i < LINES_PER_THREAD; i++)
		{
			*(results + i + LINES_PER_THREAD * RANK) = scalar_product(*(lines->values + i), column_vector, MATRIX_SIZE);
		}

		// send the matrix size
		MPI_Send((void*) &MATRIX_SIZE,
				 1,
				 MPI_INT,
				 RANK + 1,
				 0,
				 MPI_COMM_WORLD);

		print_message_header();
		printf("Just sent the matrix size (%d) to thread n°%d\n", MATRIX_SIZE, RANK + 1);

		// send the calculated values
		MPI_Send((void*) results,
				 (LINES_PER_THREAD * RANK + LINES_PER_THREAD),
				 MPI_INT,
				 RANK + 1,
				 1,
				 MPI_COMM_WORLD);

		print_message_header();
		printf("Just sent the following calculated results to thread n°%d : ", 1);
		print_vector(results, LINES_PER_THREAD);

		MPI_Send((void*) column_vector,
				 MATRIX_SIZE,
				 MPI_INT,
				 RANK + 1,
				 2,
				 MPI_COMM_WORLD);

		print_message_header();
		printf("Just sent the following column matrix to thread n°%d : ", 1);
		print_vector(column_vector, MATRIX_SIZE);

		// send the matrix lines
		for (int i = (LINES_PER_THREAD * RANK + LINES_PER_THREAD); i < MATRIX_SIZE; i++)
		{
			MPI_Send((void*) *(lines->values + i - (LINES_PER_THREAD * RANK + LINES_PER_THREAD)),
					 MATRIX_SIZE,
					 MPI_INT,
					 1,
					 3 + i - (LINES_PER_THREAD * RANK + LINES_PER_THREAD),
					 MPI_COMM_WORLD);

			print_message_header();
			printf("Just sent the following line to thread n°%d : ", 1);
			print_vector(*(lines->values + i), MATRIX_SIZE);
		}
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

	matrix->values[height][width] = value;
	//* ( * ( matrix->values + height ) + width ) = value;
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

int* generate_column_vector(int size)
{
	int* vector = malloc(sizeof(int) * size);

	for (int i = 0; i < size; i++)
	{
		*(vector + i) = (rand() % (DEFAULT_MATRIX_HIGHEST_VALUE - DEFAULT_MATRIX_LOWEST_VALUE)) + DEFAULT_MATRIX_LOWEST_VALUE;
	}

	return vector;
}

void print_vector(int* vector, int size)
{
	for (int i = 0; i < size; i++)
	{
		printf("%d ", *(vector + i));
	}

	printf("\n");
}

int scalar_product(int* line, int* column, int size)
{
	int result = 0;

	for (int i = 0; i < size; i++)
	{
		result += *(line + i) * *(column + i);
	}

	return result;
}

Matrix* read_matrix_from_file(char* file_name)
{
	FILE *file = fopen(file_name, "r");
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
	Matrix* matrix = allocate_matrix(counter, counter);

	fseek(file, 0, SEEK_SET);
	int val = 0;

	for (int i = 0; i < matrix->height; i++)
	{
		for (int j = 0; j < matrix->width; j++)
		{
			fscanf(file, "%d", &val);
			set_matrix_value(matrix, i, j, val);
		}
	}

	return matrix;
}

Matrix* generate_random_matrix()
{
	Matrix* matrix = allocate_matrix(DEFAULT_MATRIX_SIZE, DEFAULT_MATRIX_SIZE);
	fill_matrix_with_random_values(matrix, DEFAULT_MATRIX_LOWEST_VALUE, DEFAULT_MATRIX_HIGHEST_VALUE);

	return matrix;
}
