cmake_minimum_required(VERSION 3.28)
project(tg_cloud)

set(CMAKE_CXX_STANDARD 17)

add_executable(tg_cloud 
        main.cpp
        tdclient.h
        tdclient.cpp

        tgfs/tgfs.h
        tgfs/tgfs_fuse_dependencies.h
        tgfs/tgfs_main.cpp
        tgfs/tgfs_ops.cpp
        tgfs/tgfs_data.h
        tgfs/tgfs_data.cpp
        tgfs/tgfs_dir.h
        tgfs/tgfs_dir.cpp
        tgfs/tgfs_helpers.h
        tgfs/tgfs_helpers.cpp
        )

set_target_properties(tg_cloud 
        PROPERTIES 
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED true
        CXX_EXTENSIONS false
        )

add_subdirectory(td)
target_link_libraries(tg_cloud PRIVATE Td::TdStatic)