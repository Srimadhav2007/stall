<<<<<<< Updated upstream
# [20/02/26]-[9:00PM]
=======
# stall
COA-2026 Project: CS24B025-CS24B006

# [Date-1]-[Time]
>>>>>>> Stashed changes
## Abehri 009
    - Discussed about the language to use for developing the simulator:
        1. C/c++ : More memory control, speed and efficiency, but need to write larger code and not good GUI support as we know
        2. Java: Good GUI and development support as we know, but too much to code in and it's been a while since we touched it
        3. Python: Too much memory consumption and comparitively slow, but easier to code, with lesser number of lines for a given task.
    - Finalized to using python.
    - Chose to stick on to terminal based development for now, later we thought we will decide if we will use GUI.

<<<<<<< Updated upstream
# [26/02/26]-[8:45AM]
=======
# [Date-2]-[Time]
>>>>>>> Stashed changes
## On the way to AB-1
    - Varshith said we focus on instructions which do not have any data dependency etc.
    - Madhav said we will start working on simulator which does not even have pipelining,no pipeline registers introduced yet, just through function calls, then upgrade to pipelining and then to forwarding,

<<<<<<< Updated upstream
# [03/03/26]-[9:00PM]
=======
# [Date-3]-[Time]
>>>>>>> Stashed changes
## Abheri 009
    - Brainstormed on ideas as to how to implement pipelining, in the process we felt that we should not have started with non-pipelined simulator.
    - Here too, Madhav first thought of calling function for a stage through another stage function, i.e WB() from MEM() etc.
    - Varshith said that does not look like pipelining, and for sometime insisted that we use Process() in python, which Madhav rejected saying that if the machine doesn't have enough cores, it will not work, and also we do not use threading too.
    - Decided to explore independently regarding the pipelining while trying to build the parsing logic, pipeline registers and the stage functions

<<<<<<< Updated upstream
# [04/03/26]-[7:45PM]
=======
# [Date-4]-[Time]
>>>>>>> Stashed changes
## South Mess
    - We recalled that we need to write the stages in reverse order in while loop, for which we saw the reason, decided on implementing it that way.
    - Now that the registers are ready too, we decided to start the implementation immediately, for r-type instructions.

<<<<<<< Updated upstream
# [05/03/26]-[9:30PM]
## Abheri 009
    - Madhav had second thoughts on implementing the project in C++ in combination with Java all of a sudden, well actually it has been in his mind for a while. That is because he thought that such design would leverage both the advantages of efficiency from C++ and application building from Java, and said that he had been writing C++ code for the same logic in parallel.
    - Varshith said that c++ would be good too.

# [06/03/26]-[6:00PM]
## Abheri 009
=======
# [Date-5]-[Time]
## Abheri 009
    - Madhav had second thoughts on implementing the project in C++ in combination with Java all of a sudden, well actually it has been in his mind for a while. That is because he thought that such design would leverage both the advantages of efficiency from C++ and application building from Java, and said that he had been writing C++ code for the same logic in parallel.
    - Varshith said that we should stick on to python for now.

# [Date-6]-[Time]
## SST Laboratory
>>>>>>> Stashed changes
    - Varshith decided that the config file shall be JSON file, and that it shall contain the latency of each execute and memory instruction, memory size in bytes, pipeline forwarding as a true/false value. We agreed upon that.