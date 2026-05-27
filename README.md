<img src="https://github.com/ares-emulator/ares/blob/master/ares/ares/resource/logo@2x.png" width="350"/>

## Headless Ares N64 Runner

Implementing minimal N64-only Ares native headless runner for running libdragon/tiny3d/Pyrite64 homebrew outside standard desktop UI. Investigating whether Ares N64 core can be used for browser-deployable WASM frontends. Goal is to load a `.z64` or similar file, start the N64 core, use GPU-backed RDP path, and save frame buffers.

Impelmentation currently in `tools/n64-headless/`

### How to run (tentative commands)

#### Build headless implementation:
```
rm -rf build-headless

cmake -S . -B build-headless -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DARES_CORES="n64" \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_CXX_COMPILER=g++ \
  -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=OFF \
  -DCMAKE_C_FLAGS="-march=cascadelake -mtune=cascadelake -mno-avx512fp16" \
  -DCMAKE_CXX_FLAGS="-march=cascadelake -mtune=cascadelake -mno-avx512fp16"

cmake --build build-headless --target n64-headless -j 8
```

#### Test through frame (1000th) inspection
```
./build-headless/tools/n64-headless/n64-headless \
  --rom <rom>.z64 \
  --frames 1000 \
  --dump frame.ppm \
  --gpu 1 \
  --homebrew 1 \
  --recompiler 1

convert frame.ppm frame.png
```
