function(serializeToByteString RESOURCE_FOLDER RESOURCE_LIST OUT_BYTE_STR OUT_MAP_STR)
    set(BYTE_STR "")
    set(MAP_STR "")
    foreach(shader_filename ${RESOURCE_LIST})
        message(${shader_filename})
        file(READ ${RESOURCE_FOLDER}/${shader_filename} SHADER_CODE HEX )
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," SHADER_CODE ${SHADER_CODE} )
        string(REGEX REPLACE "[/\\.]" "_" C_VAR_NAME ${RESOURCE_FOLDER}/${shader_filename} )
        message(${C_VAR_NAME})
        set(BYTE_STR "${BYTE_STR}const char ${C_VAR_NAME}[] = {${SHADER_CODE}0x00};\n")
        set(MAP_STR "${MAP_STR}{\"${shader_filename}\", ${C_VAR_NAME}},\n")
    endforeach()

    # message(${BYTE_STR})
    set(${OUT_BYTE_STR} ${BYTE_STR} PARENT_SCOPE)
    set(${OUT_MAP_STR} ${MAP_STR} PARENT_SCOPE)

endfunction(serializeToByteString)

function(generateShaderManagerCode RESOURCE_FOLDER RESOURCE_LIST OUT_FILENAME IS_STATIC)
    set(RESOURCE_MANAGER_TEMPLATE "invalid")
    if(IS_STATIC)
        set(SHADER_FOLDER_STR ${RESOURCE_FOLDER})
        configure_file (${STATIC_RESOURCE_MANAGER_TEMPLATE} ${OUT_FILENAME})
    elseif(NOT IS_STATIC)
        serializeToByteString(
            ${RESOURCE_FOLDER}
            "${RESOURCE_LIST}"
            SHADER_HEX_STRING
            SHADER_SRC_MAP_STRING)
        configure_file (${BINARIZE_RESOURCE_MANAGER_TEMPLATE} ${OUT_FILENAME})
    endif()

endfunction(generateShaderManagerCode)

