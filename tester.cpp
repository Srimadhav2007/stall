#include <bits/stdc++.h>
using namespace std;
#include "simulator.cpp"

int main() {

    Core core;

    vector<string> programs = {

R"(.data
array: .word 1 2 3 4 5
.text
la x1 array
lw x2 4(x1)
add x5 x2 x3
add x4 x2 x5
sub x6 x4 x2
)",

R"(li x2 7
alve: add x1 x2 x3
label: add x4 x1 x5
titty: sub x6 x4 x2
sub x6 x4 x2
)",

R"( li x2 5
add x1 x2 x3
add x1 x2 x3
add x1 x2 x3
add x1 x2 x3
)",

R"(li x1 7
li x2 3
mul x3 x1 x2
mul x4 x3 x2
)",

R"(addi x1 x0 5
addi x0 x2 6
add x3 x1 x2
sub x4 x3 x1
mul x5 x4 x2
mul x6 x5 x1
)"

    };
    for(int i = 0; i < programs.size(); i++) {

        cout << "=========================\n";
        cout << "Running Program " << i+1 << endl;
        cout << "=========================\n";
        
        core.execute(programs[i]);

        cout << endl;
    }
    return 0;
}