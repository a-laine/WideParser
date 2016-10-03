cmake_minimum_required(VERSION 3.5)

# declaration du projet
project(WideParser)
set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})
set (CMAKE_CXX_STANDARD 11)



# generation de la liste des fichiers sources
file(
	GLOB_RECURSE
	sources_files
	src/*
)


# repetoires d'includes
include_directories(src/)



# declaration de l'executable
add_executable(
	parser
	${sources_files}
)


# configuration de l'edition de liens
target_link_libraries(
        parser
)