#!/bin/bash

cc load_raw.c -oload_raw -ggdb
cc load_ez.c -oload_ez -ggdb
cc save_raw.c -osave_raw -ggdb
cc save_ez.c -osave_ez -ggdb

