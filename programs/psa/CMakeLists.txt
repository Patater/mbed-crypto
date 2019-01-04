add_executable(crypto_examples crypto_examples.c)
target_link_libraries(crypto_examples mbedcrypto)
target_include_directories(crypto_examples
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

install(TARGETS crypto_examples
        DESTINATION "bin"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

