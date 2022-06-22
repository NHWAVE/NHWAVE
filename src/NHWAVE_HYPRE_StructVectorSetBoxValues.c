/*
 * Fortran interface to the unexposed HYPRE GPU device-binding function.
 */

#include "mpi.h"
#include "_hypre_utilities.h"
#include "HYPRE.h"
#include "fortran.h"
#include "fortran-interfaces.h"
#include <time.h>

void
FortranCInterface_GLOBAL(nhwave_hypre_structvectorsetboxvalues,NHWAVE_HYPRE_STRUCTVECTORSETBOXVALUES)(
    hypre_F90_Obj *vector,
    hypre_F90_IntArray *ilower,
    hypre_F90_IntArray *iupper,
    hypre_F90_Int *num_values,
    hypre_F90_ComplexArray *values,
    hypre_F90_Int *ierr
)
{
    time_t          t0 = time(NULL), t1;
    fprintf(stderr, "[INFO][%16llu][%16s] IN %s\n", (unsigned long long)t0, "", __func__); fflush(stderr);
#if defined (HYPRE_CUDA)
    hypre_F90_ComplexArray  *VALUES = hypre_CTAlloc(HYPRE_Complex,  *num_values, HYPRE_MEMORY_DEVICE);
   
    hypre_Memcpy(VALUES, values, sizeof(HYPRE_Complex) * *num_values, HYPRE_MEMORY_DEVICE, HYPRE_MEMORY_HOST);
    FortranCInterface_GLOBAL(hypre_structvectorsetboxvalues,HYPRE_STRUCTVECTORSETBOXVALUES)(
            vector, ilower, iupper, VALUES, ierr
        );
    hypre_Memcpy(values, VALUES, sizeof(HYPRE_Complex) * *num_values, HYPRE_MEMORY_HOST, HYPRE_MEMORY_DEVICE);
    hypre_TFree(VALUES, HYPRE_MEMORY_DEVICE);
#else
    FortranCInterface_GLOBAL(hypre_structvectorsetboxvalues,HYPRE_STRUCTVECTORSETBOXVALUES)(
            vector, ilower, iupper, values, ierr
        );
#endif
    t1 = time(NULL);
    fprintf(stderr, "[INFO][%16llu][%16llu] OUT %s\n", (unsigned long long)t1, (unsigned long long)(t1 - t0), __func__); fflush(stderr);
}
