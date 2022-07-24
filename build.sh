#!/bin/bash

# get current directory-path and the path of the parent-directory
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PARENT_DIR="$(dirname "$DIR")"

# create build-directory
BUILD_DIR="$PARENT_DIR/build"
mkdir -p $BUILD_DIR

# create directory for the final result
RESULT_DIR="$PARENT_DIR/result"
mkdir -p $RESULT_DIR

#-----------------------------------------------------------------------------------------------------------------

function build_kitsune_lib_repo () {
    REPO_NAME=$1
    NUMBER_OF_THREADS=$2

    # create build directory for repo and go into this directory
    REPO_DIR="$BUILD_DIR/$REPO_NAME"
    mkdir -p $REPO_DIR
    cd $REPO_DIR

    # build repo library with qmake
    if [[ $REPO_NAME == "libKitsunemimiHanamiSdk" ]]
    then
        /usr/lib/x86_64-linux-gnu/qt5/bin/qmake "$PARENT_DIR/$REPO_NAME/cpp/$REPO_NAME.pro" -spec linux-g++ "CONFIG += optimize_full staticlib"
    else
        /usr/lib/x86_64-linux-gnu/qt5/bin/qmake "$PARENT_DIR/$REPO_NAME/$REPO_NAME.pro" -spec linux-g++ "CONFIG += optimize_full staticlib"
    fi
    /usr/bin/make -j$NUMBER_OF_THREADS

    # copy build-result and include-files into the result-directory
    echo "----------------------------------------------------------------------"
    echo $RESULT_DIR
    cp $REPO_DIR/src/$REPO_NAME.a $RESULT_DIR/
    if [[ $REPO_NAME == "libKitsunemimiHanamiSdk" ]]
    then
        cp -r $PARENT_DIR/$REPO_NAME/cpp/include $RESULT_DIR/
    else
        cp -r $PARENT_DIR/$REPO_NAME/include $RESULT_DIR/
    fi
    ls -l $RESULT_DIR/include/
    ls -l $RESULT_DIR
}

function get_required_kitsune_lib_repo () {
    REPO_NAME=$1
    TAG_OR_BRANCH=$2
    NUMBER_OF_THREADS=$3

    # clone repo
    git clone https://github.com/kitsudaiki/$REPO_NAME.git "$PARENT_DIR/$REPO_NAME"
    cd "$PARENT_DIR/$REPO_NAME"
    git checkout $TAG_OR_BRANCH

    build_kitsune_lib_repo $REPO_NAME $NUMBER_OF_THREADS
}

function get_required_private_repo_gitlab () {
    REPO_NAME=$1
    TAG_OR_BRANCH=$2
    TOKEN=$3
    NUMBER_OF_THREADS=$4

    # clone repo
    git clone http://kitsudaiki:$TOKEN@10.0.3.120/hanami/$REPO_NAME.git "$PARENT_DIR/$REPO_NAME"
    cd "$PARENT_DIR/$REPO_NAME"
    git checkout $TAG_OR_BRANCH

    build_kitsune_lib_repo $REPO_NAME $NUMBER_OF_THREADS
}

function get_required_private_repo_github () {
    REPO_NAME=$1
    TAG_OR_BRANCH=$2
    NUMBER_OF_THREADS=$3

    # clone repo
    git clone https://kitsudaiki:986ec116cd18aa45cfb81e57916518f6ff83bf19@github.com/kitsudaiki/$REPO_NAME.git "$PARENT_DIR/$REPO_NAME"
    cd "$PARENT_DIR/$REPO_NAME"
    git checkout $TAG_OR_BRANCH

    build_kitsune_lib_repo $REPO_NAME $NUMBER_OF_THREADS
}


#-----------------------------------------------------------------------------------------------------------------

