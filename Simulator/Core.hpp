#ifndef CORE_HPP
#define CORE_HPP

#include <iostream>
#include <vector>
#include <cstring>
#include <algorithm>
#include <map>
#include <fstream>
#include "json.hpp"
#include "Registers.hpp"

using namespace std;
using json = nlohmann::json;

class Core{
private:
    map<string,int> opcodes;
    vector<function<void()>> functions;
    vector<string> instructions;
    map<string,int> labels;
    map<string,int> datamap;
    int nextdatastart;

    int pc;
    int clock;
    int latency_acc;
    float ipc;
    int num_stalls;
    int remaining_ex_cycles;

    Register* registers;

    json config;
    bool forwarding;
    vector<int> latencies;
    char* memory;
    int memsize;

    IF_IDRF pr1;
    IDRF_EX pr2;
    EX_MEM pr3;
    MEM_WB pr4;

    bool stall;

    bool hazard(int reg);
    bool is_load_use(int reg);
    bool is_data_hazard(int reg);

    void IF();
    void IDRF();
    void EX();
    void MEM();
    void WB();

    void AluConfig();
    void execConfig(string& program);
    void Parse(stringstream& ss);

public:
    Core();
    void execute(string& program);
    ~Core();
};

#endif
