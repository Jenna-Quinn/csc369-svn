csc369-a1-seed
==============

Starter code & documentation for CSC369 Fall 2014, Assignment 1

## Overview

This repository contains the starter code and a slightly-modified `Makefile` for Assignment 1:

<http://www.cdf.toronto.edu/~csc369h/fall/assignments/a1/a1.html>

## Makefile

The `Makefile` is modified to compile a `test_malloc` binary *and* a `test_malloc_opt` binary
instead of just `test_malloc`.

## Usage

Initialize your project directory as a [`git-svn`][git-svn] repository:

    cd path/to/CSC369/a1/
    git svn init --username=g4name https://markus.cdf.toronto.edu/svn/csc369-2014-09/group_xxxx/a1

(Replace `g4name` with your CDF name and `group_xxxx` with your own)

Pull this repository's contents:

    git pull https://github.com/elliottsj/csc369-a1-seed.git
    
Then commit it to your MarkUs Subversion repository:
    
    git svn dcommit

[git-svn]: http://git-scm.com/book/en/Git-and-Other-Systems-Git-and-Subversion
