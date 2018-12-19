set(libs
    mbedcrypto
)

if(USE_PKCS11_HELPER_LIBRARY)
    set(libs ${libs} pkcs11-helper)
endif(USE_PKCS11_HELPER_LIBRARY)

if(ENABLE_ZLIB_SUPPORT)
    set(libs ${libs} ${ZLIB_LIBRARIES})
endif(ENABLE_ZLIB_SUPPORT)

add_executable(selftest selftest.c)
target_link_libraries(selftest ${libs})
target_include_directories(selftest
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(benchmark benchmark.c)
target_link_libraries(benchmark ${libs})
target_include_directories(benchmark
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

if(TEST_CPP)
    add_executable(cpp_dummy_build cpp_dummy_build.cpp)
    target_link_libraries(cpp_dummy_build ${libs})
endif()

add_executable(zeroize zeroize.c)
target_link_libraries(zeroize ${libs})
target_include_directories(zeroize
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

install(TARGETS selftest benchmark
        DESTINATION "bin"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