echo "###########################################################################################################"
echo ""
get_required_kitsune_lib_repo "libKitsunemimiCommon" "v0.26.1" 8
get_required_kitsune_lib_repo "libKitsunemimiJson" "v0.11.3" 1
get_required_kitsune_lib_repo "libKitsunemimiJinja2" "v0.9.1" 1
get_required_kitsune_lib_repo "libKitsunemimiIni" "v0.5.1" 1
get_required_kitsune_lib_repo "libKitsunemimiNetwork" "v0.8.2" 8
get_required_kitsune_lib_repo "libKitsunemimiArgs" "v0.4.0" 8
get_required_kitsune_lib_repo "libKitsunemimiConfig" "v0.4.0" 8
get_required_kitsune_lib_repo "libKitsunemimiObj" "v0.2.0" 8
echo ""
echo "###########################################################################################################"
echo ""
get_required_private_repo_github "libKitsunemimiSqlite" "v0.3.0" 8
get_required_private_repo_github "libKitsunemimiCrypto" "v0.2.0" 8
get_required_private_repo_github "libKitsunemimiJwt" "v0.4.1" 8
echo ""
echo "###########################################################################################################"
echo ""
get_required_kitsune_lib_repo "libKitsunemimiSakuraNetwork" "v0.8.4" 8
get_required_kitsune_lib_repo "libKitsunemimiSakuraLang" "v0.12.0" 1
get_required_private_repo_github "libKitsunemimiSakuraDatabase" "v0.5.0" 8
echo ""
echo "###########################################################################################################"
echo ""
get_required_private_repo_gitlab "libKitsunemimiHanamiCommon" "master" "2ue6RNxkCDs2A7qp1xtN" 8
get_required_private_repo_gitlab "libKitsunemimiHanamiEndpoints" "v0.1.0" "ysR35grcGsLpFQiXXf1A" 1
get_required_private_repo_gitlab "libKitsunemimiHanamiDatabase" "master" "UjDbSSJzoU3uK4pZynmD" 8
get_required_private_repo_gitlab "libKitsunemimiHanamiMessaging" "master" "vkEae-QF8jvt9W2xz3LR" 8
get_required_private_repo_gitlab "libKitsunemimiHanamiSdk" "master" "bfzFyQfmS2mpkWxCeuHA" 8
echo ""
echo "###########################################################################################################"
echo ""
get_required_private_repo_gitlab "libSagiriArchive" "master" "glpat-dCkDLv96JmBscfZey4zJ" 8
get_required_private_repo_gitlab "libAzukiHeart" "master" "glpat-R3vZd81MXGVUTSxgHz1q" 8
get_required_private_repo_gitlab "libMisakaGuard" "v0.1.0" "glpat-sUgJE_tJH_jV19zL8X_x" 8
echo ""
echo "###########################################################################################################"

#-----------------------------------------------------------------------------------------------------------------

# create build directory for KyoukoMind and go into this directory
LIB_KITSUNE_SAKURA_TREE_DIR="$BUILD_DIR/KyoukoMind"
mkdir -p $LIB_KITSUNE_SAKURA_TREE_DIR
cd $LIB_KITSUNE_SAKURA_TREE_DIR

# build KyoukoMind library with qmake
if [ $1 = "test" ]; then
    /usr/lib/x86_64-linux-gnu/qt5/bin/qmake "$PARENT_DIR/KyoukoMind/KyoukoMind.pro" -spec linux-g++ "CONFIG += optimize_full run_tests"
    /usr/bin/make -j8
    # copy build-result and include-files into the result-directory
    cp "$LIB_KITSUNE_SAKURA_TREE_DIR/KyoukoMind_Test" "$RESULT_DIR/"
else
    /usr/lib/x86_64-linux-gnu/qt5/bin/qmake "$PARENT_DIR/KyoukoMind/KyoukoMind.pro" -spec linux-g++ "CONFIG += optimize_full"
    /usr/bin/make -j8
    # copy build-result and include-files into the result-directory
    cp "$LIB_KITSUNE_SAKURA_TREE_DIR/KyoukoMind" "$RESULT_DIR/"
fi

#-----------------------------------------------------------------------------------------------------------------

