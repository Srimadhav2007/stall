#ifndef MMU_HPP
#define MMU_HPP

#include "TLB.hpp"
#include "Page_table.hpp"
#include <vector>
#include <unordered_map>
#include <cstring>

struct FrameInfo {
    bool     occupied = false;
    bool     dirty    = false;
    uint32_t vpn      = 0;
    uint64_t lastUsed = 0;
};

struct MMU {
    TLB                                             dtlb;
    PageTable                                       pageTable;
    std::vector<FrameInfo>                          frames;
    std::unordered_map<uint32_t, std::vector<char>> swapMap;

    char* ram;        // points to Core's char* memory

    int      pageSize;
    int      numFrames;
    int      offsetBits;
    uint64_t timer = 0;

    int tlbHitLat    = 1;
    int pageWalkLat  = 10;
    int pageFaultLat = 50;
    int swapLat      = 500;

    int stallsAccumulated = 0;

    // stats
    int tlbHits          = 0;
    int tlbMisses        = 0;
    int pageFaults       = 0;
    int evictions        = 0;
    int dirtyWritebacks  = 0;
    int swapIns          = 0;
    int swapOuts         = 0;
    int totalTranslationPenalty = 0;

    void init(int physSize, int pgSize, int tlbEntries,
              int thl, int pwl, int pfl, int swl, char* ramPtr) {
        pageSize     = pgSize;
        numFrames    = physSize / pgSize;
        tlbHitLat    = thl;
        pageWalkLat  = pwl;
        pageFaultLat = pfl;
        swapLat      = swl;
        ram          = ramPtr;

        offsetBits = 0;
        int tmp = pgSize;
        while(tmp > 1){ offsetBits++; tmp >>= 1; }

        dtlb.init(tlbEntries);
        frames.resize(numFrames);
    }

    void ground() {
        dtlb.ground();
        pageTable.ground();
        swapMap.clear();
        for(auto& f : frames) f = FrameInfo{};
        timer = stallsAccumulated = 0;
        tlbHits = tlbMisses = pageFaults = 0;
        evictions = dirtyWritebacks = swapIns = swapOuts = 0;
        totalTranslationPenalty = 0;
    }

    int getFreeFrame() {
        for(int i = 0; i < numFrames; i++)
            if(!frames[i].occupied) return i;
        return -1;
    }

    int evictLRU(uint32_t& evictedVPN, bool& wasDirty) {
        int lru = 0;
        for(int i = 1; i < numFrames; i++)
            if(frames[i].lastUsed < frames[lru].lastUsed) lru = i;
        evictedVPN = frames[lru].vpn;
        wasDirty   = frames[lru].dirty;
        evictions++;
        if(wasDirty) dirtyWritebacks++;
        frames[lru].occupied = false;
        return lru;
    }

    uint32_t translate(uint32_t va, bool isWrite) {
        stallsAccumulated = 0;
        timer++;

        uint32_t vpn    = va >> offsetBits;
        uint32_t offset = va & (uint32_t)(pageSize - 1);

        // Step 1 — TLB lookup
        int pfn = dtlb.lookup(vpn);
        if(pfn != -1) {
            tlbHits++;
            stallsAccumulated += tlbHitLat;
            if(isWrite){
                dtlb.setDirty(vpn);
                pageTable.setDirty(vpn);
                frames[pfn].dirty = true;
            }
            frames[pfn].lastUsed = timer;
            totalTranslationPenalty += stallsAccumulated;
            return (uint32_t)pfn * pageSize + offset;        
        }

        // Step 2 — TLB miss, page table walk
        tlbMisses++;
        stallsAccumulated += pageWalkLat;
        pfn = pageTable.lookup(vpn);
        if(pfn != -1) {
            dtlb.insert(vpn, (uint32_t)pfn, isWrite);
            if(isWrite){
                pageTable.setDirty(vpn);
                frames[pfn].dirty = true;
            }
            frames[pfn].lastUsed = timer;
            totalTranslationPenalty += stallsAccumulated;
            return (uint32_t)pfn * pageSize + offset;        
        }

        pageFaults++;
        stallsAccumulated += pageFaultLat;

        int frame = getFreeFrame();

        if(frame == -1) {
            uint32_t evictedVPN;
            bool wasDirty;
            frame = evictLRU(evictedVPN, wasDirty);

            int frameBase = frame * pageSize;
            swapMap[evictedVPN].assign(
                ram + frameBase,
                ram + frameBase + pageSize
            );
            swapOuts++;
            stallsAccumulated += swapLat;

            pageTable.invalidate(evictedVPN);
            pageTable.setSwapSlot(evictedVPN, 1);
            dtlb.invalidate(evictedVPN);
        }

        // Step 4 — load page into frame (from swap or zero-fill)
        int frameBase = frame * pageSize;
        auto it = swapMap.find(vpn);
        if(it != swapMap.end()) {
            memcpy(ram + frameBase, it->second.data(), pageSize);
            swapMap.erase(it);
            pageTable.clearSwapSlot(vpn);
            swapIns++;
            stallsAccumulated += swapLat;
        } else {
            memset(ram + frameBase, 0, pageSize);
        }

        // Step 5 — install mapping
        frames[frame] = {true, isWrite, vpn, timer};
        pageTable.insert(vpn, (uint32_t)frame, isWrite);
        dtlb.insert(vpn, (uint32_t)frame, isWrite);
        totalTranslationPenalty += stallsAccumulated;
        return (uint32_t)frame * pageSize + offset;
    }
};

#endif