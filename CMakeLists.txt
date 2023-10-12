cmake_minimum_required(VERSION 3.26.0)

project(GustEngine
		VERSION 0.0.0
		LANGUAGES CXX C
		DESCRIPTION "Vulkan Practice")
		
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE GUST_SOURCE
    "${CMAKE_CURRENT_SOURCE_DIR}/EngineSrc/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/EngineSrc/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/EngineSrc/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/EngineSrc/*.frag"
)

if(MSVC)
    set(GLFW_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vender/glfw/include")
    set(VULKAN_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vender/vulkan/include")
    set(SPDLOG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vender/spdlog/include")
    set(GML_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vender/glm")
    set(STB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vender/stb")
    #I need to uses a config file that is generated here. So it's not a 
    #mistake it's a binrary directory.
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${GUST_SOURCE})

    set(VULKAN_LIB "${CMAKE_CURRENT_SOURCE_DIR}/vender/vulkan/lib/vulkan-1.lib")

    #Set the location of the engine source.
    set(ENGINE_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/EngineSrc")
endif()

#spdlog has it's own CMake file that I haven't touched so it can remain in the vender base.
add_subdirectory(vender/spdlog)
add_subdirectory(vender)

add_compile_definitions("$<$<CONFIG:Debug>:GUST_DEBUG>")

add_executable(Gust ${GUST_SOURCE})

target_include_directories(Gust PUBLIC ${SPDLOG_INCLUDE_DIR} 
                                       ${GML_INCLUDE_DIR} 
                                       ${GLFW_INCLUDE_DIR} 
                                       ${STB_INCLUDE_DIR} 
                                       ${VULKAN_INCLUDE_DIR} 
                                       ${ENGINE_INCLUDE})

target_link_libraries(Gust PRIVATE spdlog STB GLFW ${VULKAN_LIB})

target_compile_options(Gust PRIVATE
					   $<$<CXX_COMPILER_ID:MSVC>:/W4>
					   $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)