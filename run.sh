#! /bin/bash

rm -f log_file.*
rm -f worker.*
rm -f master.*

if [ "$1" == "s" ]
    then

    if [ "${10}" == "v" ]
        then
        make
        valgrind -v --trace-children=yes --show-leak-kinds=all --leak-check=full  --fair-sched=yes --track-origins=yes ./whoServer ${*:2}
    elif [ "${10}" == "g" ]
        then
        make
        gdb --args ./whoServer ${*:2}

    else
        make
        ./whoServer ${*:2}
        # killall worker
    fi


elif [ "$1" == "c" ]
    then


    if [ "${10}" == "v" ]
        then
        make
        valgrind -v --trace-children=yes --show-leak-kinds=all --leak-check=full  --fair-sched=yes --track-origins=yes ./whoClient ${*:2}
    elif [ "${10}" == "g" ]
        then
        make
        gdb --args ./whoClient ${*:2}

    else
        make
        ./whoClient ${*:2}
        # killall worker
    fi


elif [ "$1" == "m" ]
    then

    if [ "${12}" == "v" ]
        then
        make
        valgrind -v --trace-children=yes --show-leak-kinds=all --leak-check=full  --fair-sched=yes --track-origins=yes ./master ${*:2}
    elif [ "${12}" == "g" ]
        then
        make
        gdb --args ./master ${*:2}

    else
        make
        ./master ${*:2}
        # killall worker
    fi



fi

