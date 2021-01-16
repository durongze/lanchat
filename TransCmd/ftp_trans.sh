#!/bin/bash

IP="127.0.0.1"
Port="21"
FtpUser=""
FtpPwd=""
FtpDir="/ftphome/downloadData"
LocDir="/local/getDownloadData"

function FtpDown()
{
ftp -v -n ${IP}:${Port}<<EOF
    user $FtpUser $FtpPwd
    binary
    cd $FtpDir
    lcd $LocDir
    prompt
    mget *
    bye
EOF
}

FtpDown
