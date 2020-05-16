TARGET=$1
if [ "$TARGET" = "gcc" ]; then
    gcc lang.c -Wall -Wshadow -Ofast -o lang
elif [ "$TARGET" = "emcc" ]; then
    emcc lang.c -s WASM=1 -s FORCE_FILESYSTEM=1 -s EXIT_RUNTIME=1 -s INVOKE_RUN=0 -s MODULARIZE=1 -s 'EXPORT_NAME="MyCode"' -s 'EXTRA_EXPORTED_RUNTIME_METHODS=["FS", "callMain"]' -s ALLOW_MEMORY_GROWTH=1 -o lang.js
else
    echo "Usage: ./compile.sh (gcc|emcc)"
fi

