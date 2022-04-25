add_contract(eosio.libre eosio.libre ${CMAKE_CURRENT_SOURCE_DIR}/src/eosio.libre.cpp)

target_include_directories(eosio.libre
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   ${CMAKE_CURRENT_SOURCE_DIR}/../eosio.system/include)

set_target_properties(eosio.libre
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/eosio.libre.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/eosio.libre.contracts.md @ONLY )

target_compile_options( eosio.libre PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
