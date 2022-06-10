/*
 * HYPRE Utilities
 * Fortran functions that call-through to C API functionality.
 *
 */

#include "mpi.h"
#include "_hypre_utilities.h"
#include "HYPRE.h"
#include "fortran-interfaces.h"

void
FortranCInterface_GLOBAL(nhwave_hypre_init,NHWAVE_HYPRE_INIT)()
{
    int                   rank, size;

    HYPRE_Int             ierr = 0;
   
    /* default execution policy and memory space */
    HYPRE_ExecutionPolicy default_exec_policy = HYPRE_EXEC_DEVICE;
    HYPRE_MemoryLocation  memory_location     = HYPRE_MEMORY_DEVICE;

#ifdef HYPRE_USING_DEVICE_POOL
    /* device pool allocator */
    hypre_uint            mempool_bin_growth   = 8,
                          mempool_min_bin      = 3,
                          mempool_max_bin      = 9;
    size_t                mempool_max_cached_bytes = 2000LL * 1024 * 1024;
#endif

#if defined(HYPRE_USING_GPU)
    HYPRE_Int             spgemm_use_vendor = 0;
    HYPRE_Int             spmv_use_vendor = 1;
    HYPRE_Int             use_curand = 1;
#if defined(HYPRE_USING_HIP) || defined(HYPRE_USING_SYCL)
    spgemm_use_vendor = 1;
#endif
    HYPRE_Int             spgemm_alg = 1;
    HYPRE_Int             spgemm_rowest_mtd = 3;
    HYPRE_Int             spgemm_rowest_nsamples = 32;
    HYPRE_Real            spgemm_rowest_mult = 1.5;
    char                  spgemm_hash_type = 'L';
#endif

    static int            argc = 1;
    static char*          argv[] = { "nhwave", NULL };

#if defined (PARALLEL)
    hypre_MPI_Comm_size(hypre_MPI_COMM_WORLD, &rank );
    hypre_MPI_Comm_rank(hypre_MPI_COMM_WORLD, &size );

# if defined (HYPRE_USING_GPU)
    hypre_bind_device(rank, size, hypre_MPI_COMM_WORLD);
# endif
#endif

    HYPRE_Init();

#ifdef HYPRE_USING_DEVICE_POOL
    /* To be effective, hypre_SetCubMemPoolSize must immediately follow HYPRE_Init */
    HYPRE_SetGPUMemoryPoolSize( mempool_bin_growth, mempool_min_bin,
                                mempool_max_bin, mempool_max_cached_bytes );
#endif

#if defined(HYPRE_USING_UMPIRE)
    /* Setup Umpire pools */
    HYPRE_SetUmpireDevicePoolName("HYPRE_DEVICE_POOL_NHWAVE");
    HYPRE_SetUmpireUMPoolName("HYPRE_UM_POOL_NHWAVE");
    HYPRE_SetUmpireHostPoolName("HYPRE_HOST_POOL_NHWAVE");
    HYPRE_SetUmpirePinnedPoolName("HYPRE_PINNED_POOL_NHWAVE");
    HYPRE_SetUmpireDevicePoolSize(4LL * 1024 * 1024 * 1024);
    HYPRE_SetUmpireUMPoolSize(4LL * 1024 * 1024 * 1024);
    HYPRE_SetUmpireHostPoolSize(4LL * 1024 * 1024 * 1024);
    HYPRE_SetUmpirePinnedPoolSize(4LL * 1024 * 1024 * 1024);
#endif

    /* default memory location */
    HYPRE_SetMemoryLocation(memory_location);

    /* default execution policy */
    HYPRE_SetExecutionPolicy(default_exec_policy);

#if defined(HYPRE_USING_GPU)
   ierr = HYPRE_SetSpMVUseVendor(spmv_use_vendor); hypre_assert(ierr == 0);
   /* use vendor implementation for SpGEMM */
   ierr = HYPRE_SetSpGemmUseVendor(spgemm_use_vendor); hypre_assert(ierr == 0);
   ierr = hypre_SetSpGemmAlgorithm(spgemm_alg); hypre_assert(ierr == 0);
   ierr = hypre_SetSpGemmRownnzEstimateMethod(spgemm_rowest_mtd); hypre_assert(ierr == 0);
   ierr = hypre_SetSpGemmRownnzEstimateNSamples(spgemm_rowest_nsamples); hypre_assert(ierr == 0);
   ierr = hypre_SetSpGemmRownnzEstimateMultFactor(spgemm_rowest_mult); hypre_assert(ierr == 0);
   ierr = hypre_SetSpGemmHashType(spgemm_hash_type); hypre_assert(ierr == 0);
   /* use cuRand for PMIS */
   HYPRE_SetUseGpuRand(use_curand);
#endif
}

