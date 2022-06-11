/*
 * Fortran interface to the unexposed HYPRE GPU device-binding function.
 */

#include "mpi.h"
#include "_hypre_utilities.h"
#include "HYPRE.h"
#include "fortran-interfaces.h"

#if defined (HYPRE_CUDA)
void
FortranCInterface_GLOBAL(hypre_bind_device,HYPRE_BIND_DEVICE)()
{
    int    rank, size;

    hypre_MPI_Comm_size(hypre_MPI_COMM_WORLD, &rank );
    hypre_MPI_Comm_rank(hypre_MPI_COMM_WORLD, &size );
    hypre_bind_device(rank, size, hypre_MPI_COMM_WORLD);
}
#endif

