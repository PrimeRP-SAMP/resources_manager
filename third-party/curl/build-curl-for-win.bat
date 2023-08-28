@echo off

REM You need to set `$IS_DEBUG`, `$IS_64`, `$IS_SHARED` first
REM E.g.: IS_DEBUG=YES, IS_64=NO, IS_SHARED=YES
REM For x64 you have to run your this script from x64 Native Tools Command Prompt for VS 201x, not the Developer Command Prompt for VS 201x.
REM Using Qt Jom is recommended to build OpenSSL faster. Download it here and add to PATH: https://wiki.qt.io/Jom
REM Required: Python3, Git, CMake, Perl, Visual Studio, nasm

IF "%IS_SHARED%" == "" (
    echo "IS_SHARED is undefined, assuming as NO."
    set IS_SHARED=NO
)

IF "%IS_DEBUG%" == "" (
    echo "IS_DEBUG is undefined, assuming as NO."
    set IS_DEBUG=NO
)

IF "%IS_64%" == "" (
    echo "IS_64 is undefined, assuming as NO."
    set IS_64=NO
)

IF "%IS_DEBUG%"=="YES" (
    set OPENSSL_BUILD_TYPE=--debug
    set CMAKE_BUILD_TYPE=Debug

    set ZLIB_LIB_NAME=zlibstaticd.lib
    set CURL_LIB_NAME=libcurl-d.lib
    set CURL_STATICLIB_NAME=libcurl-d_static.lib
) else (
    set OPENSSL_BUILD_TYPE=--release
    set CMAKE_BUILD_TYPE=Release

    set ZLIB_LIB_NAME=zlibstatic.lib
    set CURL_LIB_NAME=libcurl.lib
    set CURL_STATICLIB_NAME=libcurl_static.lib
)

IF "%IS_64%"=="YES" (
    set CMAKE_VS_PLATFORM_NAME=x64
    set OPENSSL_BUILD_TARGET=VC-WIN64A
) else (
    set CMAKE_VS_PLATFORM_NAME=Win32
    set OPENSSL_BUILD_TARGET=VC-WIN32
)

set WORKING_SCR_DIR=%cd%

rmdir /S /Q openssl zlib nghttp2 libssh2 curl out

REM Building OpenSSL
git clone -b openssl-3.1.2 https://github.com/openssl/openssl --depth=1
cd openssl
set SSL_DIR=%cd%

where jom >NUL
IF %ERRORLEVEL% NEQ 0 (
    echo "Attention! Jom is not found. Building will be too slow"
    echo "Download it here and add to PATH: https://wiki.qt.io/Jom"
    perl Configure %OPENSSL_BUILD_TARGET% no-shared %OPENSSL_BUILD_TYPE%
    nmake
) else (
    perl Configure %OPENSSL_BUILD_TARGET% no-shared %OPENSSL_BUILD_TYPE% /FS
    jom
)

cd %WORKING_SCR_DIR%
REM Building OpenSSL

REM Building zlib 
git clone -b v1.2.13 https://github.com/madler/zlib --depth=1
cd zlib
set ZLIB_DIR=%cd%
mkdir build
cd build
cmake .. ^
    -DCMAKE_BUILD_TYPE=%CMAKE_BUILD_TYPE% ^
    -A %CMAKE_VS_PLATFORM_NAME%
msbuild zlib.sln /p:Configuration=%CMAKE_BUILD_TYPE% /p:Platform=%CMAKE_VS_PLATFORM_NAME%
copy zconf.h ..\ /Y
cd %CMAKE_BUILD_TYPE%
set ZLIB_BUILD_OUT_DIR=%cd%
cd %WORKING_SCR_DIR%
REM Building zlib

REM Building nghttp2 
git clone -b v1.55.1 https://github.com/nghttp2/nghttp2 --depth=1
cd nghttp2
set NGHTTP_DIR=%cd%
mkdir build
cd build
cmake .. ^
    -DCMAKE_BUILD_TYPE=%CMAKE_BUILD_TYPE% ^
    -DENABLE_LIB_ONLY=YES ^
    -DENABLE_SHARED_LIB=NO ^
    -DENABLE_STATIC_LIB=YES ^
    -A %CMAKE_VS_PLATFORM_NAME%
msbuild nghttp2.sln -target:nghttp2_static /p:Configuration=%CMAKE_BUILD_TYPE% /p:Platform=%CMAKE_VS_PLATFORM_NAME%
cd lib
copy includes\nghttp2\nghttp2ver.h %NGHTTP_DIR%\lib\includes\nghttp2 /Y
cd %CMAKE_BUILD_TYPE%
set NGHTTP_BUILD_OUT_DIR=%cd%
cd %WORKING_SCR_DIR%
REM Building nghttp2

