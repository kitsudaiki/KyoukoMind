# KyoukoMind

## Description

- New concept for a dynamically growing artificial neuronal network
    
## Build

#### Required build tools

name | repository | version | task
--- | --- | --- | ---
g++ | g++ | >= 8.0 | Compiler for the C++ code.
make | make | >= 4.0 | process the make-file, which is created by qmake to build the programm with g++
qmake | qt5-qmake | >= 5.0 | This package provides the tool qmake, which is similar to cmake and create the make-file for compilation.
FLEX | flex | >= 2.6 | Build the lexer-code for all used parser.
GNU Bison | bison | >= 3.0 | Build the parser-code together with the lexer-code.
xxd | xxd | >= 1.10 | converts text files into source code files, which is used to complile the OpenCl-kernel file into the source-code.

#### Required generic libraries

name | repository | version | task
--- | --- | --- | ---
ssl library | libssl-dev | 1.1.x | encryption for tls connections
crpyto++ | libcrypto++-dev | >= 5.6 | provides encryption-functions like AES
uuid | uuid-dev | >= 2.34 | generate uuid's
sqlite3 library | libsqlite3-dev | >= 3.0 | handling of sqlite databases
opencl-headers  | opencl-headers | 2.x | Header-files for opencl
ocl-icd-opencl-dev | ocl-icd-opencl-dev | 2.x | libraries for opencl

#### Required kitsunemimi libraries

Repository-Name | Version-Tag | Download-Path
--- | --- | ---
libKitsunemimiCommon | v0.25.1 |  https://github.com/kitsudaiki/libKitsunemimiCommon.git
libKitsunemimiJson | v0.11.3 |  https://github.com/kitsudaiki/libKitsunemimiJson.git
libKitsunemimiJinja2 | v0.9.1 |  https://github.com/kitsudaiki/libKitsunemimiJinja2.git
libKitsunemimiIni | v0.5.1 |  https://github.com/kitsudaiki/libKitsunemimiIni.git
libKitsunemimiNetwork | v0.8.2 |  https://github.com/kitsudaiki/libKitsunemimiNetwork.git
libKitsunemimiArgs | v0.4.0 |  https://github.com/kitsudaiki/libKitsunemimiArgs.git
libKitsunemimiConfig | v0.4.0 |  https://github.com/kitsudaiki/libKitsunemimiConfig.git
libKitsunemimiObj | v0.2.0 |  https://github.com/kitsudaiki/libKitsunemimiObj.git
libKitsunemimiOpencl | v0.4.0 |  https://github.com/kitsudaiki/libKitsunemimiOpencl.git
libKitsunemimiCrypto | v0.2.0 |  -
libKitsunemimiJwt | v0.4.1 |  -
libKitsunemimiSqlite | v0.3.0 |  -
libKitsunemimiSakuraNetwork | v0.8.2 |  https://github.com/kitsudaiki/libKitsunemimiSakuraNetwork.git
libKitsunemimiSakuraLang | v0.12.0 |  https://github.com/kitsudaiki/libKitsunemimiSakuraLang.git
libKitsunemimiSakuraDatabase | v0.4.1 |  -
libKitsunemimiHanamiCommon | v0.1.0 |  -
libKitsunemimiHanamiEndpoints | v0.1.0 |  -
libKitsunemimiHanamiDatabase | v0.1.0 |  -
libKitsunemimiHanamiMessaging | v0.2.0 |  -
libKitsunemimiHanamiPredefinitions | v0.1.0 |  -

HINT: These Kitsunemimi-Libraries will be downloaded and build automatically with the build-script below.

### build library

In all of my repositories you will find a `build.sh`. You only have to run this script. It doesn't required sudo, because you have to install required tool via apt, for example, by yourself. But if other projects from me are required, it download them from github and build them in the correct version too. This script is also use by the ci-pipeline, so its tested with every commit.


Run the following commands:

```
git clone https://github.com/kitsudaiki/KyoukoMind.git
cd KyoukoMind
./build.sh
cd ../result
```

It create automatic a `build` and `result` directory in the directory, where you have cloned the project. At first it build all into the `build`-directory and after all build-steps are finished, it copy the include directory from the cloned repository and the build library into the `result`-directory. So you have all in one single place.

Tested on Debian and Ubuntu. If you use Centos, Arch, etc and the build-script fails on your machine, then please write me a mail and I will try to fix the script.

## Usage

(sorry, docu comes later)


## Contributing

Please give me as many inputs as possible: Bugs, bad code style, bad documentation and so on.

## License

This project is licensed under the Apache License Version 2.0 - see the [LICENSE](LICENSE) file for details
