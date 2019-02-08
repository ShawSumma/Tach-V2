# if clang++ -std=c++17 -O3 -Ofast -ffast-math main.cpp -o out/int.exe
if clang++ -std=c++17 -O3 -Ofast -ffast-math main.cpp -o out/int.exe
    then
    python3 parser.py lang/auto.txt run --opcode
    time ./out/int.exe
    echo "^ IntC++ ^"
fi
