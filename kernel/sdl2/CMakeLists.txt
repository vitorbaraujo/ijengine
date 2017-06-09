cmake_minimum_required(VERSION 3.0.2)
project(sdl_kernel)

# Creating variables

# set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
# set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
# set(LIB_DIR "${PROJECT_SOURCE_DIR}/libs")
# set(IJENGINE_DIR "/usr/local/include/ijengine")
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/*.cpp")
file(GLOB INCLUDES "${PROJECT_SOURCE_DIR}/*.cpp")

set(IJENGINE_INCLUDE_DIR "../../include")
set(IJENGINE_LIB_DIR "../../lib")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -W -Wall -Wextra -Wshadow -Wcast-align -Wcast-qual -Wctor-dtor-privacy\
		                -Wdisabled-optimization -Wformat=2 -Wlogical-op -Wmissing-declarations\
		                -Wmissing-include-dirs -Wnoexcept -Woverloaded-virtual -Wredundant-decls\
		                -Wsign-promo -Wstrict-null-sentinel -Wswitch-default -Wundef\
		                -Wzero-as-null-pointer-constant -Wuseless-cast -Wnon-virtual-dtor\
		                -fPIC -shared -rdynamic"
                  )

# Including header files

include_directories(${IJENGINE_LIB_DIR})
include_directories(${IJENGINE_INCLUDE_DIR})

# Creating executable and adding libraries to CMake

add_executable(kernel.0.1.0.so ${SOURCES} ${LIB_DIR})
set_property(TARGET kernel.0.1.0.so PROPERTY CXX_STANDARD 11)
target_link_libraries(kernel.0.1.0.so SDL2 SDL2_image SDL2_ttf SDL2_mixer ijengine)
