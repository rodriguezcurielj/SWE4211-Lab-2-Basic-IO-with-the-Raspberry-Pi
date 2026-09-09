shopt -s extglob
rm -Rf CMakeFiles
rm -v !(*.sh)
bash createProject.sh
#cmake ../src -G"Eclipse CDT4 - Unix Makefiles" -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE -DCMAKE_TOOLCHAIN_FILE=../Toolchain-rpi.cmake 

