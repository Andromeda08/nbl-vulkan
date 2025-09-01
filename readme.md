❗**Before running:**
1. Move resources to the build directory (cmake-build-debug/nebula or cmake-build-release/nebula)
    1. Move [.hair files](./resource/hair) to build directory.
    2. Move [.ttf (font) file](./resource/font) to build directory.
2. Compile shaders (e.g. using the provided Python script)  
    This script will compile shaders and move them to the build directories.
    ```shell
    cd shader
    py ./nbl_shader_util.py
    ```