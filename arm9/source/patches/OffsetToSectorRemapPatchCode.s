.cpu arm7tdmi
.section "offsettosectorremap", "ax"
.syntax unified
.thumb

// r0 = rom offset
// returns sd sector in r0
// returns remaining sectors in cluster in lr
.global offset_to_sector_remap
.type offset_to_sector_remap, %function
offset_to_sector_remap:
    push {lr}
    lsrs r0, r0, #9 // byte address to sector address
    movs r1, #1
    mov lr, r1 // read at most 1 sector at a time
    pop {pc}

.balign 4

.pool

.end