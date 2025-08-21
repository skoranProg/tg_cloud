cmake_minimum_required(VERSION 3.28)
project(tg_cloud)

set(CMAKE_CXX_STANDARD 17)

add_executable(tg_cloud
        main.cpp
        tgcl/tdclient.cpp
        tgcl/td_helpers.cpp
        tgcl/test.cpp
        tgcl/tgfs_api.cpp

        tgfs/tgfs_main.cpp
        tgfs/tgfs_ops.cpp
        tgfs/tgfs_data.cpp
        tgfs/tgfs_dir.cpp
        tgfs/tgfs_helpers.cpp
        tgfs/tgfs_inode.cpp
        tgfs/tgfs_table.cpp
        parser/parser.cpp
)

set_target_properties(tg_cloud
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED true
        CXX_EXTENSIONS false
)

add_subdirectory(td)
target_link_libraries(tg_cloud PRIVATE Td::TdStatic)

add_subdirectory(SQLiteCpp)
target_link_libraries(tg_cloud PRIVATE SQLiteCpp)

target_link_libraries(tg_cloud PRIVATE -lfuse3 -lpthread)