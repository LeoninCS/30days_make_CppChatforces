#加载所有源文件
aux_source_directory(./db DB_LIST)
aux_source_directory(. SRC_LIST)
aux_source_directory(./redis REDIS_LIST)
aux_source_directory(./model MODEL_LIST)
#可执行文件
add_executable(ChatServer ${DB_LIST} ${SRC_LIST} ${REDIS_LIST} ${MODEL_LIST})
#链接库文件
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient hiredis pthread)