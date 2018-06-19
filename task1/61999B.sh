#!/bin/bash

function decoding {
	if [[ "${1}" == "A" ]]; then
		cat encrypted
		exit 0
	fi

	INDEX=$(grep -n "${1}" morse| cut -d: -f1 )
	LETTER="$(grep -on "[[:alpha:]]" morse | grep "^${INDEX}:" | cut -d: -f2 | tr A-Z a-z)"
	INDEX=$((INDEX-1))
	LETTER_BEFORE="$(grep -on "[[:alpha:]]" morse | grep "^${INDEX}:" | cut -d: -f2 | tr A-Z a-z)"
	tr [a-z] ["${LETTER}"-za-"${LETTER_BEFORE}"] < encrypted
}

function key {

COUNT=$(grep "fuehrer" encrypted | wc -l)	
if [[ "${COUNT}" -gt 0 ]]; then
	echo "A"
	exit 1
fi

LETTER_BEFORE="a"	

for LETTER in {b..z}; do
	RESULT=$(echo "fuehrer" | tr ["${LETTER}"-za-"${LETTER_BEFORE}"] [a-z])

	COUNT=$(grep "${RESULT}" encrypted | wc -l)	
	if [[ "${COUNT}" -gt 0 ]]; then
		echo "${LETTER}" | tr a-z A-Z
	break
	fi

	LETTER_BEFORE="${LETTER}"
done
}

if [[ -f encrypted && -f morse ]]; then
	decoding $(key)
else
	exit 1
fi
