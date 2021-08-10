@echo off

:: Config
set "ParentDir=%~dp0"
set "BuildDir=%ParentDir%build\"
set "SourceDir=%ParentDir%src\"

set "Warnings=-Werror=all -Wno-unused -Wno-gnu-anonymous-struct -Wno-gnu-zero-variadic-macro-arguments"
set "CompilerFlags=-O0 -g -m64 %Warnings% -nostdlib -mno-stack-arg-probe -pedantic"
set "LinkerFlags=-fuse-ld=lld -Wl,-subsystem:console"


if not exist "%BuildDir%" (
    echo [*] Creating %BuildDir%
    mkdir "%BuildDir%"
)

pushd %BuildDir%

clang %CompilerFlags% "%SourceDir%\exec_hide.c" -o "hide.exe" %LinkerFlags%
clang %CompilerFlags% "%SourceDir%\test_msg.c" -o "test_msg.exe" %LinkerFlags%

popd
