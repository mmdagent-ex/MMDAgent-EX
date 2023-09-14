cmake_minimum_required(VERSION 3.10)

project(MMDAgent-EX)

#if(NOT CMAKE_BUILD_TYPE)
#  set(CMAKE_BUILD_TYPE "Release")
#endif()

if(APPLE)
    set(BREW_DIR "/opt/homebrew")
    include_directories(${BREW_DIR}/include)
    link_directories(${BREW_DIR}/lib)
    add_compile_definitions(
      TARGET_OS_IPHONE=0
      GL_SILENCE_DEPRECATION=1
    )
    find_package(OpenGL REQUIRED)
    set(MMDAGENT_LINK_OPTIONS
        BulletPhysics
        FlitePlusHTSEngine
        FreeType
        MMDFILES
        MMDAGENT
        LIBPNG
        -lstdc++
        -lGLEW
        -lssl
        -lcrypto
        -lPocoCrypto -lPocoFoundation -lPocoNet -lPocoNetSSL -lPocoUtil -lPocoXML -lPocoZip -lPocoJSON
        ${OPENGL_LIBRARIES}
        "-framework AGL"
        "-framework Cocoa"
        "-framework OpenGL"
        "-framework IOKit"
    )
else()
    set(MMDAGENT_LINK_OPTIONS
        BulletPhysics
        FlitePlusHTSEngine
        FreeType
        MMDFILES
        MMDAGENT
        LIBPNG
        ${OpenCV_LIBS}
        -lGLEW
        -lssl
        -lcrypto
        -lPocoCrypto -lPocoFoundation -lPocoNet -lPocoNetSSL -lPocoUtil -lPocoXML -lPocoZip -lPocoJSON
    )
endif()

SET(CMAKE_C_FLAGS_RELEASE "-O3 -fomit-frame-pointer -march=native -ffast-math")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -fomit-frame-pointer -march=native -ffast-math")
SET(CMAKE_POSITION_INDEPENDENT_CODE "On")

add_subdirectory(Library_Bullet_Physics)
add_subdirectory(Library_Flite_plus_hts_engine)
add_subdirectory(Library_FreeType)
add_subdirectory(Library_GLFW)
add_subdirectory(Library_libpng)
add_subdirectory(Library_hts_engine_API)
add_subdirectory(Library_librdkafka)
add_subdirectory(Library_Open_JTalk)
add_subdirectory(Library_Julius)
add_subdirectory(Library_MMDFiles)
add_subdirectory(Library_MMDAgent)
add_subdirectory(main)
add_subdirectory(Plugin_VIManager)
add_subdirectory(Plugin_Audio)
add_subdirectory(Plugin_Variables)
add_subdirectory(Plugin_TextArea)
add_subdirectory(Plugin_LookAt)
add_subdirectory(Plugin_ErrorBlock)
add_subdirectory(Plugin_Flite_plus_hts_engine)
add_subdirectory(Plugin_Julius)
add_subdirectory(Plugin_Kafka)
add_subdirectory(Plugin_Network)
add_subdirectory(Plugin_Open_JTalk)
add_subdirectory(Plugin_Remote)
add_subdirectory(Plugin_RabbitMQ)
