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
#include "Cache.hpp"

using namespace std;
using json = nlohmann::json;


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
    int mem_stalls;
    int nextdatastart;
    int num_instructions;

    Register* registers;

    json config;
    bool forwarding;
    vector<int> latencies;
    char* memory;
    int memsize;

    int l1isize;
    int l1dsize;
    int l2size;
    int bsize;
    int numsetsl1;
    int numsetsl2;
    int crp;
    int bpsl1;
    int bpsl2;
    Cache l1i;
    Cache l1d;
    Cache l2;
    LRUTable lrut1i;
    LRUTable lrut1d;
    LRUTable lrut2;
    int l1llat;
    int l2llat;
    int memllat;
    int l1slat;
    int l2slat;
    int memslat;

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

    char* LRUl(bool isl1,bool isi,int tag,int numbytes);
    void LRUs(bool isl1,bool isi,int tag,int numbytes,char* data);
    char* PLRU(bool isl1,bool isi,int tag,int numbytes);

public:
    Core();
    void execute(string& program);
    void runFromTrace(string& program);
    ~Core();
};

#endif
