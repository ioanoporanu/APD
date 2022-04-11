#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "genetic_algorithm_par.h"
#include "individual_par.h"

 #define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

//function that start threads that runs the algorithm
void start_threads(const sack_object *objects, int object_count, int generations_count, int sack_capacity, int P){
	//declare P threads
    pthread_t threads[P];
	//declare same barrier for all threads
	pthread_barrier_t barrier;
	pthread_barrier_init(&barrier, NULL, P);

	//declare same current and next generation for all threads
	individual* current_generation = (individual*) calloc(object_count, sizeof(individual));
	individual* next_generation = (individual*) calloc(object_count, sizeof(individual));

	for(int i = 0; i < P; i++){
		//composing the structure that is going to be passed to the current thread
		struct thread_input *data = calloc(1, sizeof(struct thread_input)); 
		data->object_count = object_count;
		data->generations_count = generations_count;
		data->P = P;
		data->thread_id = i;
		data->sack_capacity = sack_capacity;
		data->objects = objects;
		data->current_generation = current_generation;
	    data->next_generation = next_generation;
	    data->tmp = NULL;
		data->barrier = &barrier;
		
		//create current thread
		pthread_create(&threads[i], NULL, run_genetic_algorithm, data);
	}

	//start all threads
  	for (long int id = 0; id < P; id++) {
		pthread_join(threads[id], NULL);
		
  	}

	//destroy barrier
	pthread_barrier_destroy(&barrier);

	//free current and next generation
	free_generation(current_generation);
	free_generation(next_generation);
	free(current_generation);
	free(next_generation);

}

int read_input(sack_object **objects, int *object_count, int *sack_capacity, int *generations_count, int argc, char *argv[])
{
	FILE *fp;

	if (argc < 3) {
		fprintf(stderr, "Usage:\n\t./tema1 in_file generations_count\n");
		return 0;
	}

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		return 0;
	}

	if (fscanf(fp, "%d %d", object_count, sack_capacity) < 2) {
		fclose(fp);
		return 0;
	}

	if (*object_count % 10) {
		fclose(fp);
		return 0;
	}

	sack_object *tmp_objects = (sack_object *) calloc(*object_count, sizeof(sack_object));

	for (int i = 0; i < *object_count; ++i) {
		if (fscanf(fp, "%d %d", &tmp_objects[i].profit, &tmp_objects[i].weight) < 2) {
			free(objects);
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);

	*generations_count = (int) strtol(argv[2], NULL, 10);
	
	if (*generations_count == 0) {
		free(tmp_objects);

		return 0;
	}

	*objects = tmp_objects;

	return 1;
}

void print_objects(const sack_object *objects, int object_count)
{
	for (int i = 0; i < object_count; ++i) {
		printf("%d %d\n", objects[i].weight, objects[i].profit);
	}
}

void print_generation(const individual *generation, int limit)
{
	for (int i = 0; i < limit; ++i) {
		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			printf("%d ", generation[i].chromosomes[j]);
		}

		printf("\n%d - %d\n", i, generation[i].fitness);
	}
}

void print_best_fitness(const individual *generation)
{
	printf("%d\n", generation[0].fitness);
}

void compute_fitness_function(struct thread_input* data)
{
	int weight;
	int profit;
	int start, end;

	//compute start and end for the parallelized fitness function
	start = (double)data->object_count / data->P * data->thread_id;
	end = MIN((double)data->object_count / data->P * (data->thread_id + 1), data->object_count);

	//compute fitness but only for individuals between start and end
	for (int i = start; i < end; ++i) {
		weight = 0;
		profit = 0;
		data->current_generation[i].nr_of_ones = 0;
		for (int j = 0; j < data->current_generation[i].chromosome_length; ++j) {
			if (data->current_generation[i].chromosomes[j]) {
				weight += data->objects[j].weight;
				profit += data->objects[j].profit;
				//compute how many bites of one a chromosome has
				data->current_generation[i].nr_of_ones++;
			}
		}

		data->current_generation[i].fitness = (weight <= data->sack_capacity) ? profit : 0;
	}
}

int cmpfunc(const void *a, const void *b)
{
	individual *first = (individual *) a;
	individual *second = (individual *) b;

	//compare 2 individuals by number of bites of one they have
	int res = second->fitness - first->fitness; // decreasing by fitness
	if (res == 0) {
		res = first->nr_of_ones - second->nr_of_ones; // increasing by number of objects in the sack
		if (res == 0) {
			return second->index - first->index;
		}
	}

	return res;
}

void mutate_bit_string_1(const individual *ind, int generation_index)
{
	int i, mutation_size;
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	if (ind->index % 2 == 0) {
		// for even-indexed individuals, mutate the first 40% chromosomes by a given step
		mutation_size = ind->chromosome_length * 4 / 10;
		for (i = 0; i < mutation_size; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	} else {
		// for even-indexed individuals, mutate the last 80% chromosomes by a given step
		mutation_size = ind->chromosome_length * 8 / 10;
		for (i = ind->chromosome_length - mutation_size; i < ind->chromosome_length; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	}
}

void mutate_bit_string_2(const individual *ind, int generation_index)
{
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	// mutate all chromosomes by a given step
	for (int i = 0; i < ind->chromosome_length; i += step) {			
			ind->chromosomes[i] = 1 - ind->chromosomes[i];		
	}
}

void crossover(individual *parent1, individual *child1, int generation_index)
{
	individual *parent2 = parent1 + 1;
	individual *child2 = child1 + 1;
	int count = 1 + generation_index % parent1->chromosome_length;

	memcpy(child1->chromosomes, parent1->chromosomes, count * sizeof(int));
	memcpy(child1->chromosomes + count, parent2->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));

	memcpy(child2->chromosomes, parent2->chromosomes, count * sizeof(int));
	memcpy(child2->chromosomes + count, parent1->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));

}

