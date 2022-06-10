# NHWAVE Build and Installation

Classically, the NHWAVE software is built from a templated Makefile, several examples of which reside in the [src](./src) directory (as well as another in the [extras](./extras) directory).  One of these Makefile files can be copied and modified to suit any site's needs.  The `nhwave` executable is then build by entering the [src](./src) directory and issuing the command

```
$ cd src
$ make -f Makefile.my_site.mpif90
```

If successful, an `nhwave` executable should be present in the [src](./src) directory.


## Options

There exists a numbered series of optional build features/properties for NHWAVE at the top of the Makefile.  These options control single- or double-precision numerics, MPI-parallelism, etc.  Each is enabled by uncommenting its line in the Makefile.


## CMake-guided Builds

The [CMake](https://cmake.org/) build system generator can also be used to build NHWAVE.  Version 3.1.2 or newer is required.

The build features/properties options exist as boolean options in the CMake system.  They are easily configured using command-line flags to the `cmake` command or visually using the `ccmake` interface:

```
 BALANCE2D                        OFF
 BUBBLE                           OFF
 CORALREEF                        OFF
 COUPLING                         OFF
 DEFORMABLESLIDE                  OFF
 DOUBLE_PRECISION                 ON
 FLUIDSLIDE                       OFF
 FROUDE_CAP                       ON
 INTEL                            ON
 LANDSLIDE                        OFF
 LANDSLIDE_COMPREHENSIVE          OFF
 OBSTACLE                         OFF
 PARALLEL                         ON
 POROUSMEDIA                      OFF
 SALINITY                         OFF
 SEDIMENT                         OFF
 TEMPERATURE                      OFF
 TWOLAYERSLIDE                    OFF
 VEGETATION                       OFF
```

All options default to OFF — except the `INTEL` option, which defaults to ON if the Fortran compiler is determined to be Intel-branded.  Enabling the `SEDIMENT` feature from the `cmake` command is accomplished by adding the flag `-DSEDIMENT=On`.


### Example CMake Build

The following example was performed on the University of Delaware's Caviness cluster.

After entering the appropriate workgroup, a distinct version of the NHWAVE software is designated and a directory created for it (I'll use the current date as a version number):

```
$ mkdir -p ${WORKDIR}/sw/nhwave/20220609
$ cd ${WORKDIR}/sw/nhwave/20220609
$ git clone https://github.com/jtfrey/NHWAVE.git src
```

Next, ensure a newer version of CMake is present in the shell environment, as well as the workgroup's build of the HYPRE library (for CUDA and MPI).  For HYPRE, additional development-oriented variables are desired:

```
$ vpkg_require cmake/default
Adding package `cmake/3.21.4` to your environment

$ vpkg_devrequire hypre/2.24.0:cuda,openmpi
Adding dependency `libfabric/1.13.2` to your environment
Adding dependency `intel-oneapi/2022.1.1.97` to your environment
Adding dependency `openmpi/4.1.2:intel-oneapi` to your environment
Adding dependency `cuda/11.6.2-510.47.03` to your environment
Adding package `hypre/2.24.0:openmpi,cuda` to your environment
```

One of the benefits of CMake is that out-of-tree builds are easily supported.  So we will enter the source code subdirectory and create a build directory, again named by date:

```
$ cd ${WORKDIR}/sw/nhwave/20220609/src/src
$ mkdir build-20220609
$ cd build-20220609
```

Next, CMake will be used to configure the build:

```
$ FC=mpifort cmake \
    -DCMAKE_INSTALL_PREFIX="${WORKDIR}/sw/nhwave/20220609" \
    -DCMAKE_BUILD_TYPE=Release \
    -DHYPRE_DIR="$HYPRE_PREFIX" \
    -DPARALLEL=On -DMPI_HOME="$OPENMPI_PREFIX" \
    -DFROUDE_CAP=On \
    -DDOUBLE_PRECISION=On \
    ..
```

The `$HYPRE_PREFIX` and `$OPENMPI_PREFIX` are set by VALET when using the `vpkg_devrequire` command to add software to the shell environment; here, they make it very easy to point CMake to the dependencies.  Per the settings for [the Farber cluster](src/Makefile.farber.mpif90), the `FROUDE_CAP` feature is enabled in addition to MPI parallelism (`PARALLEL`) and double-precision floating-point accuracy.  The successful creation of the build system looked like this:

```
-- The Fortran compiler identification is Intel 2021.5.0.20211109
-- Detecting Fortran compiler ABI info
-- Detecting Fortran compiler ABI info - done
-- Check for working Fortran compiler: /opt/shared/openmpi/4.1.2-intel-oneapi/bin/mpifort - skipped
-- Checking whether /opt/shared/openmpi/4.1.2-intel-oneapi/bin/mpifort supports Fortran 90
-- Checking whether /opt/shared/openmpi/4.1.2-intel-oneapi/bin/mpifort supports Fortran 90 - yes
-- Found HYPRE: /work/kirby/sw/hypre/2.24.0-cuda-openmpi/lib/libHYPRE.so  
-- Found MPI_Fortran: /opt/shared/openmpi/4.1.2-intel-oneapi/bin/mpifort (found version "3.1") 
-- Found MPI: TRUE (found version "3.1")  
-- Configuring done
-- Generating done
-- Build files have been written to: /work/kirby/sw/nhwave/20220609/src/src/build-20220609
```

The various options can be modified easily (and visually) using the `ccmake` command from this same `build-20220609` directory:

```
$ ccmake ..
```

Options can be enabled/disabled with a press of the spacebar key, build files generated by pressing the "C" key followed by the "G" key.

The `nhwave` executable is built using `make`:

```
$ make -j 4
Scanning dependencies of target nhwave
[  9%] Building Fortran object CMakeFiles/nhwave.dir/mod_global.F.o
[ 27%] Building Fortran object CMakeFiles/nhwave.dir/two_layer_slide.F.o
[ 27%] Building Fortran object CMakeFiles/nhwave.dir/fluid_slide.F.o
[ 36%] Building Fortran object CMakeFiles/nhwave.dir/mod_util.F.o
[ 45%] Building Fortran object CMakeFiles/nhwave.dir/deformable_slide.F.o
[ 54%] Building Fortran object CMakeFiles/nhwave.dir/source_terms.F.o
[ 63%] Building Fortran object CMakeFiles/nhwave.dir/waves.F.o
[ 72%] Building Fortran object CMakeFiles/nhwave.dir/pressure.F.o
[ 81%] Building Fortran object CMakeFiles/nhwave.dir/initialize.F.o
[ 90%] Building Fortran object CMakeFiles/nhwave.dir/nhwave.F.o
[100%] Linking Fortran executable nhwave
[100%] Built target nhwave
```

The completed executable can be installed for easy integration in the VALET package management system:

```
$ make install
[100%] Built target nhwave
Install the project...
-- Install configuration: "Release"
-- Installing: /work/kirby/sw/nhwave/20220609/bin/nhwave
```

### VALET Configuration

The workgroup lacked a VALET package definition for NHWAVE, so a new file was added at `${WORKDIR}/sw/valet/nhwave.vpkg_yaml`:

```
nhwave:
    prefix: /work/kirby/sw/nhwave
    description: Nonhydrostatic WAVE dynamics
    url: "https://github.com/NHWAVE/NHWAVE"

    versions:
        "20220609":
            description: Open MPI, Intel Fortran, HYPRE for CUDA
            dependencies:
                - hypre/2.24.0:cuda,openmpi

```

Henceforth, when this version of NHWAVE is to be used (either from an interactive shell or a job script):

```
$ vpkg_require nhwave/20220609
Adding dependency `libfabric/1.13.2` to your environment
Adding dependency `intel-oneapi/2022.1.1.97` to your environment
Adding dependency `openmpi/4.1.2:intel-oneapi` to your environment
Adding dependency `cuda/11.6.2-510.47.03` to your environment
Adding dependency `hypre/2.24.0:openmpi,cuda` to your environment
Adding package `nhwave/20220609` to your environment

$ which nhwave
/work/kirby/sw/nhwave/20220609/bin/nhwave
```

The bare command `nhwave` equates with the version of NHWAVE that was compiled and installed above:

```
$ cd $WORKDIR/sw/nhwave/20220609/examples/submerged_bar
$ mpirun -np 5 nhwave
CUDA ERROR (code = 35, CUDA driver version is insufficient for CUDA runtime version) at memory.c:298
CUDA ERROR (code = 35, CUDA driver version is insufficient for CUDA runtime version) at memory.c:298
CUDA ERROR (code = 35, CUDA driver version is insufficient for CUDA runtime version) at memory.c:298
CUDA ERROR (code = 35, CUDA driver version is insufficient for CUDA runtime version) at memory.c:298
CUDA ERROR (code = 35, CUDA driver version is insufficient for CUDA runtime version) at memory.c:298
--------------------------------------------------------------------------
Primary job  terminated normally, but 1 process returned
a non-zero exit code. Per user-direction, the job has been aborted.
--------------------------------------------------------------------------
--------------------------------------------------------------------------
mpirun detected that one or more processes exited with non-zero status, thus causing
the job to be terminated. The first process to do so was:

  Process name: [[56869,1],2]
  Exit code:    1
--------------------------------------------------------------------------
```

The underlying HYPRE library is built to run on nodes with NVIDIA GPUs present, and our login nodes on Caviness do not have GPUs or the NVIDIA GPU driver running on them.
