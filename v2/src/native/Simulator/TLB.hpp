#ifndef TLB_HPP
#define TLB_HPP

#include <cstdint>
#include <vector>

struct TLBEntry {
    bool valid = false;
    bool dirty = false;
    uint32_t vpn = 0;
    uint32_t pfn = 0;
    uint64_t lastUsed = 0;    // for LRU replacement
};

struct TLB {
    int numEntries;
    std::vector<TLBEntry> entries;
    uint64_t timer = 0;   // increments on every access

    // stats
    int hits   = 0;
    int misses = 0;

    void init(int n) {
        numEntries = n;
        entries.resize(n);
    }

    void ground() {
        for(auto& e : entries) e = TLBEntry{};
        timer = 0;
        hits = misses = 0;
    }

    // Returns pfn if hit, -1 if miss
    int lookup(uint32_t vpn) {
        timer++;
        for(auto& e : entries) {
            if(e.valid && e.vpn == vpn) {
                e.lastUsed = timer;
                hits++;
                return (int)e.pfn;
            }
        }
        misses++;
        return -1;
    }

    // Insert translation, evict LRU if full
    void insert(uint32_t vpn, uint32_t pfn, bool dirty) {
        // Check if already present (update)
        for(auto& e : entries) {
            if(e.valid && e.vpn == vpn) {
                e.pfn      = pfn;
                e.dirty    = dirty;
                e.lastUsed = timer;
                return;
            }
        }
        // Find empty slot first
        for(auto& e : entries) {
            if(!e.valid) {
                e = {true, dirty, vpn, pfn, timer};
                return;
            }
        }
        // Evict LRU
        TLBEntry* lru = &entries[0];
        for(auto& e : entries)
            if(e.lastUsed < lru->lastUsed) lru = &e;
        *lru = {true, dirty, vpn, pfn, timer};
    }

    // Invalidate a specific VPN (called during page eviction)
    void invalidate(uint32_t vpn) {
        for(auto& e : entries)
            if(e.valid && e.vpn == vpn) e.valid = false;
    }

    // Mark a VPN dirty in TLB (called on store hit)
    void setDirty(uint32_t vpn) {
        for(auto& e : entries)
            if(e.valid && e.vpn == vpn) e.dirty = true;
    }
};

#endif