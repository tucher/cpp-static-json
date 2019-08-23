TEMPLATE = app
CONFIG += console c++1z
CONFIG -= app_bundle
CONFIG -= qt


SOURCES = main.cpp
#SOURCES = trie_test.cpp



#CONFIG += force_debug_info


HEADERS += \
    static_json.hpp \
    trie.hpp \
    utils.h \
    ../template-tools/typestring/typestring.hpp \
    ../template-tools/typecalc/typecalc.hpp \
    ../template-tools/trie/trie.hpp

QMAKE_CXXFLAGS += "-Wno-gnu-string-literal-operator-template"
#QMAKE_LFLAGS += "--strip-all"

DISTFILES += \
    ../template-tools/trie/README.md
