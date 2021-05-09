#add_compile_options(-Wno-missing-declarations)

file(GLOB srcs *.cpp *.h *.cc)
add_library(gem5 STATIC ${srcs})
target_compile_definitions(gem5 PRIVATE GEM5)
target_compile_options(gem5 PRIVATE ${gem5_warnings})