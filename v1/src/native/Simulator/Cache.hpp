#ifndef CACHE_HPP
#define CACHE_HPP

struct LRUTable{
    int numsets;
    int blocksperset;
    int** table;
    LRUTable(){
        table=nullptr;
    }
    void init(int ns,int bps){
        numsets=ns;
        blocksperset=bps;table=new int*[numsets];
        for(int i=0;i<numsets;i++){
            table[i]=new int[blocksperset];
            for(int j=0;j<blocksperset;j++){
                table[i][j]=-1;
            }
        }
    }
    void ground(){
        for(int i=0;i<numsets;i++){
            for(int j=0;j<blocksperset;j++){
                table[i][j]=-1;
            }
        }
    }
    ~LRUTable(){
        for(int i=0;i<numsets;i++){
            delete[] table[i];
        }
        delete[] table;
    }
};

struct Block{
    int tag;
    bool dirty;
    bool isinst;
    char* bytes;
    int bsize;
    Block(){
        bsize=0;
        bytes=nullptr;
    }
    void init(int size){
        bsize=size;
        bytes=new char[bsize];
        tag=-1;
    }
    void ground(){
        tag=-1;
    }
    ~Block(){
        delete[] bytes;
    }
};

struct Set{
    int ssize;
    int bsize;
    Block* blocks;
    Set(){
        ssize=0;
        blocks=nullptr;
    }
    void init(int size,int bsize){
        ssize=size;
        blocks=new Block[ssize];
        for(int i=0;i<ssize;i++){
            blocks[i].init(bsize);
        }
    }
    void ground(){
        for(int i=0;i<ssize;i++){
            blocks[i].ground();
        }
    }
    ~Set(){
        delete[] blocks;
    }
};

struct Cache{
    int numsets;
    int numblocks;
    int numbytes;
    Set* sets;
    Cache(){
        sets=nullptr;
    }
    void init(int ns,int bps,int bpb){
        numsets=ns;
        numblocks=bps;
        numbytes=bpb;
        sets=new Set[numsets];
        for(int i=0;i<numsets;i++){
            sets[i].init(numblocks,numbytes);
        }
    }
    void ground(){
        for(int i=0;i<numsets;i++){
            sets[i].ground();
        }
    }
    ~Cache(){
        delete[] sets;
    }
};


#endif