void copy_individual(const individual *from, const individual *to)
{
	memcpy(to->chromosomes, from->chromosomes, from->chromosome_length * sizeof(int));
}

void free_generation(individual *generation)
{
	int i;

	for (i = 0; i < generation->chromosome_length; ++i) {
		free(generation[i].chromosomes);
		generation[i].chromosomes = NULL;
		generation[i].fitness = 0;
	}
}

//function that execute the genetic algorithm
void *run_genetic_algorithm(void *arg)
{
	int count, cursor;
	//cast input to thread_input structure
	struct thread_input* data = (struct thread_input*)arg;
	//declare start, end for the next loops that are going to be 
	//parallelized
	int start, end;
	
	//compute start and end for the loop that initialize the generation
	start = (double)data->object_count / data->P * data->thread_id;
	end = MIN((double)data->object_count / data->P * (data->thread_id + 1), data->object_count);
	pthread_barrier_wait(data->barrier);

	// set initial generation (composed of object_count individuals with a single item in the sack)
	for (int i = start; i < end; ++i) {
		data->current_generation[i].fitness = 0;
		data->current_generation[i].chromosomes = (int*) calloc(data->object_count, sizeof(int));
		data->current_generation[i].chromosomes[i] = 1;
		data->current_generation[i].index = i;
		data->current_generation[i].chromosome_length = data->object_count;

		data->next_generation[i].fitness = 0;
		data->next_generation[i].chromosomes = (int*) calloc(data->object_count, sizeof(int));
		data->next_generation[i].index = i;
		data->next_generation[i].chromosome_length = data->object_count;
	}

	//wait for all threads to finish the initialization
	pthread_barrier_wait(data->barrier);
	// iterate for each generation
	for (int k = 0; k < data->generations_count; ++k) {
		cursor = 0;
		// compute fitness and sort by it
		compute_fitness_function(data);
		// wait for all threads to compute fitness;
		pthread_barrier_wait(data->barrier);

		//sort only once (for thread 0) the chromosomes in the generation
		if (data->thread_id == 0){
			qsort(data->current_generation, data->object_count, sizeof(individual), cmpfunc);
		}
		//all threads must wait for thread 0 to sort the generation
		pthread_barrier_wait(data->barrier);
		//compute start and end for the next loop that copies the first 30% elite children
		count = data->object_count * 3 / 10;
		start = (double)count / data->P * data->thread_id;
		end = MIN((double)count / data->P * (data->thread_id + 1), count);

		// keep first 30% children (elite children selection)
		for (int i = start; i < end; ++i) {
			copy_individual(data->current_generation + i, data->next_generation + i);
		}

		cursor = count;
		//compute start and end for the first mutate
		count = data->object_count * 2 / 10;
		start = (double)count / data->P * data->thread_id;
		end = MIN((double)count / data->P * (data->thread_id + 1), count);

		// mutate first 20% children with the first version of bit string mutation
		for (int i = start; i < end; ++i) {
			copy_individual(data->current_generation + i, data->next_generation + cursor + i);
			mutate_bit_string_1(data->next_generation + cursor + i, k);
		}

		cursor += count;

		//compute start and end for the second mutate
		count = data->object_count * 2 / 10;
		start = (double)count / data->P * data->thread_id;
		end = MIN((double)count / data->P * (data->thread_id + 1), count);

		// mutate next 20% children with the second version of bit string mutation
		for (int i = start; i < end; ++i) {
			copy_individual(data->current_generation + i + count, data->next_generation + cursor + i);
			mutate_bit_string_2(data->next_generation + cursor + i, k);
		}

		cursor += count;

		// (if there is an odd number of parents, the last one is kept as such)
		count = data->object_count * 3 / 10;

		if (count % 2 == 1) {
			copy_individual(data->current_generation + data->object_count - 1, data->next_generation + cursor + count - 1);
			count--;
		}

		//compute start and end for crossover
		start = (double)(count / data->P) * data->thread_id;
		end = MIN((double)count / data->P * (data->thread_id + 1), count - 1);

		// crossover first 30% parents with one-point crossover
		for (int i = start; i < end; i += 2) {
			crossover(data->current_generation + i, data->next_generation + cursor + i, k);
		}

		//wait for all threads to finish crossover
		pthread_barrier_wait(data->barrier);

		// switch to new generation
		data->tmp = data->current_generation;
		data->current_generation = data->next_generation;
		data->next_generation = data->tmp;

		//wait for all threads to switch generations
		pthread_barrier_wait(data->barrier);

		//compute start and end for for initialization of the  current generation
		start = (double)data->object_count / data->P * data->thread_id;
		end = MIN((double)data->object_count / data->P * (data->thread_id + 1), data->object_count);

		//initialize current generation only in one thread
		if(data->thread_id == 0){
			for (int i = start; i < end; ++i) {
				data->current_generation[i].index = i;
			}
		}

		//all threads must wait for thread 0 to initialize current generation
		pthread_barrier_wait(data->barrier);

		//print best fitness
		if (k % 5 == 0 && data->thread_id == 0) {
			print_best_fitness(data->current_generation);
		}

		pthread_barrier_wait(data->barrier);
	}
	
	//compute best fitness and print it
	compute_fitness_function(data);
	pthread_barrier_wait(data->barrier);

	if(data->thread_id == 0){
		qsort(data->current_generation, data->object_count, sizeof(individual), cmpfunc);
	}

	if (data->thread_id == 0) {
			print_best_fitness(data->current_generation);
	}

	//exit threads
	pthread_exit(NULL);
	return NULL;
}
