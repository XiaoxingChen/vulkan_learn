function(WriteShaderBank SHADER_FOLDER SHADER_LIST INPUT_TEMPLATE OUTPUT_FILENAME)
    message("SHADER_LIST: " ${SHADER_LIST})
    set(STRENG_BYTES_LIMIT 5000)
    foreach(shader_filename ${SHADER_LIST})
        set(SHADER_CODE "")
        set(CODE_OFFSET 0)
        set(PART_SHADER_CODE " ")
        while(NOT PART_SHADER_CODE STREQUAL "") # Do this ugly string seperation because of MSVC C2026: string too big, limit of 16380 single-byte.
            # file(READ ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${shader_filename} PART_SHADER_CODE OFFSET ${CODE_OFFSET} LIMIT ${STRENG_BYTES_LIMIT} )
            file(READ ${SHADER_FOLDER}/${shader_filename} PART_SHADER_CODE OFFSET ${CODE_OFFSET} LIMIT ${STRENG_BYTES_LIMIT} )
            string(SUBSTRING "${PART_SHADER_CODE}" 0 ${STRENG_BYTES_LIMIT} PART_SHADER_CODE)
            set(SHADER_CODE "${SHADER_CODE} R\"(${PART_SHADER_CODE})\"")
            MATH(EXPR CODE_OFFSET "${CODE_OFFSET}+${STRENG_BYTES_LIMIT}")
        endwhile(NOT PART_SHADER_CODE STREQUAL "")

        set(SHADER_SRC_MAP_STRING "${SHADER_SRC_MAP_STRING}{\"${shader_filename}\", ${SHADER_CODE}},\n")
    endforeach()

    configure_file(${INPUT_TEMPLATE} ${OUTPUT_FILENAME})

endfunction()