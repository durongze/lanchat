#!/bin/bash

PushClient="adbself"

function InputWords()
{
    declare -A sermap=(["A"]="*"
                       ["B"]="#"
                       ["C"]="*"
                       ["D"]="#"
                       ["E"]="7"
                       ["F"]="7"
                       ["G"]="7"
                       ["H"]="7"
                       ["I"]="#"
                       ["J"]="*"
                       ["K"]="#"
                       ["L"]="*"  )
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

function DialNumber()
{
	$PushClient "\"adb shell service call phone 1 s16 %2A%23%2A%237777%23%2A%23%2A \""	
}

function GetUiCtx()
{
	$PushClient "\"adb shell uiautomator dump /sdcard/ui.xml\""
}

function PressPower()
{
    $PushClient "\"adb shell input keyevent KEYCODE_POWER \""
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
    #$PushClient "\"adb shell input swipe $x2 $y2 $x1 $y1\""
}

function OpenViewSvr()
{
	$PushClient "\"adb shell service call window 1 i32 4939\""	
}

function CloseViewSvr()
{
	$PushClient "\"adb shell service call window 2 i32 4939\""	
}

function GetWmSizeW()
{
	local WinW=$($PushClient "\"adb shell wm size\"")	
	local WinW=$(echo $WinW | grep "msg" | cut -f4 -d' ' | cut -f1 -d'x')
	echo "$WinW"
}

function GetWmSizeH()
{
	local WinH=$($PushClient "\"adb shell wm size\"")	
	local WinH=$(echo $WinH | grep "msg" | cut -f4 -d' ' | cut -f2 -d'x')
	echo "$WinH"
}

function GetEventPosW()
{
	local PosW=$($PushClient "\"adb shell getevent -p | grep -e \"0035\" \"")
	local PosW=$(echo $PosW | grep "msg" | cut -f10 -d' ' | cut -f1 -d',')
	echo "$PosW"
}

function GetEventPosH()
{
	local PosH=$($PushClient "\"adb shell getevent -p | grep -e \"0036\" \"")	
	local PosH=$(echo $PosH | grep "msg" | cut -f10 -d' ' | cut -f1 -d',')
	echo "$PosH"
}

function PageDown()
{
	#$PushClient "\"adb shell input keyevent KEYCODE_PAGE_DOWN \""	
	WmW=$(GetWmSizeW)
	WmH=$(GetWmSizeH)
	HalfWmW=$(expr $WmW / 2)
	HalfWmH=$(expr $WmH / 2)
	QWmW=$(expr $WmW / 4)
	QWmH=$(expr $WmH / 4)
	SwipeScreen "$HalfWmW" "$HalfWmH" "$QWmW" "$QWmW"
}

function PageUp()
{
	#$PushClient "\"adb shell input keyevent KEYCODE_PAGE_UP \""	
	WmW=$(GetWmSizeW)
	WmH=$(GetWmSizeH)
	HalfWmW=$(expr $WmW / 2)
	HalfWmH=$(expr $WmH / 2)
	QWmW=$(expr $WmW / 6)
	QWmH=$(expr $WmH / 6)
	SwipeScreen "$QWmW" "$QWmH"  "$HalfWmW" "$HalfWmH"
}

function GetScreenStat()
{
	local ScreenStat=$($PushClient "\"adb shell dumpsys power | find \"Display Power: state=\"\"")
	local ScreenStat=$(echo $ScreenStat | grep "msg" | cut -f4 -d' ' | cut -f2 -d'=')
	echo $ScreenStat
}

function ProcProps()
{
    cmd="$1"
    props="$2"
    val="$3"  
    if [[ $cmd == "1" ]];then
        cmd="setprop"
    else
        cmd="getprop"
        val=""
    fi
    for prop in $props
    do
        echo $PushClient "\"adb shell $cmd $prop $val\""
        $PushClient "\"adb shell $cmd $prop $val\""
    done
}

Props="debug.mdpcomp.logs persist.camera.debug.logfile persist.data.qmi.adb_logmask"
Val="0"

ProcProps "1" "$Props" "$Val"

WmW=$(GetWmSizeW)
WmH=$(GetWmSizeH)
EvtPosW=$(GetEventPosW)
EvtPosH=$(GetEventPosH)
HalfWmW=$(expr $WmW / 2)
HalfWmH=$(expr $WmH / 2)

ScreenStat=$(GetScreenStat)
if [[ "$ScreenStat" == "OFF" ]];then
	PressPower
fi

echo "$HalfWmW" "$HalfWmH" "1" "1"
SwipeScreen "$HalfWmW" "$HalfWmH" "1" "1"
#InputDevelop
DialNumber
sleep 1
PageUp
PageUp
PageDown
GetUiCtx

