rm test-halfband.exe
rm *.txt
g++ -Wall -o test-halfband.exe test-halfband.cpp
./test-halfband.exe smoke && echo OK smoke
./test-halfband.exe impulse > taps.txt && echo OK impulse
./test-halfband.exe test > test.txt && echo OK test
octave --no-window-system --eval "test_halfband"
