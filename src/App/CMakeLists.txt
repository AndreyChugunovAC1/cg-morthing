set(SRCS
    main.cpp
    Window.cpp
    Duck.cpp
    Morth.cpp
    Duck.h
    Window.h
    Morth.h

    Shaders/diffuse.fs
    Shaders/diffuse.vs
    Shaders/morth.fs
    Shaders/morth.vs
    Models/Duck.glb
    Textures/Duck.png

    resources.qrc
)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(demo-app ${SRCS})

target_link_libraries(demo-app
    PRIVATE
        Qt5::Widgets
        FGL::Base
        thirdparty::tinygltf
)