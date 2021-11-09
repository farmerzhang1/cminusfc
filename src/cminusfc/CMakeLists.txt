add_executable(
    cminusfc
    cminusfc.cpp
    cminusf_builder.cpp
    utils.cpp
    utils.hpp # can i do this?
)

target_link_libraries(
    cminusfc
    IR_lib
    common
    syntax
)

install(
    TARGETS cminusfc
    RUNTIME DESTINATION bin
)
