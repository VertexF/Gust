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
	"${CMAKE_CURRENT_SOURCE_DIR}/GameSrc/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/GameSrc/*.frag"
)

if(MSVC)
    set(GLFW_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vender/glfw/include")
    set(VULKAN_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vender/vulkan/include")
    set(SPDLOG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vender/spdlog/include")
    set(GML_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vender/glm")
    set(STB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vender/stb")
    set(OBJ_LOADER_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vender/tiny_obj_loader")
    #I need to uses a config file that is generated here. So it's not a 
    #mistake it's a binrary directory.
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${GUST_SOURCE})

    set(VULKAN_LIB "${CMAKE_CURRENT_SOURCE_DIR}/vender/vulkan/lib/vulkan-1.lib")
    set(GLFW_LIB "${CMAKE_CURRENT_SOURCE_DIR}/vender/glfw/lib-vc2022/glfw3.lib")
endif()

add_subdirectory(vender/spdlog)
add_subdirectory(vender/stb)

#Only here for testing reasons TODO: DELETE
add_library(TINY_OBJ STATIC "${CMAKE_CURRENT_SOURCE_DIR}/vender/tiny_obj_loader/tiny_obj_loader.h"
                            "${CMAKE_CURRENT_SOURCE_DIR}/vender/tiny_obj_loader/tiny_obj_loader.cpp")

add_compile_definitions("$<$<CONFIG:Debug>:GUST_DEBUG>")

add_executable(Gust ${GUST_SOURCE})

target_include_directories(Gust PUBLIC ${SPDLOG_INCLUDE_DIR} 
                                       ${PRECOMPILED_HEADER} 
                                       ${IMGUI_INCLUDE_DIR} 
                                       ${GML_INCLUDE_DIR} 
                                       ${GLFW_INCLUDE_DIR} 
                                       ${STB_INCLUDE_DIR} 
                                       ${VULKAN_INCLUDE_DIR} 
                                       ${OBJ_LOADER_INCLUDE_DIR} 
                                       ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Gust PRIVATE spdlog STB TINY_OBJ ${GLFW_LIB} ${VULKAN_LIB})

target_compile_options(Gust PRIVATE
					   $<$<CXX_COMPILER_ID:MSVC>:/W4>
					   $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)