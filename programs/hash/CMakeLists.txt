add_executable(hello hello.c)
target_link_libraries(hello mbedcrypto)
target_include_directories(hello
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(generic_sum generic_sum.c)
target_link_libraries(generic_sum mbedcrypto)
target_include_directories(generic_sum
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

install(TARGETS hello generic_sum
        DESTINATION "bin"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
