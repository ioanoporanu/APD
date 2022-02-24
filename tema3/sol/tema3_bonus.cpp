#include "tema3.h"

using namespace std;

//functie care citeste inputul pentru un cluster
int read_inp (int rank, int* &workers) {
    string filename = "cluster" + to_string(rank) + ".txt"; 
    ifstream fin;
    int nr_of_workers, worker;
    fin.open(filename);
    fin >> nr_of_workers;
    workers = (int*)calloc(nr_of_workers, sizeof(int));
    for(int i = 0; i < nr_of_workers; i++){
        fin >> worker;
        workers[i] = worker;
    }
    fin.close();
    return nr_of_workers;
}

//functie care formeaza string-ul pentru topologie
string make_topology(topology topology) {
    string result = "";

    result += "0:"; 
    for(int i = 0; i < topology.size_cluster0 - 1; i++){
        result += to_string(topology.cluster0[i]).c_str();
        result += ",";
    }
    result += to_string(topology.cluster0[topology.size_cluster0 - 1]).c_str();

    result += " 1:";
    for(int i = 0; i < topology.size_cluster1 - 1; i++){
        result += to_string(topology.cluster1[i]).c_str();
        result += ",";
    }
    result += to_string(topology.cluster1[topology.size_cluster1 - 1]).c_str();

    result += " 2:";
    for(int i = 0; i < topology.size_cluster2 - 1; i++){
        result += to_string(topology.cluster2[i]).c_str();
        result += ",";
    }
    result += to_string(topology.cluster2[topology.size_cluster2 - 1]);

    return result;
}


