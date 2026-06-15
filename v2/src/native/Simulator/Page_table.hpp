#ifndef PAGETABLE_HPP
#define PAGETABLE_HPP

#include <cstdint>
#include <unordered_map>

struct PageTableEntry {
    bool     valid     = false;
    bool     dirty     = false;
    uint32_t pfn       = 0;
    int      swapSlot  = -1;    // ADD — -1 means not in swap
};

struct PageTable {
    std::unordered_map<uint32_t, PageTableEntry> entries;

    void ground() { entries.clear(); }

    int lookup(uint32_t vpn) {
        auto it = entries.find(vpn);
        if(it != entries.end() && it->second.valid)
            return (int)it->second.pfn;
        return -1;
    }

    void insert(uint32_t vpn, uint32_t pfn, bool dirty = false) {
        auto& e  = entries[vpn];
        e.valid    = true;
        e.dirty    = dirty;
        e.pfn      = pfn;
        e.swapSlot = -1;
    }

    void invalidate(uint32_t vpn) {
        auto it = entries.find(vpn);
        if(it != entries.end()) it->second.valid = false;
    }

    void setDirty(uint32_t vpn) {
        auto it = entries.find(vpn);
        if(it != entries.end()) it->second.dirty = true;
    }

    // ADD these three:
    void setSwapSlot(uint32_t vpn, int slot) {
        entries[vpn].swapSlot = slot;
        entries[vpn].valid    = false;
    }

    int getSwapSlot(uint32_t vpn) {
        auto it = entries.find(vpn);
        if(it != entries.end()) return it->second.swapSlot;
        return -1;
    }

    void clearSwapSlot(uint32_t vpn) {
        auto it = entries.find(vpn);
        if(it != entries.end()) it->second.swapSlot = -1;
    }
};

#endif