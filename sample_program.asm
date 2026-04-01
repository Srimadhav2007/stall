.data
arr: .word 5 3 8 1 2
n:   .word 5

.text
la x10 arr
la x11 n
lw x11 0(x11)

li x1 0

outer_loop:
sub x12 x11 x1
addi x12 x12 -1
beqz x12 end_sort

li x2 0

inner_loop:
sub x13 x12 x2
beqz x13 next_outer

li x14 4
mul x14 x2 x14
add x15 x10 x14

lw x3 0(x15)
lw x4 4(x15)

slt x5 x4 x3
beqz x5 no_swap

sw x4 0(x15)
sw x3 4(x15)

no_swap:
addi x2 x2 1
j inner_loop

next_outer:
addi x1 x1 1
j outer_loop

end_sort:
add x0 x0 x0