int main (int argc, char *argv[])
{
    MPI_Status status;
    string s;
    int old_rank, old_size, topology_string_size, frag_size, data_size;
    char* topology_string;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &old_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &old_rank);
    int N;
    int *Vector;
    topology topology;
    if (old_rank == 0) {

        //se citeste inputul pentru clusterul 0
        topology.size_cluster0 = read_inp(old_rank, topology.cluster0);

        //se trimite nr de workeri din acest cluster catre cluster-ul 2
        MPI_Send(&topology.size_cluster0, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        cout << "M(0,2)" << endl; 

        //se primeste de la clusterul 2 numarul de workeri pentru cluster-ul 2 si 1
        MPI_Recv(&topology.size_cluster2, 1, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&topology.size_cluster1, 1, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //se aloca memorie pentru vectorii de workeri pentur celelalte 2 clustere
        topology.cluster1 = (int*)calloc(topology.size_cluster1, sizeof(int));
        topology.cluster2 = (int*)calloc(topology.size_cluster2, sizeof(int));

        //se trimite vectorul cu workerii catre clusterul 2
        MPI_Send(topology.cluster0, topology.size_cluster0, MPI_INT, 2, 0, MPI_COMM_WORLD);
        cout << "M(0,2)" << endl; 

        //se primesc vectorii cu workerii pentru celelalte 2 clustere
        MPI_Recv(topology.cluster2, topology.size_cluster2, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(topology.cluster1, topology.size_cluster1, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //se creeaza string-ul pentru topologie si se afiseaza
        topology_string_size = 4 * (topology.size_cluster0 + topology.size_cluster1 + topology.size_cluster2);
        topology_string = (char*)calloc(topology_string_size, sizeof(char));
        s = make_topology(topology);
        topology_string = (char*)s.c_str();
        cout << old_rank << " -> " << topology_string << endl;

        //se trimite dimensiunea topologiei si topologia catre workeri 
        for(int i = 0; i < topology.size_cluster0; i++){
            MPI_Send(&topology_string_size, 1, MPI_INT, topology.cluster0[i], 0, MPI_COMM_WORLD);
            cout << "M(0," << topology.cluster0[i] << ")" << endl; 
            MPI_Send(topology_string, topology_string_size, MPI_CHAR, topology.cluster0[i], 0, MPI_COMM_WORLD);
            cout << "M(0," << topology.cluster0[i] << ")" << endl; 
        }

        //se creeza vectorul care trebuie multiplicat
        N = atoi(argv[1]);
        int nr_of_workers = topology.size_cluster0 + topology.size_cluster1 + topology.size_cluster2;
        if(N % nr_of_workers != 0){
            Vector = (int*)calloc(N + nr_of_workers - N % nr_of_workers, sizeof(int));
            frag_size = (N + nr_of_workers - N % nr_of_workers) / nr_of_workers;
        } else {
            Vector = (int*)calloc(N, sizeof(int));
            frag_size = N / nr_of_workers;
        }
        for ( int i = 0; i < N; i++) {
            Vector [i] = i;
        }
        
        //se trimit catre coordonatorul 2 dimensiunea si vectorul ce trebuie prelucrat de clusterul 1
        data_size = frag_size * topology.size_cluster1;
        MPI_Send(&data_size, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        cout << "M(0,2)" << endl;
        MPI_Send(Vector + topology.size_cluster0 * frag_size, data_size, MPI_INT, 2, 0, MPI_COMM_WORLD);
        cout << "M(0,2)" << endl;
        
        //se trimit catre coordonatorul 2 dimensiunea si vectorul ce trebuie prelucrat de clusterul 2
        data_size = frag_size * topology.size_cluster2;
        MPI_Send(&data_size, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        cout << "M(0,2)" << endl;
        MPI_Send(Vector + (topology.size_cluster0 + topology.size_cluster1) * frag_size, data_size, MPI_INT, 2, 0, MPI_COMM_WORLD);
        cout << "M(0,2)" << endl;

        //se trimit catre workeri dimensiunea fragmentelor ce trebuie prelucrate si fragmentele 
        for(int i = 0; i < topology.size_cluster0; i++){
            MPI_Send(&frag_size, 1, MPI_INT, topology.cluster0[i], 0, MPI_COMM_WORLD);
            cout << "M(0," << topology.cluster0[i] << ")" << endl; 
            MPI_Send(Vector + i * frag_size, frag_size, MPI_INT, topology.cluster0[i], 0, MPI_COMM_WORLD);
            cout << "M(0," << topology.cluster0[i] << ")" << endl; 
        }

        //se primesc de la workeri fragmentele prelucrate
        for(int i = 0; i < topology.size_cluster0; i++){
           MPI_Recv(Vector + i * frag_size, frag_size, MPI_INT, topology.cluster0[i], topology.cluster0[i], MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        
        //se primeste vectorul prelucrate de coordonatorul 2 de la coordonatorul 2
        data_size = frag_size * topology.size_cluster2;
        MPI_Recv(Vector + (topology.size_cluster0 + topology.size_cluster1) * frag_size, data_size, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        //se primeste vectorul prelucrate de coordonatorul 1 de la coordonatorul 2
        data_size = frag_size * topology.size_cluster1;
        MPI_Recv(Vector + topology.size_cluster0 * frag_size, data_size, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        //se afiseaza vectorul prelucrat
        cout << "Rezultat: ";
        for(int i = 0; i < N; i++){
            cout << Vector[i] << " ";
        }
        cout << endl;
    } else if (old_rank == 1) {
        
        //se citeste inputul pentru clusterul 1
        topology.size_cluster1 = read_inp(old_rank, topology.cluster1);

        //se trimite dimensiunea pentru acest cluster clusterului 2
        MPI_Send(&topology.size_cluster1, 1, MPI_INT, 2, 1, MPI_COMM_WORLD);
        cout << "M(1,2)" << endl; 
       
        //se primesc dimensiunile pentru clusterul 2 si 0 de la clusterul 2
        MPI_Recv(&topology.size_cluster2, 1, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&topology.size_cluster0, 1, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //se aloca vectorii pentru clusterele 0 si 2
        topology.cluster0 = (int*)calloc(topology.size_cluster0, sizeof(int));
        topology.cluster2 = (int*)calloc(topology.size_cluster2, sizeof(int));

        //se trimite dimensiunea acestui cluster, clusterului 2
        MPI_Send(topology.cluster1, topology.size_cluster1, MPI_INT, 2, 1, MPI_COMM_WORLD);
        cout << "M(1,2)" << endl; 

        //se primesc de la clusterul 2 vectorii cu workeri pentru clusterele 0 si 1
        MPI_Recv(topology.cluster2, topology.size_cluster2, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(topology.cluster0, topology.size_cluster0, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //se creeaza si se afiseaza string-ul pentru topologie
        topology_string_size = 4 * (topology.size_cluster0 + topology.size_cluster1 + topology.size_cluster2);
        topology_string = (char*)calloc(topology_string_size, sizeof(char));
        s = make_topology(topology);
        topology_string = (char*)s.c_str();
        cout << old_rank << " -> " << topology_string << endl;

        //se trimite workeri-lor topologia
        for(int i = 0; i < topology.size_cluster1; i++){
            MPI_Send(&topology_string_size, 1, MPI_INT, topology.cluster1[i], 1, MPI_COMM_WORLD);
            cout << "M(1," << topology.cluster1[i] << ")" << endl; 
            MPI_Send(topology_string, topology_string_size, MPI_CHAR, topology.cluster1[i], 1, MPI_COMM_WORLD);
            cout << "M(1," << topology.cluster1[i] << ")" << endl; 
        }

        //se primeste de la clusterul 2 vectorul ce trebuie prelucrat de clusterul 1 si dimensiunea acestuia
        MPI_Recv(&N, 1, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        Vector = (int*)calloc(N, sizeof(int));
        MPI_Recv(Vector, N, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //se trimit workeri-lor fragmentele pe care acestia trebuie sa le prelucreze
        frag_size = N / topology.size_cluster1;

        for(int i = 0; i < topology.size_cluster1; i++){
            MPI_Send(&frag_size, 1, MPI_INT, topology.cluster1[i], 1, MPI_COMM_WORLD);
            cout << "M(1," << topology.cluster1[i] << ")" << endl; 
            MPI_Send(Vector + i * frag_size, frag_size, MPI_INT, topology.cluster1[i], 1, MPI_COMM_WORLD);
            cout << "M(1," << topology.cluster1[i] << ")" << endl; 
        }

        //se primesc de la workeri fragmentele prelucrate
        for(int i = 0; i < topology.size_cluster1; i++){
           MPI_Recv(Vector + i * frag_size, frag_size, MPI_INT, topology.cluster1[i], topology.cluster1[i], MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        //se trimite clusterului 2 dimensiunea si vectorul prelucrat
        MPI_Send(&N, 1, MPI_INT, 2, 1, MPI_COMM_WORLD);
        cout << "M(1,2)" << endl; 
        MPI_Send(Vector, N, MPI_INT, 2, 1, MPI_COMM_WORLD);
        cout << "M(1,2)" << endl; 

    } else if (old_rank == 2) {
        
        //se citeste inputul pentru clusterul 2
        topology.size_cluster2 = read_inp(old_rank, topology.cluster2);

        //se trimite celorlalte 2 clustere dimensiunea clusterului 2
        MPI_Send(&topology.size_cluster2, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        cout << "M(2,0)" << endl; 
        MPI_Send(&topology.size_cluster2, 1, MPI_INT, 1, 2, MPI_COMM_WORLD);
        cout << "M(2,1)" << endl; 

        //se primesc de la cele 2 cluestere dimensiunile acestora
        MPI_Recv(&topology.size_cluster0, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&topology.size_cluster1, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //se aloca vectorii pentru celelalte 2 clustere
        topology.cluster0 = (int*)calloc(topology.size_cluster0, sizeof(int));
        topology.cluster1 = (int*)calloc(topology.size_cluster1, sizeof(int));

        //se trimite clusterului 0 dimensiunea clusterului 1 si invers
        MPI_Send(&topology.size_cluster1, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        cout << "M(2,0)" << endl; 
        MPI_Send(&topology.size_cluster0, 1, MPI_INT, 1, 2, MPI_COMM_WORLD);
        cout << "M(2,1)" << endl; 

        //se trimite clusterului 0 si clusterului 1 vectorul cu workeri ai clusterului 2 
        MPI_Send(topology.cluster2, topology.size_cluster2, MPI_INT, 0, 2, MPI_COMM_WORLD);
        cout << "M(2,0)" << endl; 
        MPI_Send(topology.cluster2, topology.size_cluster2, MPI_INT, 1, 2, MPI_COMM_WORLD);
        cout << "M(2,1)" << endl; 
        
        //se primesc vectorii cu workerii de la celalte 2 clustere
        MPI_Recv(topology.cluster0, topology.size_cluster0, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(topology.cluster1, topology.size_cluster1, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //se trimite clusterului 1 vectorul cu workeri ai clusterului 0 si invers
        MPI_Send(topology.cluster1, topology.size_cluster1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        cout << "M(2,0)" << endl; 
        MPI_Send(topology.cluster0, topology.size_cluster0, MPI_INT, 1, 2, MPI_COMM_WORLD);
        cout << "M(2,1)" << endl; 

        //se creeaza string-ul pentru topologie si se afiseaza
        topology_string_size = 4 * (topology.size_cluster0 + topology.size_cluster1 + topology.size_cluster2);
        topology_string = (char*)calloc(topology_string_size, sizeof(char));
        s = make_topology(topology);
        topology_string = (char*)s.c_str();
        cout << old_rank << " -> " << topology_string << endl;

        //se trimite workerilor stringul cu topologia 
        for(int i = 0; i < topology.size_cluster2; i++){
            MPI_Send(&topology_string_size, 1, MPI_INT, topology.cluster2[i], 2, MPI_COMM_WORLD);
            cout << "M(2," << topology.cluster2[i] << ")" << endl; 
            MPI_Send(topology_string, topology_string_size, MPI_CHAR, topology.cluster2[i], 2, MPI_COMM_WORLD);
            cout << "M(2," << topology.cluster2[i] << ")" << endl; 
        }

        //se primeste vectorul ce trebuie prelucrat de clusterul 1 de la clusterul 0
        MPI_Recv(&N, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        Vector = (int*)calloc(N, sizeof(int));
        MPI_Recv(Vector, N, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //se trimite vectorul primit anterior clusterului 1
        MPI_Send(&N, 1, MPI_INT, 1, 2, MPI_COMM_WORLD);
        cout << "M(2,1)" << endl; 
        MPI_Send(Vector, N, MPI_INT, 1, 2, MPI_COMM_WORLD);
        cout << "M(2,1)" << endl; 

        //se primeste de la clusterul 0 vectorul ce trebuie prelucrat de clusterul 2
        MPI_Recv(&N, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        free(Vector);
        Vector = (int*)calloc(N, sizeof(int));
        MPI_Recv(Vector, N, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //se trimit fragmentele vectorului ce trebuie prelucrat workerilor 
        frag_size = N / topology.size_cluster2;

        for(int i = 0; i < topology.size_cluster2; i++){
            MPI_Send(&frag_size, 1, MPI_INT, topology.cluster2[i], 1, MPI_COMM_WORLD);
            cout << "M(2," << topology.cluster2[i] << ")" << endl; 
            MPI_Send(Vector + i * frag_size, frag_size, MPI_INT, topology.cluster2[i], 1, MPI_COMM_WORLD);
            cout << "M(2," << topology.cluster2[i] << ")" << endl; 
        }

        //se primesc inapoi fragmentele prelucrate
        for(int i = 0; i < topology.size_cluster2; i++){
           MPI_Recv(Vector + i * frag_size, frag_size, MPI_INT, topology.cluster2[i], topology.cluster2[i], MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        //se trimite clusterului 0 vectorul prelucrat oentru clusterul 2
        MPI_Send(Vector, N, MPI_INT, 0, 2, MPI_COMM_WORLD);
        cout << "M(2,0)" << endl; 
        free(Vector);
        
        //se primeste de la clusterul 1 vectorul prelucrate pentru clusterul 1
        MPI_Recv(&N, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        Vector = (int*)calloc(N, sizeof(int));
        MPI_Recv(Vector, N, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        //se trimite clusterului 0 vectorul prelucrat de clusterul 1
        MPI_Send(Vector, N, MPI_INT, 0, 2, MPI_COMM_WORLD);
        cout << "M(2,0)" << endl; 
    } else {
        //se primeste topologia de la coordonator
        MPI_Recv(&topology_string_size, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        topology_string = (char*)calloc(topology_string_size, sizeof(char));
        MPI_Recv(topology_string, topology_string_size, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cout << old_rank << " -> " << topology_string << endl;
        topology.coordinator = status.MPI_SOURCE;

        //se primeste fragmentul ce trebuie prelucrat
        MPI_Recv(&N, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        Vector = (int*)calloc(N, sizeof(int));
        MPI_Recv(Vector, N, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //se prelucreaza fragmentul
        for(int i = 0; i < N; i++){
            Vector[i] *= 2;
        }

        //se trimite inapoi fragmentul
        MPI_Send(Vector, N, MPI_INT, topology.coordinator, old_rank, MPI_COMM_WORLD); 
        cout << "M(" << old_rank << "," << topology.coordinator << ")" << endl; 
    }

     MPI_Finalize();
}
