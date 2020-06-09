#!/bin/bash
#
# Git sync script, should be called manually or from makefile
# @author chairs
#
git add .
git commit -a -m "$1"
git push --origin master
