#
# CMake build system for NHWAVE
#
CMAKE_MINIMUM_REQUIRED (VERSION 3.1.2)
PROJECT (NHWAVE VERSION 3.0 LANGUAGES Fortran C)

# Our own add-ons:
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
INCLUDE(GNUInstallDirs)

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
OPTION(DOUBLE_PRECISION "Floating-point compution in double- rather than single-precision." OFF)
OPTION(PARALLEL "Include MPI parallelism." OFF)
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
OPTION(HYPRE_CUDA "HYPRE library uses CUDA functionality." OFF)

# We need HYPRE:
FIND_PACKAGE(HYPRE REQUIRED)

# If PARALLEL is selected, we need an MPI compiler:
IF (${PARALLEL})
   FIND_PACKAGE(MPI REQUIRED)
ENDIF()

#
# The product:
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
        hypre_utils.c
        nhwave.F
    )
ADD_EXECUTABLE(nhwave ${NHWAVE_SOURCES})

# The executable must link against HYPRE:
TARGET_INCLUDE_DIRECTORIES(nhwave PUBLIC ${HYPRE_INCLUDE_DIRS} ${CMAKE_BINARY_DIR})
TARGET_LINK_LIBRARIES(nhwave ${HYPRE_LIBRARY})

# The executable optionally links against an MPI library:
IF (${PARALLEL})
    TARGET_INCLUDE_DIRECTORIES(nhwave PUBLIC ${MPI_INCLUDE_DIRS})
    TARGET_LINK_LIBRARIES(nhwave ${MPI_Fortran_LIB_NAMES})
    TARGET_LINK_DIRECTORIES(nhwave PUBLIC ${MPI_Fortran_MODULE_DIR})
ENDIF()

# Add preprocessing flags for each option enabled:
FOREACH(OPTION_FLAG
        DOUBLE_PRECISION PARALLEL LANDSLIDE LANDSLIDE_COMPREHENSIVE TWOLAYERSLIDE DEFORMABLESLIDE
        SALINITY TEMPERATURE BUBBLE SEDIMENT VEGETATION INTEL BALANCE2D OBSTACLE CORALREEF POROUSMEDIA
        FROUDE_CAP COUPLING FLUIDSLIDE
    )
    IF ( ${${OPTION_FLAG}} )
        TARGET_COMPILE_DEFINITIONS(nhwave PUBLIC ${OPTION_FLAG})
    ENDIF()
ENDFOREACH()

# Install the executable:
INSTALL(TARGETS nhwave
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
