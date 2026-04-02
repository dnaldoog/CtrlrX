taskkill /f /im ninja.exe >nul 2>&1
taskkill /f /im cl.exe >nul 2>&1
rd /s /q build
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCTRLRX_USE_LUAJIT=ON
cmake --build build