# RISC-V32 Simulator

A configurable RISC-V32 simulator implemented in **C++** with a **Java Swing GUI**. The simulator models instruction execution, memory hierarchy behavior, and cache performance, allowing users to experiment with architectural parameters and observe their impact on execution.

## Overview

This project simulates a subset of the RISC-V32 ISA using a custom 30-bit instruction encoding format. The simulation core is implemented in C++ for performance, while the user interface is developed in Java and communicates with the simulator through a native bridge.

The simulator supports configurable instruction latencies, cache parameters, and memory timings through a JSON configuration file.

## Project Variants

### Version 1 (v1)

Implements a two-level cache hierarchy:

- L1 Instruction Cache (L1I)
- L1 Data Cache (L1D)
- Unified L2 Cache

This version is intended for studying cache hierarchy behavior and the impact of cache configurations on execution performance.

### Version 2 (v2)

Implements:

- Single-level cache
- Virtual memory support

This version focuses on memory translation and address mapping mechanisms in addition to cache simulation.

## Supported Instructions

The simulator supports the following 26 RISC-V instructions:

### Arithmetic Instructions

- `add`
- `sub`
- `mul`
- `div`
- `addi`

### Data Movement Instructions

- `li`
- `la`
- `lw`
- `sw`

### Branch and Jump Instructions

- `beq`
- `bne`
- `beqz`
- `bnez`
- `j`
- `jal`

### Logical Instructions

- `and`
- `or`
- `xor`
- `andi`
- `ori`
- `xori`

### Shift Instructions

- `sll`
- `srl`
- `slli`
- `srli`

### Comparison Instructions

- `slt`

## Instruction Encoding

Instructions are internally represented using a custom **30-bit encoded format** optimized for simulation purposes.

The simulator translates supported assembly instructions into this encoded representation before execution.

## Configurable Parameters

The simulator behavior can be customized through the `config.json` file.

### Instruction Latencies

Execution latency can be specified for each instruction type.

Example:

```json
{
    "add": 1,
    "sub": 1,
    "mul": 3,
    "div": 1
}
```

### Memory Parameters

Users can configure:

- Main memory latency
- Memory size

### Cache Parameters

Users can configure:
- Cache size
- Block size
- Associativity
- Cache latency
- Replacement policy(LRU or pseudo-LRU)

This allows experimentation with different architectural designs and performance trade-offs.

## Features

- RISC-V32 instruction simulation
- Configurable instruction latencies
- Configurable cache hierarchy
- Memory hierarchy simulation
- Virtual memory support (v2)
- Java Swing graphical interface
- Native C++ simulation backend
- JSON-based configuration system
- Custom instruction encoding
- Performance analysis through configurable architectural parameters