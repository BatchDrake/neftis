#!/bin/bash

if [ $# != 1 ]; then
	echo "Forma de uso: $0 <archivo>"
	exit 1
fi

cat "$1" | grep '^[a-zA-Z0-9_]* (' | sed 's/ (.*/);/g' | sed 's/^/DEBUG_FUNC (/g'



