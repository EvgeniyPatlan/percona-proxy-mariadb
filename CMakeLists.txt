message(STATUS "CMake version: ${CMAKE_VERSION}")
cmake_minimum_required(VERSION 3.16)

if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.24.0)
  # We need the timestamps in the files to be the same as they are in the archive, otherwise the build starts
  # failing as automake thinks the files are out-of-date and need to be reconfigured.
  # https://cmake.org/cmake/help/latest/policy/CMP0135.html
  cmake_policy(SET CMP0135 OLD)
endif()

include(${CMAKE_SOURCE_DIR}/cmake/macros.cmake)
enable_testing()

# Packaging builds install to /usr and other builds to /usr/local
if(PACKAGE)
  set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Prefix prepended to install directories.")
endif()

# Set default values for cache entries and set the Percona Proxy version
include(cmake/defaults.cmake)

include(VERSION.cmake)
if (PERCONA_PROXY_MATURITY EQUAL "Develop")
  add_definitions(-DMAXSCALE_DEVELOP=1)
endif()

include(ExternalProject)
include(FetchContent)

set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

project(PerconaProxy)

# Set the installation layout
include(${CMAKE_SOURCE_DIR}/cmake/install_layout.cmake)

# Configure RPATH
# (it has to be before any executable generation)
include(cmake/rpath.cmake)

# Do the platform check
include(cmake/CheckPlatform.cmake)

check_dirs()
find_package(GSSAPI)
find_package(OpenSSL REQUIRED)
find_package(Git REQUIRED)
find_package(LibUUID REQUIRED)
find_package(CURL REQUIRED)
find_package(PAM REQUIRED)
find_package(GnuTLS REQUIRED)
find_package(Libatomic REQUIRED)
find_package(Tclsh REQUIRED)
find_package(ODBC REQUIRED)

if (FORCE_BUNDLE)
  include(cmake/BuildMicroHttpd.cmake)
  include(cmake/BuildJansson.cmake)
  include(cmake/BuildPCRE2.cmake)

  if (EXISTS /usr/include/openssl/engine.h)
    file(READ /usr/include/openssl/engine.h OPENSSL_ENGINE_CONTENTS)
    string(FIND "${OPENSSL_ENGINE_CONTENTS}" "ENGINE_init" HAVE_OPENSSL_ENGINES)
    if (HAVE_OPENSSL_ENGINES GREATER -1)
      include(cmake/BuildLibSSH.cmake)
      set(BUILDING_LIBSSH TRUE CACHE INTERNAL "")
    else()
      message(STATUS "<openssh/engine.h> doesn't define ENGINE_init(). This is probably RHEL 10 or newer which removed support for OpenSSL engines due to them being deprecated. Linking against system libssh as it cannot be built.")
      find_package(LibSSH REQUIRED)
      set(BUILDING_LIBSSH FALSE CACHE INTERNAL "")
    endif()
    message(STATUS "Couldn't find <openssh/engine.h>. Linking against system libssh as it cannot be built.")
    find_package(LibSSH REQUIRED)
    set(BUILDING_LIBSSH FALSE CACHE INTERNAL "")
  endif()

  # Both of these modules require librdkafka
  if (BUILD_KAFKACDC OR BUILD_KAFKAIMPORTER OR BUILD_MIRROR OR BUILD_SYSTEM_TESTS)
    include(cmake/BuildRdkafka.cmake)
    include_directories(${RDKAFKA_INCLUDE_DIR})
  endif()

else()
  find_package(MicroHttpd REQUIRED)
  find_package(Jansson REQUIRED)
  find_package(PCRE2 REQUIRED)
  find_package(LibSSH REQUIRED)
  find_package(Rdkafka)
endif()

if (PCRE2_VERSION VERSION_LESS_EQUAL "10.32")
  message(STATUS "PCRE2 ${PCRE2_VERSION} is too old, disabling JIT compilation.")
  add_definitions(-DMXS_PCRE2_NO_JIT=1)
