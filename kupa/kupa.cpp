
#include <mpi.h>
#include <iostream>
#include <math.h>
#include <time.h>     
#include <windows.h> 
#include <stdlib.h>    
#include <array>
#include <numeric>
#include <string.h>
#include <cstdlib>

#define PRODUCERS_NBR    0
#define BUFFERS         1
#define IN_QUEUE        100
#define PRODUCT_NBRS   1000

using namespace std;
// ---------------------------------------------------------------------

void producer()
{

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    for (unsigned int i = 0; i < PRODUCT_NBRS; i++)
    {
        int  value[100000];

        for (int j = 0; j < 100000; j++)
            {
            value[j] = rand() % 10 + 1;
            }


        Sleep(20);

        MPI_Ssend(&value, 100000, MPI_INT, BUFFERS, 1, MPI_COMM_WORLD);
    }
}
// ---------------------------------------------------------------------

void buffer()
{
    int info_s,
        pos = 0,
        role;
    int  value[100000];
    MPI_Status status;

    for (unsigned int i = 0; i < PRODUCT_NBRS*2; i++)
    {
        int num;

        if (pos == 0)         
        {
            role = 0;
            MPI_Probe(MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
            num = status.MPI_SOURCE;
        }

        else if (pos == IN_QUEUE)   
        {
            role = 1;
            MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            num = status.MPI_SOURCE;
        }
        else                 
        {

            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);


            num = status.MPI_SOURCE;

            if (num <= PRODUCERS_NBR)
                role = 0;
            else
                role = 1;
        }
        switch (role)
        {
        case 0:
            MPI_Recv(&value, 100000, MPI_INT, num, 1, MPI_COMM_WORLD, &status);

            pos++;
            break;

        case 1:
            MPI_Recv(&info_s, 1, MPI_INT, num, 0, MPI_COMM_WORLD, &status);
            MPI_Ssend(&value, 100000, MPI_INT, num, 2, MPI_COMM_WORLD);
            //cout << "Buffer send " << value [0] << " to consumer " << num << endl ;
            pos--;
            break;
        }
    }
            
    
}

// ---------------------------------------------------------------------

void consumer()
{
    int  value[100000];
    std::array<int, 100000> values;
    int    if_ready = 1;
    MPI_Status  status;
    int consumed_n = 0;
    

    for (unsigned int i = 0; i < PRODUCT_NBRS; i++)
    {
        MPI_Ssend(&if_ready, 1, MPI_INT, BUFFERS, 0, MPI_COMM_WORLD);
        MPI_Recv(&value, 100000, MPI_INT, BUFFERS, 2, MPI_COMM_WORLD, &status);

        
        int sum = 0;
        for (int j = 0; j < 100000; j++)
        {
            sum  = sum + value[j];
            values[j] = value[j];
        }
        std::sort(values.begin(), values.end());

        int rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);

        consumed_n += 1;
        //cout << "Consumer sum " << sum << endl;
        cout << "Consumer:  " << rank << "  consumed " <<   consumed_n << endl;
        Sleep(20);


    }
}
// ---------------------------------------------------------------------

int main(int argc, char* argv[])
{
    int rank, size;


    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);


    double t1 = MPI_Wtime();
    if (size < 1)
    {
        cout << "BAD NUMBER OF PROCESSES " << endl;
        return 0;
    }


    if (rank <= PRODUCERS_NBR)
        producer();
    else if (rank == BUFFERS)
        buffer();
    else
        consumer();

    double t2 = MPI_Wtime();
    Sleep(500);
    cout << "Time : \n" << (t2 - t1) << endl;
    MPI_Finalize();
    return 0;

}
