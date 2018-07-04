#!/bin/bash

for i in `find -regex '.*/.*\.\(cpp\|h\)$'
`
do
  if ! grep -q Copyright $i
  then
  	echo $i
    cat _license_.txt $i >$i.new && mv $i.new $i
  fi
done
