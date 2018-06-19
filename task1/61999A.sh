#!/bin/bash

if [[ ! -f morse && ! -f secret_message ]]; then
	exit 1
fi

for WORD in `cat secret_message`;  do
	LETTER=$(awk -v w="$WORD" -F' ' '$2==w {print $1}' morse | tr A-Z a-z)
	echo -n "${LETTER}" 
done > encrypted
echo >> encrypted	
