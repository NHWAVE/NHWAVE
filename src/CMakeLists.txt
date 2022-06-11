#
# CMake build system for NHWAVE
#
CMAKE_MINIMUM_REQUIRED (VERSION 3.8)
PROJECT (NHWAVE VERSION 3.0 LANGUAGES Fortran C)

# Our own add-ons:
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
INCLUDE(GNUInstallDirs)

# For preserving config around source checks:
INCLUDE(CMakePushCheckState)

# Check execution of code tests:
INCLUDE(CheckCSourceCompiles)

# Fortran source should be treated as free-format:
SET(CMAKE_Fortran_FORMAT "FREE")

# Intel Fortran?
IF (${CMAKE_Fortran_COMPILER_ID} MATCHES "^Intel")
    SET(IS_INTEL_FORTRAN On)
ELSE()
    SET(IS_INTEL_FORTRAN Off)
ENDIF()

# Get the C-to-Fortran interface info:
INCLUDE(FortranCInterface)
FortranCInterface_HEADER(${CMAKE_BINARY_DIR}/fortran-interfaces.h)

#
# The various build options.  Almost all of them default to being
# disabled/off, except the INTEL option:  if the Fortran compiler is
# identified by CMake as being Intel-branded, then the INTEL option
# defaults to enabled/on.
#
# Another notable exception is the HYPRE_CUDA option, which gets
# auto-detected once the HYPRE library is found.
#
SET(OPTION_NAMES
        PARALLEL
        DOUBLE_PRECISION
        LANDSLIDE
        LANDSLIDE_COMPREHENSIVE
        TWOLAYERSLIDE
        DEFORMABLESLIDE
        SALINITY
        TEMPERATURE
        BUBBLE
        SEDIMENT
        VEGETATION
        INTEL
        BALANCE2D
        OBSTACLE
        CORALREEF
        POROUSMEDIA
        FROUDE_CAP
        COUPLING
        FLUIDSLIDE
        HYPRE_CUDA
    )
#
OPTION(PARALLEL "Include MPI parallelism; requires the HYPRE library." OFF)
OPTION(DOUBLE_PRECISION "Floating-point compution in double- rather than single-precision." OFF)
OPTION(LANDSLIDE "Include landslide terms." OFF)
OPTION(LANDSLIDE_COMPREHENSIVE "Include comprehensive landslide terms." OFF)
OPTION(TWOLAYERSLIDE "Include multi-layer landslide terms." OFF)
OPTION(DEFORMABLESLIDE "Include deformable landslide terms." OFF)
OPTION(SALINITY "Include salinity terms." OFF)
OPTION(TEMPERATURE "Include temperature-coupling terms." OFF)
OPTION(BUBBLE "Include bubble terms." OFF)
OPTION(SEDIMENT "Include sediment terms." OFF)
OPTION(VEGETATION "Include vegetation terms." OFF)
OPTION(INTEL "Compilation with Intel Fortran?" ${IS_INTEL_FORTRAN})
OPTION(BALANCE2D "Include cross-shore momentum balance terms." OFF)
OPTION(OBSTACLE "Include obstacle effect terms." OFF)
OPTION(CORALREEF "Include coral reef term." OFF)
OPTION(POROUSMEDIA "Include generic porous media terms." OFF)
OPTION(FROUDE_CAP "Include maximum Froude number limiting terms." OFF)
OPTION(COUPLING "Enable computational nesting." OFF)
OPTION(FLUIDSLIDE "Include fluid slide terms." OFF)

#
# If PARALLEL is enabled, we need to find HYPRE and MPI libraries:
#
IF (${PARALLEL})
    # We need HYPRE:
    FIND_PACKAGE(HYPRE REQUIRED)

    # Also need MPI:
    FIND_PACKAGE(MPI REQUIRED)

    # Check if HYPRE PARALLEL and GPU functionality are present:
    CMAKE_PUSH_CHECK_STATE()
        SET(CMAKE_REQUIRED_LIBRARIES ${HYPRE_LIBRARY})
        SET(CMAKE_REQUIRED_INCLUDES ${HYPRE_INCLUDE_DIRS})

        CHECK_C_SOURCE_COMPILES("
            #include \"_hypre_utilities.h\"
            #include \"HYPRE.h\"
            int
            main()
            {
                #ifdef HYPRE_SEQUENTIAL
                #error FAILED:  HYPRE NOT PARALLEL
                #endif
                return 0;
            }
            "
            HAS_HYPRE_PARALLEL
        )
        IF (NOT ${HAS_HYPRE_PARALLEL} )
            MESSAGE(FATAL_ERROR "NHWAVE PARALLEL option selected but HYPRE library lacks parallelism.")
        ENDIF()
    
        CHECK_C_SOURCE_COMPILES("
            #include \"_hypre_utilities.h\"
            #include \"HYPRE.h\"
            int
            main()
            {
                #ifndef HYPRE_USING_GPU
                #error FAILED:  MISSING HYPRE_USING_GPU
                #endif
                return 0;
            }
            "
            HAS_HYPRE_CUDA
        )
        OPTION(HYPRE_CUDA "HYPRE library uses CUDA functionality." ${HAS_HYPRE_CUDA})
        
    CMAKE_POP_CHECK_STATE()
ENDIF()

#
# List of source files that comprise the executable we wish to build:
#
LIST(APPEND NHWAVE_SOURCES
        mod_global.F
        mod_util.F
        initialize.F
        two_layer_slide.F
        fluid_slide.F
        deformable_slide.F
        source_terms.F
        waves.F
        pressure.F
        nhwave.F
    )
IF (${PARALLEL})
    IF (${HYPRE_CUDA})
        LIST(APPEND NHWAVE_SOURCES
                hypre_bind_device.c
            )
    ENDIF()
ELSE()
    LIST(APPEND NHWAVE_SOURCES
            nspcg.f
        )
ENDIF()

#
# Add the "nhwave" executable target:
#
ADD_EXECUTABLE(nhwave ${NHWAVE_SOURCES})

IF (${PARALLEL})
    # The executable must link against HYPRE and MPI:
    TARGET_INCLUDE_DIRECTORIES(nhwave PUBLIC ${HYPRE_INCLUDE_DIRS} ${MPI_INCLUDE_DIRS} ${CMAKE_BINARY_DIR})
    TARGET_LINK_LIBRARIES(nhwave ${HYPRE_LIBRARY} ${MPI_Fortran_LIB_NAMES})
    TARGET_LINK_DIRECTORIES(nhwave PUBLIC ${MPI_Fortran_MODULE_DIR})
ENDIF()

# Add preprocessing flags for each option enabled:
FOREACH(OPTION_FLAG ${OPTION_NAMES})
    IF ( ${${OPTION_FLAG}} )
        TARGET_COMPILE_DEFINITIONS(nhwave PUBLIC ${OPTION_FLAG})
    ENDIF()
ENDFOREACH()

# Install the executable:
INSTALL(TARGETS nhwave
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
