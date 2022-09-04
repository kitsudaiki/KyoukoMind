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
    if [[ $REPO_NAME == "libKitsumiAiSdk" ]]
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
    if [[ $REPO_NAME == "libKitsumiAiSdk" ]]
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

function get_required_private_repo_github () {
    REPO_NAME=$1
    TAG_OR_BRANCH=$2
    NUMBER_OF_THREADS=$3

    # clone repo
    git clone https://kitsudaiki:$CLONE_TOKEN@github.com/kitsudaiki/$REPO_NAME.git "$PARENT_DIR/$REPO_NAME"
    cd "$PARENT_DIR/$REPO_NAME"
    git checkout $TAG_OR_BRANCH

    build_kitsune_lib_repo $REPO_NAME $NUMBER_OF_THREADS
}

function download_private_repo_github () {
    REPO_NAME=$1
    TAG_OR_BRANCH=$2

    # clone repo
    git clone https://kitsudaiki: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@github.com/kitsudaiki/$REPO_NAME.git "$BUILD_DIR/$REPO_NAME"
    git clone https://kitsudaiki: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@github.com/kitsudaiki/$REPO_NAME.git "$PARENT_DIR/$REPO_NAME"
    cd "$BUILD_DIR/$REPO_NAME"
    git checkout $TAG_OR_BRANCH
    cd "$PARENT_DIR/$REPO_NAME"
    git checkout $TAG_OR_BRANCH
}

#-----------------------------------------------------------------------------------------------------------------

echo "###########################################################################################################"
echo ""
get_required_kitsune_lib_repo "libKitsunemimiCommon" "v0.27.0" 8
get_required_kitsune_lib_repo "libKitsunemimiJson" "develop" 1
get_required_kitsune_lib_repo "libKitsunemimiJinja2" "develop" 1
get_required_kitsune_lib_repo "libKitsunemimiIni" "develop" 1
get_required_kitsune_lib_repo "libKitsunemimiNetwork" "develop" 8
get_required_kitsune_lib_repo "libKitsunemimiArgs" "develop" 8
get_required_kitsune_lib_repo "libKitsunemimiConfig" "develop" 8
get_required_kitsune_lib_repo "libKitsunemimiObj" "develop" 8
echo ""
echo "###########################################################################################################"
echo ""
get_required_private_repo_github "libKitsunemimiSqlite" "develop" 8
get_required_private_repo_github "libKitsunemimiCrypto" "develop" 8
get_required_private_repo_github "libKitsunemimiJwt" "develop" 8
echo ""
echo "###########################################################################################################"
echo ""
get_required_kitsune_lib_repo "libKitsunemimiSakuraNetwork" "develop" 8
get_required_kitsune_lib_repo "libKitsunemimiSakuraLang" "develop" 1
get_required_private_repo_github "libKitsunemimiSakuraDatabase" "develop" 8
echo ""
echo "###########################################################################################################"
echo ""
get_required_private_repo_github "libKitsunemimiHanamiCommon" "develop" 8
get_required_private_repo_github "libKitsunemimiHanamiEndpoints" "develop" 1
get_required_private_repo_github "libKitsunemimiHanamiDatabase" "develop" 8
get_required_private_repo_github "libKitsunemimiHanamiNetwork" "develop" 8
download_private_repo_github "libKitsunemimiHanamiMessages" "develop"
get_required_private_repo_github "libKitsumiAiSdk" "develop" 8
echo ""
echo "###########################################################################################################"
echo ""
get_required_private_repo_github "libAzukiHeart" "develop" 8
get_required_private_repo_github "libMisakiGuard" "develop" 8
get_required_private_repo_github "libSagiriArchive" "develop" 8
echo ""
echo "###########################################################################################################"

#-----------------------------------------------------------------------------------------------------------------

# create build directory for KyoukoMind and go into this directory
LIB_KITSUNE_SAKURA_TREE_DIR="$BUILD_DIR/KyoukoMind"
mkdir -p $LIB_KITSUNE_SAKURA_TREE_DIR
cd $LIB_KITSUNE_SAKURA_TREE_DIR

# build KyoukoMind library with qmake
/usr/lib/x86_64-linux-gnu/qt5/bin/qmake "$PARENT_DIR/KyoukoMind/KyoukoMind.pro" -spec linux-g++ "CONFIG += optimize_full"
/usr/bin/make -j8
# copy build-result and include-files into the result-directory
cp "$LIB_KITSUNE_SAKURA_TREE_DIR/KyoukoMind" "$RESULT_DIR/"

#-----------------------------------------------------------------------------------------------------------------