REM Building libssh2 
git clone -b libssh2-1.11.0 https://github.com/libssh2/libssh2 --depth=1
cd libssh2
set LIBSSH2_DIR=%cd%
mkdir build
cd build
cmake .. ^
    -DCMAKE_BUILD_TYPE=%CMAKE_BUILD_TYPE% ^
    -DBUILD_STATIC_LIBS=ON ^
    -DBUILD_SHARED_LIBS=OFF ^
    -DOPENSSL_ROOT_DIR=%SSL_DIR% ^
    -DENABLE_ZLIB_COMPRESSION=ON ^
    -DZLIB_LIBRARY=%ZLIB_BUILD_OUT_DIR%\%ZLIB_LIB_NAME% ^
    -DZLIB_INCLUDE_DIR=%ZLIB_DIR% ^
    -A %CMAKE_VS_PLATFORM_NAME%
msbuild libssh2.sln -target:libssh2_static /p:Configuration=%CMAKE_BUILD_TYPE% /p:Platform=%CMAKE_VS_PLATFORM_NAME%
cd src
cd %CMAKE_BUILD_TYPE%
set LIBSSH2_BUILD_OUT_DIR=%cd%
cd %WORKING_SCR_DIR%
REM Building libssh2

REM Building curl 
git clone -b curl-8_2_1 https://github.com/curl/curl --depth=1
cd curl
set CURL_DIR=%cd%
mkdir build
cd build
cmake .. ^
    -DBUILD_SHARED_LIBS=%IS_SHARED% ^
    -DCURL_USE_OPENSSL=YES ^
    -DUSE_ZLIB=YES ^
    -DUSE_NGHTTP2=YES ^
    -DCURL_USE_LIBSSH2=YES ^
    -DOPENSSL_ROOT_DIR=%SSL_DIR% ^
    -DZLIB_LIBRARY=%ZLIB_BUILD_OUT_DIR%\%ZLIB_LIB_NAME% ^
    -DZLIB_INCLUDE_DIR=%ZLIB_DIR% ^
    -DNGHTTP2_LIBRARY=%NGHTTP_BUILD_OUT_DIR%\nghttp2.lib ^
    -DNGHTTP2_INCLUDE_DIR=%NGHTTP_DIR%\lib\includes ^
    -DLIBSSH2_LIBRARY=%LIBSSH2_BUILD_OUT_DIR%\libssh2.lib ^
    -DLIBSSH2_INCLUDE_DIR=%LIBSSH2_DIR%\include ^
    -DCMAKE_C_FLAGS="/DNGHTTP2_STATICLIB /DLIBSSH2_API=" ^
    -A %CMAKE_VS_PLATFORM_NAME%
msbuild CURL.sln -target:libcurl /p:Configuration=%CMAKE_BUILD_TYPE% /p:Platform=%CMAKE_VS_PLATFORM_NAME%
cd lib
cd %CMAKE_BUILD_TYPE%
set CURL_BUILD_OUT_DIR=%cd%
cd %WORKING_SCR_DIR%
REM Building curl

REM Moving all libs to output directory
mkdir out
cd out
set OUT_DIR=%cd%

IF "%IS_SHARED%" == "NO" (
    copy %SSL_DIR%\libcrypto.lib . /Y
    copy %SSL_DIR%\libssl.lib . /Y
    copy %ZLIB_BUILD_OUT_DIR%\%ZLIB_LIB_NAME% . /Y
    copy %NGHTTP_BUILD_OUT_DIR%\nghttp2.lib . /Y
    copy %LIBSSH2_BUILD_OUT_DIR%\libssh2.lib . /Y
    copy %CURL_BUILD_OUT_DIR%\%CURL_LIB_NAME% . /Y
) else (
    copy %CURL_BUILD_OUT_DIR%\*.* . /Y
)
cd %WORKING_SCR_DIR%
REM Moving all libs to output directory

REM Linking libraries into single static library
cd out
IF "%IS_SHARED%" == "NO" (
    lib /OUT:%CURL_STATICLIB_NAME% *.lib Ws2_32.lib Wldap32.lib Crypt32.lib
)
cd %WORKING_SCR_DIR%
REM Linking libraries into single static library

REM Downloading cacerts
cd out
curl -L https://curl.se/ca/cacert.pem -o curl-ca-bundle.crt
IF %ERRORLEVEL% NEQ 0 (
    echo "You have no curl installed, couldn't download curl-ca-bundle.crt."
    echo "Please, download, rename to curl-ca-bundle.crt and put to %OUT_DIR%: https://curl.se/ca/cacert.pem"
)
cd %WORKING_SCR_DIR%
REM Downloading cacerts

echo "Building done! See %OUT_DIR%"
IF "%IS_SHARED%" == "NO" (
    echo "Please, link %CURL_STATICLIB_NAME% to your app. Nothing else is required."
) else (
    echo "Please, link libcurl(-d)_imp.lib to your app and ship libcurl(-d).dll with it. Nothing else is required."
)

echo "Ship curl-ca-bundle.crt with your app."
echo "HTTPS connections won't work i not present!"
