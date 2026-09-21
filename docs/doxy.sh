#!/bin/bash

# Please execute on repository root

## Get version from library.properties
## Get git rev of HEAD
LIB_VERSION="$(grep -E '^[[:space:]]*version[[:space:]]*=' library.properties | head -n1 | cut -d'=' -f2 | tr -d '[:space:]')"
#echo ${DOXYGEN_PROJECT_NUMBER}
GIT_REV="$(git rev-parse --short HEAD 2>/dev/null)"
DOXYGEN_PROJECT_NUMBER="${LIB_VERSION}${GIT_REV:+ git rev:${GIT_REV}}" doxygen docs/Doxyfile
