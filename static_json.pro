TEMPLATE = app
CONFIG += console c++1z
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp \
        trie_test.cpp
CONFIG += force_debug_info

HEADERS += \
    static_json.hpp \
    trie.hpp \
    utils.h

QMAKE_CXXFLAGS += "-Wno-gnu-string-literal-operator-template"
