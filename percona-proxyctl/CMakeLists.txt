if (BUILD_MAXCTRL)
  find_package(NPM)
  find_package(NodeJS)

  if (NPM_FOUND AND NODEJS_FOUND AND NODEJS_VERSION VERSION_GREATER "6.0.0")

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lib/version.js.in ${CMAKE_CURRENT_BINARY_DIR}/lib/version.js @ONLY)

    file(GLOB_RECURSE MAXCTRL_SOURCES lib/*)

    add_custom_target(percona-proxyctl_copy_sources
      COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/copy_sources.sh ${CMAKE_SOURCE_DIR}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

    # This target generates the node_modules directory which is needed by both
    # packaging methods.
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/node_modules
      DEPENDS percona-proxyctl_copy_sources package.json package-lock.json
      COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/npm_install.sh ${CMAKE_SOURCE_DIR}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    add_custom_target(percona-proxyctl_node_modules DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/node_modules)

    if (WITH_SYSTEM_NODEJS)
      # Runs webpack to bundle all the sources and dependencies into one file.
      add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dist/percona-proxyctl.js
        DEPENDS percona-proxyctl_node_modules ${MAXCTRL_SOURCES} percona-proxyctl.js
        COMMAND npm ci
        COMMAND npm run build
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
      add_custom_target(percona-proxyctl_build DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dist/percona-proxyctl.js)

      # Bundle the node_modules and sources into /usr/share/percona-proxy/ and then
      # install a wrapper script that just calls `node percona-proxyctl.js` with the
      # arguments.
      configure_file(${CMAKE_CURRENT_SOURCE_DIR}/percona-proxyctl.in ${CMAKE_CURRENT_BINARY_DIR}/percona-proxyctl @ONLY)
      add_custom_target(percona-proxyctl ALL DEPENDS percona-proxyctl_build)

      install_custom_file(${CMAKE_CURRENT_BINARY_DIR}/dist/percona-proxyctl.js ${PERCONA_PROXY_SHAREDIR}/percona-proxyctl core)
      install_script(${CMAKE_CURRENT_BINARY_DIR}/percona-proxyctl core)
    else()
      # Build Percona Proxyctl into a single executable using pkg. This makes it so that
      # the same Node.js version is used in all operating systems.
      add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/percona-proxyctl
        DEPENDS ${MAXCTRL_SOURCES} percona-proxyctl.js
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/build.sh ${CMAKE_SOURCE_DIR}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
      add_custom_target(percona-proxyctl ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/percona-proxyctl percona-proxyctl_node_modules)

      install_script(${CMAKE_BINARY_DIR}/percona-proxyctl/percona-proxyctl core)
    endif()

    add_custom_target(test_percona-proxyctl
      COMMAND ${CMAKE_SOURCE_DIR}/test/run_npm_test.sh
      ${CMAKE_SOURCE_DIR}           # Path to Percona Proxy sources
      ${CMAKE_CURRENT_SOURCE_DIR}   # Path to test sources
      ${CMAKE_BINARY_DIR}/percona-proxyctl-test/ # Location where tests are built and run
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

  elseif(NOT BUILD_SYSTEM_TESTS)
    message(FATAL_ERROR "Not building Percona Proxyctl: npm or Node.js >= 6.0.0 not found. Add the following to skip Percona Proxyctl: -DBUILD_MAXCTRL=N")
  endif()
else()
  message(STATUS "Not building Percona Proxyctl: BUILD_MAXCTRL=N")
endif()