endif()

if (WITH_LTO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT ipo_res OUTPUT ipo_out)
  if(ipo_res)
    # IPO supported, enable it on all targets.
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    message(STATUS "Link-time optimization enabled")
  else()
    message(FATAL_ERROR "Link-time optimization is not supported: ${ipo_out}")
  endif()
endif()

# Always build Connector-C from a known good commit
include(cmake/BuildMariaDBConnector.cmake)
include_directories(BEFORE ${MARIADB_CONNECTOR_INCLUDE_DIR})

# Include the directorories of the libraries. As these might be custom versions,
# we always include them before all other directories.
include_directories(BEFORE ${MICROHTTPD_INCLUDE_DIR})
include_directories(BEFORE ${JANSSON_INCLUDE_DIR})
include_directories(BEFORE ${PCRE2_INCLUDE_DIRS})

# Include jwt-cpp headers
include_directories(jwt-cpp/include)

if(NOT OPENSSL_FOUND AND NOT BUILD_SYSTEM_TESTS)
  message(FATAL_ERROR "Failed to locate dependency: OpenSSL")
else()
  if(OPENSSL_VERSION VERSION_LESS 1 AND NOT FORCE_OPENSSL100)
    add_definitions("-DOPENSSL_0_9")
  elseif(OPENSSL_VERSION VERSION_LESS 1.1)
    add_definitions("-DOPENSSL_1_0")
  else()
    add_definitions("-DOPENSSL_1_1")
  endif()
endif()

if(PERCONA_PROXY_COMMIT)
  # Given on the command line, e.g. when building from a source tarball that has no .git directory.
  message(STATUS "Commit ID (from -DMAXSCALE_COMMIT): ${PERCONA_PROXY_COMMIT}")
elseif(GIT_FOUND)
  message(STATUS "Found git ${GIT_VERSION_STRING}")
  execute_process(COMMAND ${GIT_EXECUTABLE} rev-list --max-count=1 HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT
    ERROR_VARIABLE GIT_ERROR
    RESULT_VARIABLE GIT_RVAL)
  if(${GIT_RVAL} EQUAL 0)
    string(REPLACE "\n" "" PERCONA_PROXY_COMMIT ${GIT_COMMIT})
    message(STATUS "Commit ID: ${PERCONA_PROXY_COMMIT}")
  else()
    message(STATUS "Git exited with non-zero value: ${GIT_ERROR}")
    message(STATUS "Could not find repository in source folder, Percona Proxy commit ID will not be resolved. Will use 'source-build' for commit ID.")
    set(PERCONA_PROXY_COMMIT "source-build")
  endif()
else()
  message(WARNING "Could not find git, Percona Proxy commit ID will not be resolved. Will use 'source-build' for commit ID.")
  set(PERCONA_PROXY_COMMIT "source-build")
endif()

# Copy cmake_flags, JENKINS_BUILD_TAG, source and value evironmental variables
# into cmake variables. These are used by the build system to store information
# about the packages being built.
set(PERCONA_PROXY_SOURCE "$ENV{source} $ENV{value}")
set(PERCONA_PROXY_CMAKE_FLAGS "$ENV{cmake_flags}")
set(PERCONA_PROXY_JENKINS_BUILD_TAG "$ENV{BUILD_TAG}")

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/server/include)
configure_file(${CMAKE_SOURCE_DIR}/server/test/percona_proxy_test.h.in ${CMAKE_BINARY_DIR}/include/percona-proxy/percona_proxy_test.h @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/etc/postinst.in ${CMAKE_BINARY_DIR}/postinst @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/etc/prerm.in ${CMAKE_BINARY_DIR}/prerm @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/etc/upstart/percona-proxy.conf.in ${CMAKE_BINARY_DIR}/upstart/percona-proxy.conf @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/test/percona-proxy-test.cnf ${CMAKE_BINARY_DIR}/percona-proxy.cnf @ONLY)

