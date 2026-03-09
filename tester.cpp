#include "Simulator/Core.hpp"
using namespace std;


int main() {
    Core core;

    vector<pair<string,string>> programs = {
{"ADD basic",
R"(addi x1 x0 10
addi x2 x0 20
add x3 x1 x2
add x4 x3 x1
)"},
{"SUB basic",
R"(addi x1 x0 50
addi x2 x0 15
sub x3 x1 x2
sub x4 x3 x2
)"},
{"MUL basic",
R"(addi x1 x0 6
addi x2 x0 7
mul x3 x1 x2
mul x4 x3 x2
)"},
{"DIV basic",
R"(addi x1 x0 100
addi x2 x0 4
div x3 x1 x2
div x4 x3 x2
)"},
{"ADDI basic",
R"(addi x1 x0 5
addi x2 x1 10
addi x3 x2 -3
addi x4 x3 100
)"},
{"LI basic",
R"(li x1 42
li x2 -1
li x3 1000
li x4 0
)"},
{"LA basic",
R"(.data
val1: .word 99
val2: .word 88
.text
la x1 val1
la x2 val2
)"},
{"SW and LW",
R"(addi x1 x0 42
sw x1 0(x0)
lw x2 0(x0)
addi x3 x0 99
sw x3 4(x0)
lw x4 4(x0)
)"},
{"LW from .data",
R"(.data
val1: .word 55
val2: .word 77
.text
la x5 val1
lw x1 0(x5)
la x6 val2
lw x2 0(x6)
)"},
{"BEQ taken",
R"(li x1 10
li x2 10
beq x1 x2 equal
add x3 x0 x0

equal: add x3 x1 x2
end:   add x0 x0 x0
)"},
{"BEQ not taken",
R"(li x1 10
li x2 20
beq x1 x2 equal
add x3 x0 x0

equal: add x4 x1 x2
end:   add x0 x0 x0
)"},
{"BNE taken",
R"(li x1 10
li x2 20
bne x1 x2 noteq
add x3 x0 x0

noteq: add x3 x1 x2
end:   add x0 x0 x0
)"},
{"BNE not taken",
R"(li x1 10
li x2 10
bne x1 x2 noteq
add x3 x0 x0

noteq: add x4 x1 x2
end:   add x0 x0 x0
)"},
{"BEQZ taken",
R"(li x1 0
beqz x1 zero
add x2 x1 x1

zero: addi x3 x0 5
end:  add x0 x0 x0
)"},
{"BEQZ not taken",
R"(li x1 7
beqz x1 zero
add x2 x1 x1

zero: addi x3 x0 5
end:  add x0 x0 x0
)"},
{"BNEZ taken",
R"(li x1 5
bnez x1 nonzero
add x2 x0 x0

nonzero: add x2 x1 x1
end:     add x0 x0 x0
)"},
{"BNEZ not taken",
R"(li x1 0
bnez x1 nonzero
add x2 x0 x0

nonzero: add x3 x1 x1
end:     add x0 x0 x0
)"},
{"Mixed arithmetic with hazards",
R"(addi x1 x0 3
addi x2 x0 4
add x3 x1 x2
sub x4 x3 x1
mul x5 x4 x2
)"},
{"Multiple branches",
R"(li x1 0
beqz x1 zero
add x2 x0 x0

zero: add x2 x1 x1
li x3 5
bnez x3 nonzero
add x4 x0 x0

nonzero: add x4 x3 x3
end:     addi x1 x0 4
)"},

{"SW/LW with arithmetic",
R"(addi x1 x0 6
addi x2 x0 7
mul x3 x1 x2
sw x3 0(x0)
lw x4 0(x0)
add x5 x4 x3
)"},

    };

    int passed = 0;
    for(int i = 0; i < (int)programs.size(); i++){
        cout << "=========================\n";
        cout << "TEST " << i+1 << ": " << programs[i].first << "\n";
        cout << "=========================\n";
        core.execute(programs[i].second);
        cout << "\n";
    }

    return 0;
}
