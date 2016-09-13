execline
=========

[![Build Status](https://img.shields.io/circleci/project/amylum/execline.svg)](https://circleci.com/gh/amylum/execline)
[![GitHub release](https://img.shields.io/github/release/amylum/execline.svg)](https://github.com/amylum/execline/releases)
[![ISC Licensed](https://img.shields.io/badge/license-ISC-green.svg)](https://tldrlegal.com/license/-isc-license)

This is my package repo for [execline](http://www.skarnet.org/software/execline/), a scripting language by [Laurent Bercot](http://skarnet.org/).

The `upstream/` directory is taken directly from upstream. The rest of the repository is my packaging scripts for compiling a distributable build.

## Usage

To build a new package, update the submodule and run `make`. This launches the docker build container and builds the package.

To start a shell in the build environment for manual actions, run `make manual`.

## License

The execline upstream code is ISC licensed. My packaging code is MIT licensed.

