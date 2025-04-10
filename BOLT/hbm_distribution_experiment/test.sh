#!/bin/bash
# Set unlimited stack size (if needed)
ulimit -s unlimited
source /tools/Xilinx/Vitis/2024.1/settings64.sh
source /opt/xilinx/xrt/setup.sh
# Loop through each subdirectory in the current directory.
for folder in */; do
    # Check if both host.cpp and run.sh exist in the folder.
    if [ -f "${folder}/host.cpp" ] && [ -f "${folder}/run.sh" ]; then
        echo "Processing folder: $folder"
        # Change into the folder.
        pushd "$folder" > /dev/null

        # Run the run.sh script.
        ./run.sh

        # Return to the parent directory.
        popd > /dev/null
        echo "Finished processing $folder"
        echo "-------------------------------------"
    fi
done
