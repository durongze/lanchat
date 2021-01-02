#!/bin/bash
function ParseTagFile()
{
    TagFile=$1
    TagList=grep "define" $TagFile | cut -d'"' -f2
}

ParseTagFile $1