set(FLAGS "-fno-omit-frame-pointer -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-but-set-variable -Werror=format-security -Werror -fPIC" CACHE STRING "Compilation flags")

# clang complains if a compile flag is not used
add_link_options("-rdynamic")

if(GCOV)
  set(FLAGS "${FLAGS} --coverage -O0 -g" CACHE STRING "Compilation flags"  FORCE)
endif()

if(PROFILE)
  message(STATUS "Profiling executables")
  set(FLAGS "${FLAGS} -pg " CACHE STRING "Compilation flags" FORCE)
endif()

if (WITH_ASAN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
  # Only relevant when Percona Proxy is linked with -fsanitize=address
  set(SERVICE_FILE_ASAN_OPTIONS "Environment=ASAN_OPTIONS=abort_on_error=1:unmap_shadow_on_exit=1")
  add_definitions(-DMXS_WITH_ASAN=1)
endif()
if (WITH_TSAN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
endif()
if (WITH_UBSAN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined -fno-sanitize=vptr -fno-sanitize-recover")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -fno-sanitize=vptr -fno-sanitize-recover")
  set(SERVICE_FILE_UBSAN_OPTIONS "Environment=UBSAN_OPTIONS=abort_on_error=1:print_stacktrace=1")
endif()

# disable warnings emanating from sqlite
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(FLAGS "${FLAGS} -Wno-tautological-constant-out-of-range-compare")
  set(FLAGS "${FLAGS} -Wno-implicit-const-int-float-conversion")
  set(FLAGS "${FLAGS} -Wno-unqualified-std-cast-call")
else() # GCC
  if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER 7.1.0)
    add_compile_options(-Wshadow=local)
  endif()
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -D_GNU_SOURCE=1 ${FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb -DSS_DEBUG -DLOG_ASSERT")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -ggdb")

# CRTP in config2.hh subclasses redefining the base class virtual to_json()
# causes a warning. Calling the virtual version with such a subclass would
# be a compilation error, so the warning is not very important.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-overloaded-virtual")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${FLAGS} ${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wsuggest-override")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb -DSS_DEBUG -DLOG_ASSERT")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -ggdb")

include_directories(include)
include_directories(server/modules/include)
include_directories(${CMAKE_BINARY_DIR}/include)
include_directories(${CURL_INCLUDE_DIRS})
include_directories(maxutils/maxbase/include)
include_directories(maxutils/maxsql/include)
include_directories(maxutils/maxsimd/include)

add_subdirectory(maxutils)
add_subdirectory(server)
add_subdirectory(include/percona-proxy)
add_subdirectory(percona-proxyctl)
add_subdirectory(connectors)
add_subdirectory(gui)

if(BUILD_TESTS)
  add_subdirectory(examples)
endif()

if(BUILD_SYSTEM_TESTS)
  add_subdirectory(system-test)
endif()

install_file(etc/percona-proxy.cnf.template core)
install_script(script/percona-proxy-generate-support-info.py core)
install_script(script/percona-proxy-migrate core)

# Install the template into /etc
if(WITH_MAXSCALE_CNF AND (NOT TARGET_COMPONENT OR "core" STREQUAL "${TARGET_COMPONENT}"))
  install_custom_file(etc/percona-proxy.cnf.template ${PERCONA_PROXY_CONFDIR} core)
endif()

install_file(${CMAKE_SOURCE_DIR}/COPYRIGHT core)
install_file(${CMAKE_SOURCE_DIR}/README.md core)
install(FILES ${CMAKE_SOURCE_DIR}/licenses/LICENSE2308.TXT RENAME LICENSE.TXT DESTINATION ${PERCONA_PROXY_SHAREDIR} COMPONENT core)
install_file(${CMAKE_SOURCE_DIR}/LICENSE-THIRDPARTY.TXT core)
install_file(${CMAKE_SOURCE_DIR}/Documentation/Changelog.md core)
install_manual(Documentation/percona-proxy.1 1 core)
install_manual(Documentation/percona-proxyctl.1 1 core)
install_manual(Documentation/percona-proxy-keys.1 1 core)
install_manual(Documentation/percona-proxy-passwd.1 1 core)
install_file(${CMAKE_SOURCE_DIR}/etc/logrotate.d/percona-proxy-logrotate core)

# For devel package, these files are put to the base folder
install_header(${CMAKE_SOURCE_DIR}/COPYRIGHT devel)
install_header(${CMAKE_SOURCE_DIR}/LICENSE.TXT devel)

# Install startup scripts and other system configuration files
if(WITH_SCRIPTS)
  include(cmake/init_scripts.cmake)
endif()

# The spec file and debian/rules build the packages themselves and only need the packaging
# install layout, not the CPack configuration. CPack also supports one component at a time,
# while native packaging builds several (core and devel) in a single pass.
option(SKIP_CPACK "Do not configure CPack even when PACKAGE is enabled" OFF)

if(PACKAGE AND NOT SKIP_CPACK)
  # Configure packaging
  include(cmake/package.cmake)

  # Generates a source tarball
  add_custom_target(dist
    COMMAND ${CMAKE_CPACK_COMMAND} -G TGZ --config ${CMAKE_BINARY_DIR}/CPackSourceConfig.cmake
    COMMENT "Building source tarball" VERBATIM)

  # If we're building a source package and the GUI or Percona Proxyctl are being built, bundle the
  # node_modules so that they don't need to be downloaded.
  set(CPACK_SOURCE_INSTALLED_DIRECTORIES "${CMAKE_SOURCE_DIR};.")

  set(CPACK_PACKAGE_CHECKSUM "SHA256")

  if (BUILD_GUI)
    set(CPACK_SOURCE_INSTALLED_DIRECTORIES "${CPACK_SOURCE_INSTALLED_DIRECTORIES};${CMAKE_BINARY_DIR}/gui/node_modules;./gui/node_modules")
    add_dependencies(dist gui_node_modules)
  endif()

  if (BUILD_MAXCTRL)
    set(CPACK_SOURCE_INSTALLED_DIRECTORIES "${CPACK_SOURCE_INSTALLED_DIRECTORIES};${CMAKE_BINARY_DIR}/percona-proxyctl/node_modules;./percona-proxyctl/node_modules")
    add_dependencies(dist percona-proxyctl_node_modules)
  endif()

endif()

if(PACKAGE)
  # Install the files copied by the postinst script into the share folder
  install_program(${CMAKE_BINARY_DIR}/percona-proxy core)
  install_file(${CMAKE_BINARY_DIR}/percona-proxy.conf core)
  install_program(${CMAKE_BINARY_DIR}/postinst core)
  install_program(${CMAKE_BINARY_DIR}/prerm core)
endif()

if(PACKAGE AND NOT SKIP_CPACK)
  # The inclusion of CPack needs to be the last effective packaging related command. All
  # configurations to packaging done after the call will be ignored.
  include(CPack)
endif()

#
# Custom targets for Percona Proxy
#

# uninstall target
# see http://www.cmake.org/Wiki/CMake_FAQ#Can_I_do_.22make_uninstall.22_with_CMake.3F
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

# NOTE: If you make changes here, ensure they are compatible with the
#       situation in paths.h.in.
if (NOT PACKAGE)
  install(DIRECTORY DESTINATION var/cache/percona-proxy)
  install(DIRECTORY DESTINATION var/log/percona-proxy)
  install(DIRECTORY DESTINATION var/run/percona-proxy)
  install(DIRECTORY DESTINATION var/lib/percona-proxy)
  install(DIRECTORY DESTINATION ${DEFAULT_MODULE_CONFIG_SUBPATH})
endif()
