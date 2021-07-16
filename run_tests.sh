#!/bin/bash
./compile.sh
cd build

folder=../sysyruntimelibrary/section1/functional_test

source_files=$(ls $folder)
for source_file in ${source_files}
do
    if [ "${source_file##*.}" == "sy" ]; then
        echo "**********running test ${source_file}***********"
        path="$folder/$source_file"
        ./ning $path -O
    #else
	#echo "${source_file%%*.}"
    fi

    #./ning $path
    #./ning "$folder:$source_file"
done
