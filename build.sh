mkdir build
cd build

conan install .. -s build_type=Debug --build missing
cmake .. -DCMAKE_BUILD_TYPE=Debug

cd ../
