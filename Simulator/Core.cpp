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
	crp=config["crp"].get<int>();
	l1isize=config["l1i"].get<int>();
	l1dsize=config["l1d"].get<int>();
	l2size=config["l2"].get<int>();
	bsize=config["bsize"].get<int>();
	l1it=new int[l1isize/bsize];
	l1dt=new int[l1dsize/bsize];
	l2t=new L2Tag[l2size/bsize];
	l1i=new char[l1isize];
	l1d=new char[l1dsize];
	l2=new char[l2size];
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
	delete[] l1d;
	delete[] l1i;
	delete[] l2;
	delete[] l1it;
	delete[] l1dt;
	delete[] l2t;
}
