#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <string.h>

//structura care retine topologia
typedef struct topology {
    int *cluster0;
    int *cluster1;
    int *cluster2;
    int size_cluster0;
    int size_cluster1;
    int size_cluster2;
    int coordinator;
} topology;

int read_inp (int rank, int* &workers);
std::string make_topology(topology topology);