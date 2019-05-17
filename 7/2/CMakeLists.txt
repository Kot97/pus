add_executable(queuemod queuemod.c libqueue.c)
target_link_libraries(queuemod -lnetfilter_queue)