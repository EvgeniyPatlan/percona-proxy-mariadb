if(BUILD_TOOLS)
    # Tool for testing local PAM config.
    add_executable(test_pam_login test_pam_login.cc)
    target_link_libraries(test_pam_login maxbase)
    install_executable(test_pam_login core)
endif()

# Pam suid wrapper. Requires setuid permission
add_executable(percona-proxy-pam-auth-tool pam_auth_tool.cc)
target_link_libraries(percona-proxy-pam-auth-tool maxbase)
install_executable_setuid(percona-proxy-pam-auth-tool core)
