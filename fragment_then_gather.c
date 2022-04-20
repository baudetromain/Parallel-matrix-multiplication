#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <time.h>

typedef struct array
{
	int* array;
	int size;
} Array;

void scan_args(int, char**);
void print_message_header();

Array* instanciate_array(int);
void free_array(Array*);
void fill_array_with_random_value(Array*, int, int);
void fill_array_with_given_values(Array*, int*);
void print_array(Array*);
void print_array_part(Array*, int, int);

int ARRAY_SIZE, SUBARRAYS_SIZE;
int RANK, NUMPROCS;

int main(int argc, char** argv)
{
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &NUMPROCS); // to learn how many proc we are
	MPI_Comm_rank(MPI_COMM_WORLD, &RANK); // to learn what is my rank

	ARRAY_SIZE = 10 * NUMPROCS;

	scan_args(argc, argv);

	SUBARRAYS_SIZE = ARRAY_SIZE / NUMPROCS;

	if (RANK == 0)
	{
		Array* array = instanciate_array(ARRAY_SIZE);
		fill_array_with_random_value(array, 0, 10);

		print_message_header();
		printf("Just generated the array : ");
		print_array(array);

		for (int i = 1; i < NUMPROCS; i++)
		{
			MPI_Send((void*) (array->array + i * SUBARRAYS_SIZE),
					 SUBARRAYS_SIZE,
					 MPI_INT,
					 i,
					 i,
					 MPI_COMM_WORLD);

			print_message_header();
			printf("Just sent the following array to process %d : ", i);
			print_array_part(array, i * SUBARRAYS_SIZE, SUBARRAYS_SIZE);
		}
	}

	else
	{
		Array* received_values = instanciate_array(SUBARRAYS_SIZE);
		MPI_Status* status = malloc(sizeof(MPI_Status));

		MPI_Recv((void*) received_values->array,
				 SUBARRAYS_SIZE,
				 MPI_INT,
				 0,
				 RANK,
				 MPI_COMM_WORLD,
				 status);

		print_message_header();
		printf("just received the following array from thread %d : ", 0);
		print_array(received_values);
	}

	MPI_Finalize();
	return 0;
}

void scan_args(int argc, char** argv)
{
	if (argc != 2)
	{
		if (RANK == 0)
		{
			printf("usage: %s <array size>\n", *argv);
		}
		exit(1);
	}

	int user_size = atoi(*(argv + 1));

	if (user_size > 0)
	{
		if (user_size % NUMPROCS != 0)
		{
			ARRAY_SIZE = user_size - (user_size % NUMPROCS);
			if (RANK == 0)
			{
				printf("Warning: given array size (%d) is not a multiple of the amount of threads (%d), rounding it to %d\n", user_size, NUMPROCS, ARRAY_SIZE);
			}
		}

		else
		{
			ARRAY_SIZE = user_size;
			if (RANK == 0)
			{
				printf("Array size defined to %d\n", ARRAY_SIZE);
			}
		}
	}

	else
	{
		if (RANK == 0)
		{
			printf("Invalid array size; using default array size which is %d\n", ARRAY_SIZE);
		}
	}
}

void print_message_header()
{
	printf("[process n°%d] ", RANK);
}

Array* instanciate_array(int size)
{
	Array* array = (Array*) malloc(sizeof(Array));
	array->array = (int*) malloc(sizeof(int) * size);
	array->size = size;
}

void free_array(Array* array)
{
	free(array->array);
	free(array);
}

void fill_array_with_random_value(Array* array, int lower_bound, int upper_bound)
{
	for(int i = 0; i < array->size; i++)
	{
		*(array->array + i) = (rand() % (upper_bound - lower_bound)) + lower_bound;
	}
}

void fill_array_with_given_values(Array* array, int* src)
{
	for (int i = 0; i < array->size; i++)
	{
		if (i >= array->size)
		{
			print_message_header();
			printf("Warning : attempting to read out of bounds of an array.\n");
			break;
		}

		*(array->array + i) = *(src + i);
	}
}

void print_array(Array* array)
{
	for (int i = 0; i < array->size; i++)
	{
		printf("%d ", *(array->array + i));
	}

	printf("\n");
}

void print_array_part(Array* array, int offset, int size)
{
	int i = offset;

	while (i < offset + size && i < array->size)
	{
		printf("%d ", *(array->array + i));
		i++;
	}

	printf("\n");
}
