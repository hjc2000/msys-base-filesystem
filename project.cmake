# 编译目标
add_library(${ProjectName} STATIC)
target_import_src(${ProjectName})
target_import_base(${ProjectName} PUBLIC)


# 添加测试程序
if(("${platform}" STREQUAL "msys") OR
   ("${platform}" STREQUAL "msys-clang"))
	set(exe_target_name "${ProjectName}-exe")
	add_executable(${exe_target_name})
	target_import_test(${exe_target_name})

	target_import_base(${exe_target_name} PUBLIC)
	target_link_libraries(${exe_target_name} PUBLIC "${ProjectName}")
endif()
