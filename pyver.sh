time /snap/bin/micropython parser.py lang/auto.txt run
echo '^ MicroPython ^'
echo
echo
time python3 parser.py lang/auto.txt run
echo '^ CPython3 ^'
echo
echo
time /snap/bin/pypy3 parser.py lang/auto.txt run
echo '^ PyPy3 ^'