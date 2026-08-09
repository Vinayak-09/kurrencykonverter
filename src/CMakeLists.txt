add_library(kurrencykonverter MODULE
    currencyrunner.cpp
    ratesprovider.cpp
)

target_link_libraries(kurrencykonverter
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    KF6::Runner
    KF6::CoreAddons
    KF6::ConfigCore
)

# KDE/Qt plugins are conventionally named without the "lib" prefix (e.g.
# "calculator.so"); KPluginMetaData derives the plugin id from this filename,
# so keeping the default "lib" prefix here would make the derived id
# ("libkurrencykonverter") disagree with the "Id" in kurrencykonverter.json.
set_target_properties(kurrencykonverter PROPERTIES PREFIX "")

if(NOT DEFINED QT_PLUGIN_INSTALL_DIR)
    execute_process(
        COMMAND qmake6 -query QT_INSTALL_PLUGINS
        OUTPUT_VARIABLE QT_PLUGIN_INSTALL_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

install(TARGETS kurrencykonverter DESTINATION "${QT_PLUGIN_INSTALL_DIR}/kf6/krunner")
