if not exist "build_opencv" (
	mkdir build_opencv
)

cd build_opencv
cmake -DBUILD_SHARED_LIBS=OFF ../thirdparty/opencv -G"Visual Studio 16 2019"

cd ..
pause