#include <bits/stdc++.h>
using namespace std;

#include "Simulator/Core.hpp"

int main(int argc, char* argv[]) {

    if(argc != 2){
        cerr << "Usage: ./runner <assembly_file>\n";
        return 1;
    }

    string filepath = argv[1];

    ifstream file(filepath);
    if(!file.is_open()){
        cerr << "Error: Cannot open file\n";
        return 1;
    }

    stringstream buffer;
    buffer << file.rdbuf();
    string program = buffer.str();

    file.close();

    Core core;
    core.execute(program);

    return 0;
}