cmake --preset=dev && cmake --build --preset=dev

if [ $? -ne 0 ]; then
    echo "CMake failed"
    exit 1
fi

# Grep for day_* and run them
for f in $(ls build/day_*); do
 
    $f &> /tmp/out.txt

    # Check for errors

    if [ $? -eq 0 ]; then
        echo "Success $f"
    else
        echo "Failed $f, log:"
        cat /tmp/out.txt
        exit 1
    fi

done


