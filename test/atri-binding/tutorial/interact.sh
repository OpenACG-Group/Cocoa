#!/usr/bin/env bash

#function msg() {
#    echo -e "\e["
#}

declare cr_title1="\e[34;1m"
declare cr_title2="\e[33;1m"
declare cr_reset="\e[0m"
alias out="echo -e"

## Clear screen first
clear

function wait_for_enter() {
    read
    clear
}

echo -e "
${cr_title1}[*] I. Prologue${cr_reset}
            ${cr_title2}Cocoa JavaScript Language Binding Tutorial${cr_reset}
                    OpenACG Group - Cocoa Project
                            2022-02-03

Welcome to this interactive Cocoa JavaScript language binding tutorial.
This tutorial gives you a series of instructions and you should follow them to
learn how to create your own language binding.
It won't take a long time and you will learn most things about the language
binding.
If the screen no longer updates the text, press enter to the next step.

If you see a URL like 'file:///path/to/something', try clicking it with Ctrl pressed
the URL will be opened if your terminal supports that.

Press ENTER...
"
wait_for_enter

declare files_list="atri.h atri.cc exports.mds"

echo -e "
${cr_title1}[*] I. File structure${cr_reset}
This directory contains a smallest example of a language binding named 'ATRI',
which is named after a visual novel \e[3;1mATRI -My Dear Moments-\e[0m.
There are some necessary files contained in this directory. This script will check
them now.
"

for fp in $files_list; do
    printf "\e[1mChecking \e[3m${fp}\e[0m... "
    if [ -f $fp ]; then
        echo -e "OK"
    else
        echo -e "FAIL"
        echo -e "\e[31;1mMissing file ${fp}. Maybe you need to download or clone this project again."
        exit 1
    fi
done

echo -e "
All the files are OK. Now you need to know about them, and more details will
be introduced later.

A language binding is basically a series of C++ source files and a '.mds' file.
There must be a 'core' header file which declares all the native classes and functions
to be exported to JavaScript land. Most importantly, a special class called 'binding class'
must be declared in that header file.

The 'binding class' is implemented in 'core' source file that generally has the same name
to 'core' header file. For example, your 'core' header file is 'super_cool_binding.h', then
the corresponding source file should be 'super_cool_binding.cc' or 'super_cool_binding.cpp'.
Although we suggest you doing this, it's not a mandatory requirement.

At last, there is also a '.mds' file that describes the symbols to be exported to JavaScript
land.

In ATRI, the files corresponding to the above description are:
- atri.h        'core' header file of ATRI language binding.
- atri.cc       'core' source file of ATRI language binding.
- exports.mds   Module description file.
"
wait_for_enter

echo -e "
${cr_title1}[*] II. Binding Loader${cr_reset}
A Cocoa language binding is basically a dynamic shared library (aka shared object '.so')
which can be loaded dynamically at runtime via OS's dynamic linker.
As soon as Cocoa loads a shared object, it looks up symbol '__g_cocoa_hook'. If success,
the address of this symbol will be called as a function pointer and that function is supposed
to return a pointer of an instance of 'binding class', which will be deleted automatically
when Cocoa no longer needs it.
What's more, symbol '__g_cocoa_hook' must be declared with \e[3mextern \"C\"\e[0m to avoid being mangled by
C++ compiler.

The instance of 'binding class' that '__g_cocoa_hook' returns is the unique identification
of a language binding. That is to say, a shared object can only provide one language binding,
and a language binding can only have one 'binding class'.

Now open file://$(pwd)/atri.cc,
find a function named '__g_cocoa_hook' to see how it works and think
which is the 'binding class' of this demo binding.
"
wait_for_enter
