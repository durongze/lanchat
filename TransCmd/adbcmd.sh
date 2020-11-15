#!/bin/bash

PushClient="./cli"

function InputWords()
{
    declare -A sermap=(["1"]="A"
                       ["2"]="B"
                       ["3"]="C"
                       ["4"]="D"
                       ["5"]="E"
                       ["6"]="F"
                       ["7"]="G"
                       ["8"]="H"
                       ["9"]="I"
                       ["A"]="J"
                       ["B"]="K")
                       #["\\"]="/")
    for k in ${!sermap[@]}
    do 
        v=${sermap[$k]}
        keycode="KEYCODE_$v"
        echo "adb shell input keyevent $keycode"
        if [[ ! -z "$PushClient" ]];then
            $PushClient "\"adb shell input keyevent $keycode \""
        fi
    done
}

function PressPower()
{
    $PushClient "\"adb shell input keyevent KEYCODE_224 \""
}

function UnlockScreen()
{
    local password=$1
    $PushClient "\"adb shell input text $password \""
}

function GetEvent()
{
    $PushClient "\"adb shell getevent -p\""
}

function SwipeScreen()
{
    x1=$1
    y1=$2
    x2=$3
    y2=$4
    $PushClient "\"adb shell input swipe $x1 $y1 $x2 $y2\""
    $PushClient "\"adb shell input swipe $x2 $y2 $x1 $y1\""
}

PressPower
#InputWords
SwipeScreen "100" "100" "10" "10"
