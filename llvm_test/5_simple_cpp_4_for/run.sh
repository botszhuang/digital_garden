#!/bin/bash

# Call the Makefile, passing the target as a variable
source="hello_op"
make TARGET=${source}

./${source}
# make clean TARGET=${source}

