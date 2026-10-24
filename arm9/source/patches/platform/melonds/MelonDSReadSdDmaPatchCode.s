.cpu arm946e-s
.section "melonds_readsddma", "ax"
.syntax unified
.thumb

// Lecture SD par DMA pour la plateforme MelonDS.
//
// Variante de melonds_readSd : au lieu de sonder DRQ et de recopier les mots
// avec le processeur, on programme un canal DMA en mode de demarrage carte et
// on arme l'interruption de fin de transfert. C'est ce qui permet aux jeux
// pilotant eux-memes la cartouche (moteur DMA propre, hors CARDi_*) de
// continuer a tourner : leur boucle est cadencee par cette interruption.
//
// L'emulation melonDS s'y prete : CartHomebrew::ROMCommandStart traite la
// commande C0 sans examiner AUXSPICNT, NDSCartSlot::Interface::RaiseDRQ appelle
// CheckDMA() sur donnee prete, et ROMEndTransfer leve l'IRQ si le bit 14
// d'AUXSPICNT est arme.
//
// r0 = secteur source
// r1 = secteur precedent (inutilise : pas de lecture sequentielle ici)
// r2 = canal DMA
// r3 = destination
.global melonds_readSdDma
.type melonds_readSdDma, %function
melonds_readSdDma:
    push {r4-r7,lr}
    movs r5, r3                 // r5 = destination (r3 sert de registre de travail)
    movs r6, r2                 // r6 = canal DMA

    ldr r4, =0x040001A0
    movs r3, #0xC0
    strb r3, [r4,#1]            // AUXSPICNT : slot actif + IRQ de fin de transfert

    // commande de lecture SD du secteur 0xaabbccdd : C0 aa bb cc dd 00 00 00
    movs r3, #0xC0
    strb r3, [r4,#0x8]
    lsrs r3, r0, #24
    strb r3, [r4,#0x9]
    lsrs r3, r0, #16
    strb r3, [r4,#0xA]
    lsrs r3, r0, #8
    strb r3, [r4,#0xB]
    lsls r7, r0, #24
    lsrs r7, r7, #24            // r7 = dd
    str r7, [r4,#0xC]           // en petit-boutiste, l'octet bas vient en premier

    // programme le DMA avant de lancer le transfert :
    // MIi_CardDmaCopy32(canal, 0x04100010, destination, 512)
    movs r0, r6
    ldr r1, =0x04100010
    movs r2, r5
    movs r3, #1
    lsls r3, r3, #9             // 512 octets
    ldr r7, melonds_readsddma_miiCardDmaCopy32Ptr
    blx r7

    // lance le transfert : le DMA se declenche sur DRQ, l'IRQ arrive a la fin
    ldr r3, =0xA1586000
    str r3, [r4,#4]

    pop {r4-r7,pc}

// Fin de lot : la plateforme MelonDS ne gere pas de lecture sequentielle,
// il n'y a donc rien a clore. Presente pour satisfaire l'interface.
.global melonds_readSdDmaFinish
.type melonds_readSdDmaFinish, %function
melonds_readSdDmaFinish:
    bx lr

.balign 4
.global melonds_readsddma_miiCardDmaCopy32Ptr
melonds_readsddma_miiCardDmaCopy32Ptr:
    .word 0

.pool

.end
