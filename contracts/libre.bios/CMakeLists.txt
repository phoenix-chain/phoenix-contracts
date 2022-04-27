add_contract(libre.bios libre.bios ${CMAKE_CURRENT_SOURCE_DIR}/src/libre.bios.cpp)

target_include_directories(libre.bios
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   ${CMAKE_CURRENT_SOURCE_DIR}/../eosio.libre/include)

set_target_properties(libre.bios
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/libre.bios.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/libre.bios.contracts.md @ONLY )

target_compile_options( libre.bios PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
