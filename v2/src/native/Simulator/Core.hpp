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
#include "MMU.hpp"

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
    int bsize;
    int numsetsl1;
    int crp;
    int bpsl1;
    Cache l1i;
    Cache l1d;
    
    LRUTable lrut1i;
    LRUTable lrut1d;
    char* LRUl(bool isl1,bool isi,int tag,int numbytes);
    void LRUs(bool isl1,bool isi,int tag,int numbytes,char* data);
    
    PLRUTable plrut1i;
    PLRUTable plrut1d;
    char* PLRUl(bool isl1,bool isi,int tag,int numbytes);
    void  PLRUs(bool isl1,bool isi,int tag,int numbytes,char* data);

    int l1llat;
    int memllat;
    int l1slat;
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

    MMU  mmu;
    int  pageSize;
    int  physMemSize;
    int  tlbEntries;
    int  tlbHitLat;
    int  pageWalkLat;
    int  pageFaultLat;
    int  swapLat;
    
public:
    void printMMUStats();

    Core();
    vector<int> execute(string& program);
    void runFromTrace(string& program);
    ~Core();
    
};

#endif
