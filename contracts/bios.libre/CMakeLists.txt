add_contract(bios.libre bios.libre ${CMAKE_CURRENT_SOURCE_DIR}/src/bios.libre.cpp)

target_include_directories(bios.libre
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   ${CMAKE_CURRENT_SOURCE_DIR}/../eosio.libre/include)

set_target_properties(bios.libre
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/bios.libre.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/bios.libre.contracts.md @ONLY )

target_compile_options( bios.libre PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
