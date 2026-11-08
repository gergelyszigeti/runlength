# runlength
Superfast compression utility

### Build with one of these commands
You need GCC version 9.0 or later.

```bash
g++ -std=c++20 -Wall -o rl rl.cpp
g++ -std=c++20 -Wall -O2 -o rl rl.cpp
g++ -std=c++20 -Wall -O3 -o rl rl.cpp
g++ -std=c++20 -ggdb -Wall -o rl rl.cpp
```
Note: you can omit the ```-std=c++20``` switch if your gcc version is 16.0 or later.

### Test with one of these commands
```bash
./rl test/input.txt out/output.txt
./rl test/input.txt | hd
cat test/input.txt | ./rl | hd
```
