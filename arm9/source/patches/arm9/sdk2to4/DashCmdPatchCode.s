.cpu arm946e-s
.section "dash_cmd_fix", "ax"
.syntax unified
.arm

// Pokemon Dash (APDP) - lecture carte redirigee vers la SD.
//
// Point d'accroche : 0x0206D3A4, la ou le moteur propre a Dash assemble la
// commande carte 0xB7. On y substitue une lecture SD, dans le contexte
// d'execution de Dash lui-meme.
//
//   ctx   = [dbr_global_ptr]        (0x020D6144 = valeur du pool 0x0206D470)
//   dst   = [ctx+0x30] - 0x200      (destination du secteur courant)
//   count = [ctx+0x34] + 1          (secteurs restants + le courant)
//
// Zero pile : la pile IRQ de Dash est trop petite pour un push, lr est
// sauvegarde dans la section elle-meme. Offsets de pool calcules par
// l'assembleur via des etiquettes symboliques.
//
// v19 : identique a v11e, sauf que dbr_sdread pointe desormais vers une lecture
// SD *par DMA* (dash_readSdDma ci-dessous) au lieu d'une copie processeur. Le
// transfert DMA regenere l'IRQ carte que le moteur de Dash attend.

.global dash_cmd_patch_entry
.type dash_cmd_patch_entry, %function
dash_cmd_patch_entry:
    str  lr,  saved_lr
    mov  r12, r0                 // r12 = offset ROM
    ldr  r3,  dbr_fixcp15
    blx  r3
    mov  r0,  r12
    ldr  r3,  dbr_remap
    blx  r3                      // r0 = secteur SD
    mov  r12, r0
    ldr  r0,  dbr_global_ptr
    ldr  r0,  [r0]               // r0 = ctx
    ldr  r1,  [r0, #0x30]        // r1 = destination avancee
    ldr  r2,  [r0, #0x34]        // r2 = secteurs restants
    sub  r1,  r1, #0x200         // r1 = destination courante
    add  r2,  r2, #1             // r2 = total a lire (ignore par la lecture DMA)
    mov  r0,  r12                // r0 = secteur SD
    ldr  r3,  dbr_sdread
    blx  r3
    ldr  pc,  saved_lr

.balign 4
saved_lr:         .word 0
.global dbr_fixcp15
dbr_fixcp15:      .word 0
.global dbr_remap
dbr_remap:        .word 0
.global dbr_global_ptr
dbr_global_ptr:   .word 0
.global dbr_sdread
dbr_sdread:       .word 0

.pool

// ---------------------------------------------------------------------------
// Lecture SD d'UN secteur par DMA, autonome (aucune dependance a
// MIi_CardDmaCopy32 ni a la chaine DMA de CardiTryReadCardDmaPatch).
//
//   r0 = secteur SD, r1 = destination, r2 = count (ignore : un secteur)
//
// Programme le canal DMA 0 avec le mot de controle propre de Dash (0xAF000001,
// releve dans son pool en 0x0206D4A8 : mode carte DS, 32 bits, source fixe,
// repetition, 1 mot par declenchement), arme AUXSPICNT bit 14, poste la
// commande C0+secteur, puis lance ROMCTRL. La fin de transfert leve l'IRQ
// carte (IF bit 19) que le handler de Dash (0x0206D1D8) attend.
// ---------------------------------------------------------------------------
.section "dash_readsddma", "ax"
.syntax unified
.arm

.global dash_readSdDma
.type dash_readSdDma, %function
dash_readSdDma:
    ldr  r3,  dsr_card_base      // r3 = 0x040001A0
    ldr  r12, dsr_dataport       // r12 = 0x04100010 (port de donnees carte)
    str  r12, [r3, #-0xF0]       // DMASAD (0x040000B0) : source fixe
    str  r1,  [r3, #-0xEC]       // DMADAD (0x040000B4) : destination
    ldr  r12, dsr_dmactrl        // r12 = 0xAF000001
    str  r12, [r3, #-0xE8]       // DMACNT (0x040000B8) : arme le DMA
    mov  r12, #0xC0
    strb r12, [r3, #1]           // AUXSPICNT octet haut -> 0xC000
    strb r12, [r3, #8]           // cmd[0] = C0
    mov  r12, r0, lsr #24
    strb r12, [r3, #9]           // cmd[1] = secteur >> 24
    mov  r12, r0, lsr #16
    strb r12, [r3, #0xA]         // cmd[2]
    mov  r12, r0, lsr #8
    strb r12, [r3, #0xB]         // cmd[3]
    and  r12, r0, #0xFF
    str  r12, [r3, #0xC]         // cmd[4..7] = dd 00 00 00
    ldr  r12, dsr_romctrl        // 0xA1586000
    str  r12, [r3, #4]           // ROMCTRL : demarrage du transfert
    bx   lr

.balign 4
dsr_card_base:    .word 0x040001A0
dsr_dataport:     .word 0x04100010
dsr_dmactrl:      .word 0xAF000001
dsr_romctrl:      .word 0xA1586000

.pool
.end
