#!/bin/bash


build_dir="../build"
project_dir="../projects/linux/testing"
bin_dir="../bin"

usage() {
  echo "Usage: $0 [-d]"
  echo "Options:"
  echo "  -d: Delete the build directory if it exists and rebuild it"
  exit 1
}


delete_directory=0

while getopts ":d" opt; do
  case $opt in
    d)
      delete_directory=1
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      usage
      ;;
  esac
done


if [ $delete_directory -eq 1 ] && [ -d "$build_dir" ]; then
  rm -rf "$build_dir"
  echo "Removed existing build directory."
fi


if [ ! -d "$build_dir" ]; then
  mkdir -p "$build_dir"
fi


cd "$build_dir"


cmake -G Ninja "$project_dir"
ninja

cd "$bin_dir"