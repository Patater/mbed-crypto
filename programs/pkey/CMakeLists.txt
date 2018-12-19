add_executable(dh_genprime dh_genprime.c)
target_link_libraries(dh_genprime mbedcrypto)
target_include_directories(dh_genprime
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(ecdh_curve25519 ecdh_curve25519.c)
target_link_libraries(ecdh_curve25519 mbedcrypto)
target_include_directories(ecdh_curve25519
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(ecdsa ecdsa.c)
target_link_libraries(ecdsa mbedcrypto)
target_include_directories(ecdsa
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(gen_key gen_key.c)
target_link_libraries(gen_key mbedcrypto)
target_include_directories(gen_key
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(key_app key_app.c)
target_link_libraries(key_app mbedcrypto)
target_include_directories(key_app
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(key_app_writer key_app_writer.c)
target_link_libraries(key_app_writer mbedcrypto)
target_include_directories(key_app_writer
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(mpi_demo mpi_demo.c)
target_link_libraries(mpi_demo mbedcrypto)
target_include_directories(mpi_demo
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(rsa_genkey rsa_genkey.c)
target_link_libraries(rsa_genkey mbedcrypto)
target_include_directories(rsa_genkey
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(rsa_sign rsa_sign.c)
target_link_libraries(rsa_sign mbedcrypto)
target_include_directories(rsa_sign
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(rsa_verify rsa_verify.c)
target_link_libraries(rsa_verify mbedcrypto)
target_include_directories(rsa_verify
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(rsa_sign_pss rsa_sign_pss.c)
target_link_libraries(rsa_sign_pss mbedcrypto)
target_include_directories(rsa_sign_pss
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(rsa_verify_pss rsa_verify_pss.c)
target_link_libraries(rsa_verify_pss mbedcrypto)
target_include_directories(rsa_verify_pss
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(rsa_encrypt rsa_encrypt.c)
target_link_libraries(rsa_encrypt mbedcrypto)
target_include_directories(rsa_encrypt
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(rsa_decrypt rsa_decrypt.c)
target_link_libraries(rsa_decrypt mbedcrypto)
target_include_directories(rsa_decrypt
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(pk_sign pk_sign.c)
target_link_libraries(pk_sign mbedcrypto)
target_include_directories(pk_sign
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(pk_verify pk_verify.c)
target_link_libraries(pk_verify mbedcrypto)
target_include_directories(pk_verify
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(pk_encrypt pk_encrypt.c)
target_link_libraries(pk_encrypt mbedcrypto)
target_include_directories(pk_encrypt
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

add_executable(pk_decrypt pk_decrypt.c)
target_link_libraries(pk_decrypt mbedcrypto)
target_include_directories(pk_decrypt
    PRIVATE ${CMAKE_SOURCE_DIR}/library/include/)

install(TARGETS dh_genprime key_app mpi_demo rsa_genkey rsa_sign rsa_verify rsa_encrypt rsa_decrypt pk_encrypt pk_decrypt pk_sign pk_verify gen_key
        DESTINATION "bin"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
