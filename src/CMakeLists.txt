cmake_minimum_required(VERSION 3.11) # FetchContent is new in version 3.11.
project(DH2323Project)

set(CMAKE_CXX_STANDARD 17)

add_executable(DH2323Project main.cpp)
# Add required libraries (e.g., OpenGL, GLFW, etc.) here



include(FetchContent)

FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	0af55ccecd98d4e5a8d1fad7de25ba429d60e863 #refs/tags/1.0.1
)

FetchContent_MakeAvailable(glm)

target_link_libraries(DH2323Project PRIVATE glm::glm)