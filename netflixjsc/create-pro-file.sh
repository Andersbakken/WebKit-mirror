#!/bin/sh

cat netflixjsc.pro.base > netflixjsc.pro
echo "SOURCES = \\" >> netflixjsc.pro
find ../Source/JavaScriptCore -name "*.cc" -or -name "*.cpp" -or -name "*.c" \
    | grep -v "[^A-Z]Win[^a-z]" \
    | grep -v "[^A-Z]Efl[^a-z]" \
    | grep -v "[^A-Z]BlackBerry[^a-z]" \
    | grep -v "[^A-Z]G[lL]ib[^a-z]" \
    | grep -v "[^A-Z]Wx[^a-z]" \
    | grep -v "[^A-Z]Qt[^a-z]" \
    | grep -v "[^A-Z]CF[^a-z]" \
    | grep -v "[^A-Z]BSTR[^a-z]" \
    | grep -v "[^A-Z]Gtk[^a-z]" \
    | grep -v "/tests/" \
    | grep -v "/wince/" \
    | grep -v "/chromium/" \
    | grep -v "/jsc.cpp" \
    | while read i; do echo "          \$\$PWD/$i \\" >> netflixjsc.pro; done
