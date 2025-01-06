# Helper function to deploy Qt dependencies
function(deploy_qt_runtime)
    # Find Qt deployment tool
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    
    find_program(LINUXDEPLOYQT linuxdeployqt HINTS "${_qt_bin_dir}")
    
    if(LINUXDEPLOYQT)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${LINUXDEPLOYQT} $<TARGET_FILE:${PROJECT_NAME}>
                -always-overwrite
                -appimage
            COMMENT "Deploying Qt dependencies..."
        )
    endif()
endfunction()

# Call the function
deploy_qt_runtime()
