if (-not (Test-Path build) ) { mkdir build }

pushd build
cmake ..
popd