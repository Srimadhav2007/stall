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

struct L2Tag{
    bool isinst;
    int tag;
};

class Core{
private:
    map<string,int> opcodes;
    vector<function<void()>> functions;
    vector<int> instructions;
    vector<string> insts;
    map<string,int> labels;
    map<string,int> datamap;

    int pc;
    int clock;
    int latency_acc;
    float ipc;
    int num_stalls;
    int nextdatastart;
    int num_instructions;

    Register* registers;

    json config;
    bool forwarding;
    vector<int> latencies;
    char* memory;
    int* l1it;
    int* l1dt;
    L2Tag* l2t;
    char* l1i;
    char* l1d;
    char* l2;
    int memsize;
    int l1isize;
    int l1dsize;
    int l2size;
    int bsize;
    int crp;

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
    void AssemblyToMachineCode(vector<string>& insts);

    void LRU(bool isl1,bool isi,int tag);
    void PLRU(bool isl1,bool isi,int tag);

public:
    Core();
    vector<int> execute(string& program);
    ~Core();
};

#endif
