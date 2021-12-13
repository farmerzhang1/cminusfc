add_executable(
    ta_cminusfc
    cminusfc.cpp
    builder_ta.cpp
    # cminusf_builder.cpp
    # utils.cpp
    # utils.hpp
)

add_executable(
    cminusfc
    cminusfc.cpp
    # builder_ta.cpp
    cminusf_builder.cpp
    utils.cpp
    utils.hpp
)

target_link_libraries(
    cminusfc
    OP_lib
    IR_lib
    common
    syntax
)

target_link_libraries(
    ta_cminusfc
    OP_lib
    IR_lib
    common
    syntax
)

install(
    TARGETS cminusfc
    RUNTIME DESTINATION bin
)
