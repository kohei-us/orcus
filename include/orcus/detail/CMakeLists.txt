
list(APPEND _HEADERS parser_token_buffer.hpp thread.hpp)

install(FILES ${_HEADERS}
    DESTINATION include/orcus-${ORCUS_API_VERSION}/orcus/detail
)
