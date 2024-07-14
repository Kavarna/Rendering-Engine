mkdir build
cd build

conan install .. -s build_type=Debug --build missing -of .
cmake .. -G "Visual Studio 17" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=OFF -DCMAKE_POLICY_DEFAULT_CMP0091=NEW

cd ../
