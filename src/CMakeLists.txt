cmake_minimum_required(VERSION 3.11) # FetchContent is new in version 3.11.
project(DH2323Project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall")

add_executable(DH2323Project 
    main.cpp 
    Window.cpp 
    LambertianShader.cpp 
    DipoleShader.cpp 
    WireframeShader.cpp 
    
)
target_include_directories(DH2323Project PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

include(FetchContent)


FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	0af55ccecd98d4e5a8d1fad7de25ba429d60e863 #refs/tags/1.0.1
)

FetchContent_MakeAvailable(glm )

FetchContent_Declare(
	SDL3
	GIT_REPOSITORY	https://github.com/libsdl-org/SDL.git
	GIT_TAG 	main
)

FetchContent_MakeAvailable(SDL3)

target_link_libraries(DH2323Project PRIVATE glm::glm SDL3::SDL3)