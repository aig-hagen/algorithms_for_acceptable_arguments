#!/bin/bash
git submodule update --init --recursive
cd cadical && ./configure
