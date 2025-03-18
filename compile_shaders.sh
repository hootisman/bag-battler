#!/bin/bash

resource_path="resources/shaders/"

for file in $resource_path*.hlsl
do
raw="${file%.*}"
echo "shadercross $file -o $raw.spv"
shadercross $file -o $raw.spv
done