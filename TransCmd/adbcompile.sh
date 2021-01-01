#!/bin/bash

#PDNAME_EX_A_0.5.0_viv_mtk.xml
XML_FILE=""
PRODUCT_NAME=""
VER=""
REP_DIR=""
LOC_DIR=""
PROJ_LINE=""
COMMIT_ID=""

function ParseXmlFileName()
{
	local SOFT_PACKET_NAME=$1
	XML_FILE=$SOFT_PACKET_NAME

	if [[ -z $SOFT_PACKET_NAME ]];then
		return 1;
    fi
	SOFT_PACKET_NAME=${SOFT_PACKET_NAME##*/}
	PRODUCT_NAME=${SOFT_PACKET_NAME%%.*} #
	PRODUCT_NAME=${PRODUCT_NAME%_*}
	PRODUCT_NAME=${PRODUCT_NAME%_*}
	
	VER=${SOFT_PACKET_NAME#*_A} #
	VER=${VER#*_}
	VER=${VER%%_*}

	return 0;
	
}

function ParseXmlCtx()
{
	local XML_FILE=$1
	local REP_DIR=$2

	local PROJ_LINE=$(cat $XML_FILE | grep "$REP_DIR")
	PROJ_LINE=$(echo $PROJ_LINE | cut -f2 -d'<')
	PROJ_LINE=${PROJ_LINE%/>*} #
	echo "$PROJ_LINE"
}

function ParseValue()
{
	local PROJ_LINE=$1
	local VALUE_IDX=$2
	local VALUE=$(echo $PROJ_LINE | cut -f$VALUE_IDX -d' ' | cut -f2 -d'"')
	echo "${VALUE}"	
}

function ParseCommitId()
{
	local PROJ_LINE=$1
	local COMMIT_ID=$(ParseValue "$PROJ_LINE" "4")
	echo "$COMMIT_ID"	
}

function ParseBranch()
{
	local PROJ_LINE=$1
	local BRANCH_NAME=$(ParseValue "$PROJ_LINE" "5")
	echo "$BRANCH_NAME"	
}

function GetRepDir()
{
	local CUR_DIR=$1
	local filter="android_"
	result=$(echo $CUR_DIR | grep "${filter}")
	if [[ "$result" != "" ]];then
		CUR_DIR=${CUR_DIR#*_}
	fi
	REP_DIR=$(echo $CUR_DIR | tr -s "_" "/")
}

function GetLocCommitId()
{
	local LOC_DIR=$1
	pushd $LOC_DIR >>/dev/null
	local COMMIT_ID=$(git show | grep -E "^commit " | cut -f2 -d' ')
	popd >>/dev/null
	echo "$COMMIT_ID"
}

function GetLocBranchName()
{
	local LOC_DIR=$1
	pushd $LOC_DIR >>/dev/null
	local BRANCH_NAME=$(git show -s --pretty=%d HEAD | cut -f2 -d',' | cut -f1 -d')')
	popd >>/dev/null
	echo "${BRANCH_NAME#*/}"
}

function ShowCfgInfo()
{
	echo "	$XML_FILE"
	echo "	$PRODUCT_NAME"
	echo "	$VER"
	echo "	$REP_DIR"
	echo "	$LOC_DIR"
	PROJ_LINE=$(ParseXmlCtx "$XML_FILE" "$REP_DIR")
	echo "	$PROJ_LINE"
	COMMIT_ID=$(ParseCommitId "$PROJ_LINE")
	echo "	$COMMIT_ID"
	BRANCH_NAME=$(ParseBranch "$PROJ_LINE")
	echo "	$BRANCH_NAME"
	echo "	vmake -v \"eng\" -m \"$XML_FILE\" -p \"$PRODUCT_NAME\" -b \"mmma $REP_DIR  -j32 --rebuild\" -c \"no\" -o \"vendor/bin vendor/lib/ vendor/lib64/ system/lib/ system/lib64/\" -u \"no\" -w \"${WORK_DIR}/$LOC_DIR\" "
}

function CheckLVerAndRVer()
{
	local LVER="$1"
	local RVER="$2"
	if [[ $LVER == $RVER ]];then
		echo -e "	\033[32mLocal : $LVER == Remote : $RVER\033[0m"
	else
		echo -e "	\033[31mLocal : $LVER != Remote : $RVER\033[0m"
	fi
}
function GenCmdByXmlDir()
{
	local LOC_DIR="$1"
	local XML_DIR="$2"
	local ALL_XML=$(find ${XML_DIR} -iname "*viv*.xml")
	local filter="mod"
	GetRepDir "${LOC_DIR}"
	for xmlFile in $ALL_XML
	do
		result=$(echo $xmlFile | grep "${filter}")
		if [[ "$result" == "" ]];then
			echo XmlFile:$xmlFile
			LocCommitId=$(GetLocCommitId "$LOC_DIR")
			echo LocCommitId:$LocCommitId
			LocBranchName=$(GetLocBranchName "$LOC_DIR")
			echo LocBranchName:$LocBranchName
			ParseXmlFileName "$xmlFile"
			ShowCfgInfo
			CheckLVerAndRVer "$LocBranchName" "$BRANCH_NAME" 
			CheckLVerAndRVer "$LocCommitId" "$COMMIT_ID" 
			echo "############################################################"
		fi
	done
}
WORK_DIR=$(pwd)
GenCmdByXmlDir "$(ls)" "${WORK_DIR}/../cfg"


