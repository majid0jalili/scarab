#add_compile_options(-Wno-missing-declarations)

file(GLOB srcs *.cpp *.h *.cc *.hh)
add_library(gem5_lib STATIC ${srcs})
target_compile_definitions(gem5_lib PRIVATE GEM5_LIB)
target_compile_options(gem5_lib PRIVATE ${gem5_warnings})