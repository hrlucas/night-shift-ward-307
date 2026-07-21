cmake_minimum_required(VERSION 3.10)
project(GraveyardShiftGame C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")

# Arquivos-fonte: descobertos automaticamente (como o wildcard do Makefile),
# então esta lista nunca fica desatualizada ao adicionar/remover um arquivo.
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.c")

include_directories(include)

add_executable(graveyard_shift ${SOURCES})

install(TARGETS graveyard_shift DESTINATION bin)
