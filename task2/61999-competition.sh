#!/bin/bash

if [ $# -ne 2 ];then
	echo "Incorrect number of parameters"
	exit 1
fi

if [ ! -d $1 ];then
	echo "Directory $1 doesn't exist"
	exit 2
fi

DIRECTORY=$1

LAST_CHAR="${DIRECTORY: -1}"
if [ "${LAST_CHAR}" = '/' ];then
        DIRECTORY=${DIRECTORY::-1}
fi

DIRECTORY="${DIRECTORY}/bfra.bg"
if [ ! -d "${DIRECTORY}" -o -z "$(ls -A "${DIRECTORY}")" ];then
	echo "Empty directory"
	exit 3
fi
 
function participants {
	find "${DIRECTORY}" -maxdepth 1 -mindepth 1 -type f -not -name ".*" -printf "%f\n"
}

function outliers {
	ALL_PARTICIPANTS=$(participants)
	OTHERS=$(find "${DIRECTORY}" -maxdepth 1 -mindepth 1 -type f -not -name ".*" 2>/dev/null -exec cat {} \; | awk -F' ' '{print $9}' | sort | uniq)
	grep -vhFxf <( echo "${ALL_PARTICIPANTS}" ) <( echo "${OTHERS}" )
}

function unique {
	OTHERS=$(find "${DIRECTORY}" -maxdepth 1 -mindepth 1 -type f -not -name ".*" 2>/dev/null -exec cat {} \; | awk -F' ' '{print $9}' | sort | uniq)
	
	while read CURR_PARTICIPANT;do
		FILES_MATCHED=$(egrep -rlw "${CURR_PARTICIPANT}" "${DIRECTORY}")
		COUNTER=$(echo "${FILES_MATCHED}" | wc -l)
		if [ "${COUNTER}" -le 3 ];then
			echo "${CURR_PARTICIPANT}" 
		fi
	done < <( echo "${OTHERS}")
}

function cross_check { 
  while read CURR;do
    NAME=$(echo "${CURR}" | awk -F' ' '{print $6}')
    DATE=$(echo "${CURR}" | awk -F' ' '{print $4}')
    TIME=$(echo "${CURR}" | awk -F' ' '{print $5}')
    CHECK=$(echo "${CURR}" | awk -F' ' '{print $9}')

    if [ -f "${DIRECTORY}"/"${CHECK}" ];then
				  cat "${DIRECTORY}"/"${CHECK}" | egrep "${DATE}" | egrep "${TIME}" | egrep -q "\<${NAME}\>"
          if [ $? -ne 0 ];then
              echo "${CURR}"
          fi
      else
          echo "${CURR}"
    fi
  done < <(find "${DIRECTORY}" -maxdepth 1 -mindepth 1 -type f -not -name ".*"  -exec cat {} \;)
}

function difference {
	
	H1=$(date -d "$1" +%-H)
	H2=$(date -d "$2" +%-H)
	M1=$(date -d "$1" +%-M)
	M2=$(date -d "$2" +%-M)

	TOTAL_MIN_FIRST="$((H1*60+M1))"
	TOTAL_MIN_SECOND="$((H2*60+M2))"

	DIFF=$((TOTAL_MIN_FIRST - TOTAL_MIN_SECOND))
	echo "${DIFF#-}"
}

function bonus {
  while read CURR;do
    NAME=$(echo "${CURR}" | awk -F' ' '{print $6}')
    DATE=$(echo "${CURR}" | awk -F' ' '{print $4}')
    TIME=$(echo "${CURR}" | awk -F' ' '{print $5}')
    CHECK=$(echo "${CURR}" | awk -F' ' '{print $9}')

    if [ -f "${DIRECTORY}"/"${CHECK}" ];then
				  cat "${DIRECTORY}"/"${CHECK}" | egrep "${DATE}" | egrep -q "\<${NAME}\>"
          if [ $? -ne 0 ];then
                  echo "${CURR}"
          else
                  I_TIME=$( cat "${DIRECTORY}"/"${CHECK}" | egrep "\<${NAME}\>" | egrep "${DATE}"| awk -F' ' '{print $5}' | sort -n | uniq | head -n1)
                  
                  echo "${TIME}${I_TIME}" | grep -q  "\<[0-9]*\>"; 
                  if [ "$?" -ne 0 ];then
                    break
                  fi

                  if [ "${TIME}" = "${I_TIME}" ];then
                    DIFF_TIME=0
                  else
                    DIFF_TIME=$(difference ${TIME} ${I_TIME})
                  fi
          
						      if [ ${DIFF_TIME} -gt 3 ];then
                    echo "${CURR}"
						      fi
          fi
      else
          echo "${CURR}"
    fi
  done < <(find "${DIRECTORY}" -maxdepth 1 -mindepth 1 -type f -not -name ".*"  -exec cat {} \;)
}

case $2 in
   "participants" )
     participants 
   ;;
   "outliers" )
     outliers
   ;;
   "unique" )
     unique
   ;;
   "cross_check" )
     cross_check
   ;;
   "bonus" )
     bonus
   ;;
   * )
     echo "Incorrect function name"
     exit 3
esac
