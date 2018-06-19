#!/bin/bash

if ! type wget &>/dev/null;then
	echo "Command WGET doesn't exist. If you want to download files, install it!"
	exit 1
fi

if [ $# -ne 2 ];then
	echo "Incorrect number of parameters"
	exit 1
fi

if [ ! -d $1 ];then
	echo "$1 is not a directory"
	exit 2
fi

URL="$2"
DIRECTORY="$1"

LAST_CHAR="${DIRECTORY: -1}"
if [ "${LAST_CHAR}" = '/' ];then 
	DIRECTORY=${DIRECTORY::-1}
fi

mkdir -p "${DIRECTORY}/bfra.bg"

reports=$(wget -qO- "${URL}" | sed -n "/<body>/,/<\/body>/p" | egrep "href" | sed "s/<\/*[^>]*>//g" | awk -F' ' '{print $1}')

if [ $? -ne 0 ];then
	echo "Cannot download files from ${URL}"
	exit 3
fi

while read RACE; do
	wget -qO "${DIRECTORY}/bfra.bg/${RACE}" "${URL}${RACE}"
	sed -i "/${RACE}/!d" "${DIRECTORY}/bfra.bg/${RACE}"
	sed -i "/CALLSIGN:/d" "${DIRECTORY}/bfra.bg/${RACE}"
done < <(echo "${reports}")

