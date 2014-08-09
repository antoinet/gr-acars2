INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_ACARS2 acars2)

FIND_PATH(
    ACARS2_INCLUDE_DIRS
    NAMES acars2/api.h
    HINTS $ENV{ACARS2_DIR}/include
        ${PC_ACARS2_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    ACARS2_LIBRARIES
    NAMES gnuradio-acars2
    HINTS $ENV{ACARS2_DIR}/lib
        ${PC_ACARS2_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ACARS2 DEFAULT_MSG ACARS2_LIBRARIES ACARS2_INCLUDE_DIRS)
MARK_AS_ADVANCED(ACARS2_LIBRARIES ACARS2_INCLUDE_DIRS)

