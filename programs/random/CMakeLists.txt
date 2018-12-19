add_executable(gen_random_havege gen_random_havege.c)
target_link_libraries(gen_random_havege mbedcrypto)
target_include_directories(gen_random_havege
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(gen_random_ctr_drbg gen_random_ctr_drbg.c)
target_link_libraries(gen_random_ctr_drbg mbedcrypto)
target_include_directories(gen_random_ctr_drbg
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(gen_entropy gen_entropy.c)
target_link_libraries(gen_entropy mbedcrypto)
target_include_directories(gen_entropy
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

install(TARGETS gen_random_havege gen_random_ctr_drbg gen_entropy
        DESTINATION "bin"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
