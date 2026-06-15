#ifndef CACHE_HPP
#define CACHE_HPP

struct LRUTable{
    int numsets;
    int blocksperset;
    int** table;
    LRUTable(){
        numsets=0;
        blocksperset=0;
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
        if(table==nullptr) return;
        for(int i=0;i<numsets;i++){
            for(int j=0;j<blocksperset;j++){
                table[i][j]=-1;
            }
        }
    }
    ~LRUTable(){
        if(table==nullptr) return;
        for(int i=0;i<numsets;i++){ 
            delete[] table[i];
        }
        delete[] table;
    }
}; 

struct PLRUTable {
    int numsets;
    int blocksperset;
    int* bits;  

    PLRUTable() : numsets(0), blocksperset(0), bits(nullptr) {}

    void init(int ns, int bps) {
        numsets = ns;
        blocksperset = bps;
        bits = new int[numsets];
        ground();
    }

    void ground() {
        for (int i = 0; i < numsets; i++)
            bits[i] = 0;
    }

    void update(int setIndex, int way) {
        int& b = bits[setIndex]; //Taking address cuz we have to update it in memory and not a copy.
        if (blocksperset == 2) {
            if (way == 0) b |=  0x1;
            else          b &= ~0x1;
        }
        else if (blocksperset == 4) {
            switch (way) {
                case 0: b = (b | 0x02) & ~0x01; break; // root→right, left→way1
                case 1: b = (b | 0x03);          break; // root→right, left→way0
                case 2: b = (b | 0x04) & ~0x03; break; //similar
                case 3: b = (b & ~0x04) | 0x05; break; 
            }
        }
        else if (blocksperset == 8) {
            switch (way) {
                case 0: b = (b | 0x0A) & ~0x01; break;
                case 1: b = (b | 0x0B);          break;
                case 2: b = (b | 0x12) & ~0x03; break;
                case 3: b = (b | 0x13) & ~0x02; break;
                case 4: b = (b | 0x24) & ~0x07; break;
                case 5: b = (b | 0x25) & ~0x06; break;
                case 6: b = (b | 0x44) & ~0x0F; break;
                case 7: b = (b | 0x45) & ~0x0E; break;
            }
        }
    }

    int getVictim(int setIndex) const {
        uint8_t b = bits[setIndex];
        if (blocksperset == 2) 
        {
            return (b & 0x1) ? 0 : 1;
        }
        else if (blocksperset == 4) {
            int b0 = (b >> 0) & 1;
            int b1 = (b >> 1) & 1;
            int b2 = (b >> 2) & 1;
            if (b0 == 0) return (b1 == 0) ? 0 : 1;
            else         return (b2 == 0) ? 2 : 3;
        }
        else if (blocksperset == 8) {
            int b0 = (b >> 0) & 1, b1 = (b >> 1) & 1, b2 = (b >> 2) & 1;
            int b3 = (b >> 3) & 1, b4 = (b >> 4) & 1;
            int b5 = (b >> 5) & 1, b6 = (b >> 6) & 1;
            if (b0 == 0) {
                if (b1 == 0) return (b3 == 0) ? 0 : 1;
                else         return (b4 == 0) ? 2 : 3;
            } else {
                if (b2 == 0) return (b5 == 0) ? 4 : 5;
                else         return (b6 == 0) ? 6 : 7;
            }
        }
        return 0;
    }

    ~PLRUTable() { delete[] bits; }
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