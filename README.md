
# [20/02/26]-[9:00PM]
## Abehri 009
    - Discussed about the language to use for developing the simulator:
        1. C/c++ : More memory control, speed and efficiency, but need to write larger code and not good GUI support as we know
        2. Java: Good GUI and development support as we know, but too much to code in and it's been a while since we touched it
        3. Python: Too much memory consumption and comparitively slow, but easier to code, with lesser number of lines for a given task.
    - Finalized to using python.
    - Chose to stick on to terminal based development for now, later we thought we will decide if we will use GUI.


# [26/02/26]-[8:45AM]

## On the way to AB-1
    - Varshith said we focus on instructions which do not have any data dependency etc.
    - Madhav said we will start working on simulator which does not even have pipelining,no pipeline registers introduced yet, just through function calls, then upgrade to pipelining and then to forwarding,


# [03/03/26]-[9:00PM]

## Abheri 009
    - Brainstormed on ideas as to how to implement pipelining, in the process we felt that we should not have started with non-pipelined simulator.
    - Here too, Madhav first thought of calling function for a stage through another stage function, i.e WB() from MEM() etc.
    - Varshith said that does not look like pipelining, and for sometime insisted that we use Process() in python, which Madhav rejected saying that if the machine doesn't have enough cores, it will not work, and also we do not use threading too.
    - Decided to explore independently regarding the pipelining while trying to build the parsing logic, pipeline registers and the stage functions


# [04/03/26]-[7:45PM]

## South Mess
    - We recalled that we need to write the stages in reverse order in while loop, for which we saw the reason, decided on implementing it that way.
    - Now that the registers are ready too, we decided to start the implementation immediately, for r-type instructions.


# [05/03/26]-[9:30PM]
## Abheri 009
    - Madhav had second thoughts on implementing the project in C++ in combination with Java all of a sudden, well actually it has been in his mind for a while. That is because he thought that such design would leverage both the advantages of efficiency from C++ and application building from Java, and said that he had been writing C++ code for the same logic in parallel.
    - Varshith said that c++ would be good too.


# [07/03/26]-[6:00PM]
## SST Laboratory
    - Varshith decided that the config file shall be JSON file, and that it shall contain the latency of each execute and memory instruction, memory size in bytes, pipeline forwarding as a true/false value. We agreed upon that.


# [31/03/26]-[11:25AM]
## Virtual(Madhav was at home)
    - While brainstorming about ideas regarding implementing the cache, we came across an issue, that is, we implemented the IF stage to take string as an instruction. Complication that came with it is that unified L2 cache cannot store strings(as they can be of variable length)
    - Madhav proposed 2 ideas: one being converting the instructions into integers which involves changing the Core::execute,Core::IF,Core::IDRF functions and Core::IF_IDRF struct, but then we can store instruction just like we store data, i.e fixed number of bytes; while the second idea involves storing the index of the instruction only in cache as a 4 byte value, considering the index being in the cache as hit, else miss
    - After discussion, we both agreed to go with the first idea, and then for the format,first Madhav had this idea: the INT_MAX has 10 digits in decimal representation, we will use max of 9 digits dedicating 2 for Opcode,2 each for registers, and distributing rest among immediate/offset.
    - Varshith said that, as we are implementing using decimal representation, we can as well do using binary representation, more closer to the actual hardware. We agreed upon that, with considering INT_MAX as a syntax error, and for now we decided to use 30 bits because there are no func-3/func-7 etc that we need to worry about, because we just work with few instructions for which 6 bits are more than enough.

# [1/04/26]-[2:00PM]
## Virtual

    - We discussed the bug fixes for EX stage issue with variable latencies,
    - And we found another key bug, that is, when there is a loop that runs 4 instructions 10 times, the total instructions run is currently taken to be 4, but not 40. So we discussed that bug fix too
    - We decided to give pause to syntax error catches, because we felt it would be very complicated for now.
    - Along with LRU we decided to implement Pseudo-LRU as alternative Cache Replacement Policy.
    
