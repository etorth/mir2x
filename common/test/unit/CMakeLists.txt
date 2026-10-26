mir2x_add_unit_test(
    NAME arbvar
    SOURCES test_arbvar.cpp
    LIBS common
)

mir2x_add_unit_test(
    NAME itemquality
    SOURCES test_itemquality.cpp
    LIBS common
)

mir2x_add_unit_test(
    NAME servermsg
    SOURCES test_servermsg.cpp
    LIBS common
)

mir2x_add_unit_test(
    NAME hexstr
    SOURCES test_hexstr.cpp
    LIBS common
)
