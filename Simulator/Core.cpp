#include "Core.hpp"
using namespace std;
using json=nlohmann::json;

Core::Core(){
    ifstream f("config.json");
	f>>config;
	f.close();
	memsize=config["memory"].get<int>();
	if(memsize<4096)memsize=4096;
	memory=new char[memsize];

	opcodes["add"]=0;
	opcodes["sub"]=1;
	opcodes["mul"]=2;
	opcodes["div"]=3;
	opcodes["addi"]=4;
	opcodes["li"]=5;
	opcodes["la"]=6;
	opcodes["lw"]=7;
	opcodes["sw"]=8;
	opcodes["bne"]=9;
	opcodes["beq"]=10;
	opcodes["bnez"]=11;
	opcodes["beqz"]=12;
	opcodes["j"]=13;
	opcodes["jal"]=14;
	opcodes["slt"]=15;

	auto &lat = config["latency"][0];
	forwarding = config["forwarding"].get<bool>();

	latencies.push_back(lat["add"].get<int>());
	latencies.push_back(lat["sub"].get<int>());
	latencies.push_back(lat["mul"].get<int>());
	latencies.push_back(lat["div"].get<int>());
	latencies.push_back(lat["addi"].get<int>());
	latencies.push_back(lat["li"].get<int>());
	latencies.push_back(lat["la"].get<int>());
	latencies.push_back(lat["lw"].get<int>());
	latencies.push_back(lat["sw"].get<int>());
	latencies.push_back(lat["bne"].get<int>());
	latencies.push_back(lat["beq"].get<int>());
	latencies.push_back(lat["bnez"].get<int>());
	latencies.push_back(lat["beqz"].get<int>());
	latencies.push_back(lat["j"].get<int>());
	latencies.push_back(lat["jal"].get<int>());
	latencies.push_back(lat["slt"].get<int>());

    pc=0;
    clock=0;
    latency_acc=0;
	registers=new Register[32];
    AluConfig();
}

Core::~Core(){
    delete[] registers;
	delete[] memory;
}
