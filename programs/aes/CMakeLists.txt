add_executable(aescrypt2 aescrypt2.c)
target_link_libraries(aescrypt2 mbedcrypto)
target_include_directories(aescrypt2
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(crypt_and_hash crypt_and_hash.c)
target_link_libraries(crypt_and_hash mbedcrypto)
target_include_directories(crypt_and_hash
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

install(TARGETS aescrypt2 crypt_and_hash
        DESTINATION "bin"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
