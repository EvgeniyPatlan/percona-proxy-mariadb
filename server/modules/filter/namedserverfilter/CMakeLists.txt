add_library(namedserverfilter SHARED namedserverfilter.cc)
target_link_libraries(namedserverfilter percona-proxy-common)
set_target_properties(namedserverfilter PROPERTIES VERSION "1.1.0" LINK_FLAGS -Wl,-z,defs)
install_module(namedserverfilter core)
