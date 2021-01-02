#!/bin/bash
function ParseTagFile()
{
    TagFile=$1
    TagList=grep "define" $TagFile | cut -d'"' -f2
    echo $TagList | tr ' ' '\n' | sort -su
    for tag in $TagList
    do
        echo $tag
    done
}

ParseTagFile $1







