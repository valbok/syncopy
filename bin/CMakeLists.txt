add_executable(signature signature.cpp)
target_link_libraries(signature ${PROJECT_NAME})

add_executable(delta delta.cpp)
target_link_libraries(delta ${PROJECT_NAME})

add_executable(patch patch.cpp)
target_link_libraries(patch ${PROJECT_NAME})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
add_executable(server server.cpp)
target_link_libraries(server PRIVATE ${PROJECT_NAME} rpc Threads::Threads)

add_executable(client client.cpp)
target_link_libraries(client PRIVATE ${PROJECT_NAME} rpc Threads::Threads)
