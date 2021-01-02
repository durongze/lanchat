#!/bin/bash
function ParseTagFile()
{
    TagFile=$1
    TagList=grep "define" $TagFile | cut -d'"' -f2
    echo $TagList | tr ' ' '\n' | sort -su
    TagFilter=""
    idx=0
    for tag in $TagList
    do
        idx=$(expr $(idx + 1))
        mod=$(expr $(idx % 5))
        if [[ $mod -eq 0 ]];then
            
            echo $tag
        fi
    done
}

ParseTagFile $1

function ParseProperty()
{
    PropFile=$1
    PropList=$(grep "etProperty" $PropFile|cut -d'(' -f3|cut -d')' -f1|cut -d',' -f1|sort -su)

}



