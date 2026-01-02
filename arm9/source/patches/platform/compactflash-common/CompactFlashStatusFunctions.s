.cpu arm7tdmi
.syntax unified
.thumb

.section "cf_wait_functions", "ax"

.equ CF_STS_INSERTED, 0x50
.equ CF_STS_READY, 0x58
.equ CF_STS_BUSY, 0x80

.equ CF_CARD_TIMEOUT, 10000000

@ Waits until the card is ready to receive commands
@ bool cf_waitAvailableForCommands()
.global cf_waitAvailableForCommands
.type cf_waitAvailableForCommands, %function
cf_waitAvailableForCommands:
    push {r0-r4, lr}
    @ wait for card to finish previous commands
    ldr r1, =CF_CARD_TIMEOUT
    ldr r2, cf_waitFunctions_reg_cmd
    movs r4, r1
still_busy:
    ldrh r0, [r2]
    @ r0 & CF_STS_BUSY == 0
    lsls r0, #25
    bcc no_longer_busy
    subs r1, #1
    bne still_busy

no_longer_busy:
    @ wait for card to be ready for commands
    movs r3, CF_STS_INSERTED
1:
    ldrh r1, [r2]
    tst r1, r3
    @ timeout expired
    bne ready
    subs r4, #1
    @ not ready
    bne 1b

    @ timeout expired, return 0
    pop {r0-r4, pc}

@ Waits until the card is ready to write/return the next block
@ bool cf_waitCardNextBlockReady()
.global cf_waitCardNextBlockReady
.type cf_waitCardNextBlockReady, %function
cf_waitCardNextBlockReady:
    push {r0-r4, lr}
    ldr r0, =CF_CARD_TIMEOUT
    ldr r1, cf_waitFunctions_reg_cmd
1:
    ldrb r2, [r1]
    cmp r2, CF_STS_READY
    @ timeout expired
    beq ready
    subs r0, #1
    @ not ready
    bne 1b

    @ timeout expired, return 0
    pop {r0-r4, pc}

ready:
    movs r0, #1
    pop {r0-r4, pc}

.balign 4
.global cf_waitFunctions_reg_cmd
cf_waitFunctions_reg_cmd:
    .word 0
