#!/bin/bash

# get current directory-path and the path of the parent-directory
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PARENT_DIR="$(dirname "$DIR")"
CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)

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
    ADDITIONAL_CONFIGS=$3

    # create build directory for repo and go into this directory
    REPO_DIR="$BUILD_DIR/$REPO_NAME"
    mkdir -p $REPO_DIR
    cd $REPO_DIR

    # build repo library with qmake
    /usr/lib/x86_64-linux-gnu/qt5/bin/qmake "$PARENT_DIR/$REPO_NAME/$REPO_NAME.pro" -spec linux-g++ "CONFIG += optimize_full staticlib $ADDITIONAL_CONFIGS"
    /usr/bin/make -j$NUMBER_OF_THREADS

    # copy build-result and include-files into the result-directory
    echo "----------------------------------------------------------------------"
    echo $RESULT_DIR
    cp $REPO_DIR/src/$REPO_NAME.a $RESULT_DIR/
    cp -r $PARENT_DIR/$REPO_NAME/include $RESULT_DIR/
    ls -l $RESULT_DIR/include/
    ls -l $RESULT_DIR
}

function download_repo_github () {
    REPO_NAME=$1
    TAG_OR_BRANCH=$2

    echo ""
    echo ""
    echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
    echo "$REPO_NAME"
    echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
    echo "Branch/Tag: $TAG_OR_BRANCH"

    # clone repo
    git clone https://github.com/kitsudaiki/$REPO_NAME.git "$PARENT_DIR/$REPO_NAME"
    cd "$PARENT_DIR/$REPO_NAME"

    # checkout branch
    if [[ $CURRENT_BRANCH =~ ^tag.* ]] || [[ $CURRENT_BRANCH =~ ^hotfix.* ]] || [[ $CURRENT_BRANCH =~ ^v.* ]] || [[ $CURRENT_BRANCH =~ ^rolling$ ]] || [[ $CURRENT_BRANCH =~ ^staging$ ]]; then
        # if a stable branch, then use the defined tag of branch
        # check if defined branch even exist
        BRANCH_EXIST=$(git ls-remote --heads origin $TAG_OR_BRANCH)
        if [[ -z "$BRANCH_EXIST" ]]; then
            echo ""
            echo "-------------------------------------------------------------------------------------"
            echo "Branch or tag '$TAG_OR_BRANCH' does not exist for the repository '$REPO_NAME'"
            echo "-------------------------------------------------------------------------------------"
            echo ""
            exit 1
        fi
        git checkout $TAG_OR_BRANCH
    else
        # if develop or feature branch, then try to checkout the feature-branch in the other repo as well
        # or otherwise use the develop-branch as default
        BRANCH_EXIST=$(git ls-remote --heads origin $CURRENT_BRANCH)
        if [[ -n "$BRANCH_EXIST" ]]; then
            git checkout $CURRENT_BRANCH
        else
            git checkout develop
        fi
    fi
}

function get_required_kitsune_lib_repo () {
    REPO_NAME=$1
    TAG_OR_BRANCH=$2
    NUMBER_OF_THREADS=$3

    download_repo_github $REPO_NAME $TAG_OR_BRANCH
    build_kitsune_lib_repo $REPO_NAME $NUMBER_OF_THREADS
}

#-----------------------------------------------------------------------------------------------------------------

echo "###########################################################################################################"
echo ""
get_required_kitsune_lib_repo "libKitsunemimiCommon" "develop" 8
get_required_kitsune_lib_repo "libKitsunemimiJson" "develop" 1
get_required_kitsune_lib_repo "libKitsunemimiIni" "develop" 1
get_required_kitsune_lib_repo "libKitsunemimiNetwork" "develop" 8
get_required_kitsune_lib_repo "libKitsunemimiArgs" "develop" 8
get_required_kitsune_lib_repo "libKitsunemimiConfig" "develop" 8
get_required_kitsune_lib_repo "libKitsunemimiObj" "develop" 8
get_required_kitsune_lib_repo "libKitsunemimiOpencl" "develop" 8
echo ""
echo "###########################################################################################################"
echo ""
get_required_kitsune_lib_repo "libKitsunemimiSqlite" "develop" 8
get_required_kitsune_lib_repo "libKitsunemimiCrypto" "develop" 8
get_required_kitsune_lib_repo "libKitsunemimiJwt" "develop" 8
echo ""
echo "###########################################################################################################"
echo ""
get_required_kitsune_lib_repo "libKitsunemimiSakuraNetwork" "develop" 8
get_required_kitsune_lib_repo "libKitsunemimiSakuraDatabase" "develop" 8
echo ""
echo "###########################################################################################################"
echo ""
get_required_kitsune_lib_repo "libKitsunemimiHanamiCommon" "develop" 8
get_required_kitsune_lib_repo "libKitsunemimiHanamiClusterParser" "develop" 1
get_required_kitsune_lib_repo "libKitsunemimiHanamiSegmentParser" "develop" 1
get_required_kitsune_lib_repo "libKitsunemimiHanamiDatabase" "develop" 8
download_repo_github "libKitsunemimiHanamiMessages" "develop"
get_required_kitsune_lib_repo "libKitsunemimiHanamiNetwork" "develop" 8
echo ""
echo "###########################################################################################################"
echo ""
get_required_kitsune_lib_repo "libAzukiHeart" "develop" 8
get_required_kitsune_lib_repo "libMisakiGuard" "develop" 8
get_required_kitsune_lib_repo "libShioriArchive" "develop" 8
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

