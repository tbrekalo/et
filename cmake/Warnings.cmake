function(set_default_warnings TARGET_NAME QUALIFIER AS_ERROR)
  set(WARNINGS
    -Wall
    -Wextra

    -Wshadow
    -Wunused

    -Wold-style-cast
    -Wcast-align

    -Wpedantic
    -Wconversion
  )

  if (AS_ERROR)
    set(WARNINGS ${WARNINGS} -Werror)
  endif()

  target_compile_options(${TARGET_NAME} ${QUALIFIER} ${WARNINGS})

endfunction()
