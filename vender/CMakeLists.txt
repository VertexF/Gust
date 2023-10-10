
if(WIN32)
    add_library(GLFW STATIC glfw/include/GLFW/glfw3.h
                            glfw/include/GLFW/glfw3native.h
                            glfw/src/context.c
                            glfw/src/init.c
                            glfw/src/input.c
                            glfw/src/monitor.c
                            glfw/src/vulkan.c
                            glfw/src/window.c
                            glfw/src/win32_init.c
                            glfw/src/win32_joystick.c
                            glfw/src/win32_monitor.c
                            glfw/src/win32_time.c
                            glfw/src/win32_thread.c
                            glfw/src/win32_window.c
                            glfw/src/wgl_context.c
                            glfw/src/egl_context.c
                            glfw/src/internal.h
                            glfw/src/mappings.h
                            glfw/src/osmesa_context.h
                            glfw/src/osmesa_context.c)
                            
    set_property(
        TARGET GLFW
        APPEND
        PROPERTY _CRT_SECURE_NO_WARNINGS _GLFW_WIN32
    )

    add_definitions(-D_GLFW_WIN32)

elseif(LINUX)
    add_library(GLFW STATIC glfw/include/GLFW/glfw3.h
                            glfw/include/GLFW/glfw3native.h
                            glfw/src/context.c
                            glfw/src/init.c
                            glfw/src/input.c
                            glfw/src/monitor.c
                            glfw/src/vulkan.c
                            glfw/src/window.c
                            glfw/src/x11_init.c
                            glfw/src/x11_monitor.c
                            glfw/src/x11_window.c
                            glfw/src/x11_platform.h
                            # glfw/src/wl_init.c
                            # glfw/src/wl_monitor.c
                            # glfw/src/wl_window.c
                            glfw/src/wl_platform.h
                            glfw/src/posix_time.h
                            glfw/src/posix_time.c
                            glfw/src/posix_thread.h
                            glfw/src/posix_thread.c
                            glfw/src/wgl_context.c
                            glfw/src/egl_context.c
                            glfw/src/linux_joystick.h
                            glfw/src/linux_joystick.c
                            glfw/src/internal.h
                            glfw/src/mappings.h
                            glfw/src/xkb_unicode.h
                            glfw/src/xkb_unicode.c
                            glfw/src/osmesa_context.h
                            glfw/src/osmesa_context.c)
                            
    set_property(
        TARGET GLFW
        APPEND
        PROPERTY _CRT_SECURE_NO_WARNINGS _GLFW_X11
    )

    add_definitions(-D_GLFW_X11)
endif()

add_library(STB STATIC stb/stb_image.h 
                       stb/stb_image.cpp 
                       stb/stb_truetype.h 
                       stb/stb_truetype.cpp 
                       stb/stb_rect_pack.h 
                       stb/stb_rect_pack.cpp)
