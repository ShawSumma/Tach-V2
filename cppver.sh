# if clang++ -std=c++17 -O3 -Ofast -ffast-math main.cpp -o out/int.exe
if [[ $1 == "no-cpp" ]] || clang++ -std=c++17 -O3 -Ofast -ffast-math main.cpp -o out/int.exe
    then
    python3 parser.py include/auto.txt run --opcode
    time ./out/int.exe
    echo "^ IntC++ ^"
fi
