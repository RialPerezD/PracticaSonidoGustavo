cmake_minimum_required(VERSION 3.20)

project(PracticaSonidoGustavo LANGUAGES C CXX)

# -------------------------------
#   Configuración del lenguaje
# -------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------------
#   Rutas de cabeceras
# -------------------------------
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/deps/ESAT_rev248/include
)

# -------------------------------
#   Archivos fuente
# -------------------------------
file(GLOB SRC_FILES 
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/*.cc
)

# -------------------------------
#   Ejecutable final
# -------------------------------
add_executable(PracticaSonido ${SRC_FILES})

# -------------------------------
#   Forzar aplicación de consola
# -------------------------------
# Esto asegura que el linker busque main() y no WinMain()
set_target_properties(PracticaSonido PROPERTIES
    WIN32_EXECUTABLE OFF
)

# -------------------------------
#   Librerías externas
# -------------------------------
target_link_libraries(PracticaSonido
    ${PROJECT_SOURCE_DIR}/deps/ESAT_rev248/bin/ESAT_d.lib
    ${PROJECT_SOURCE_DIR}/deps/OpenAL/libs/Win32/OpenAL32.lib
    opengl32.lib
    user32.lib
    gdi32.lib
    shell32.lib
    Ws2_32.lib
)

if(MSVC)
    # Desactiva SAFESEH para este target
    set_target_properties(PracticaSonido PROPERTIES
        LINK_FLAGS "/SAFESEH:NO"
    )
endif()

# -------------------------------
#   Carpeta donde se generará el exe
# -------------------------------
set_target_properties(PracticaSonido PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build
)

# -------------------------------
#   Carpeta de solución para organizar
# -------------------------------
set_target_properties(PracticaSonido PROPERTIES FOLDER "PracticaSonido")
