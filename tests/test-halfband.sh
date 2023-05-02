rm test-halfband.exe
rm *.txt
g++ -Wall -o test-halfband.exe test-halfband.cpp
./test-halfband.exe smoke && echo OK!
./test-halfband.exe impulse > taps.txt
./test-halfband.exe chirp > chirp.txt
octave --no-window-system --eval "test_halfband"
