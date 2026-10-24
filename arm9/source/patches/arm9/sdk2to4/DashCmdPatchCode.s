.cpu arm946e-s
.section "dash_cmd_fix", "ax"
.syntax unified
.arm

// Pokemon Dash (APDP) - lecture carte redirigee vers la SD + protection FAT/FNT.
//
// Point d'accroche : 0x0206D3A4 (moteur propre a Dash, commande carte 0xB7).
//
// v19  : lecture SD par DMA (dash_readSdDma), regenere l'IRQ carte.
// v20b : AJOUT protection FAT/FNT. Les tables se chargent via CARDi en
//        0x23B5380 / 0x23B8358, mais id=5 (charge par CE moteur des l'offset ROM
//        0xD7000) se deverse en 0x23AEE60 et les RECOUVRE avant lecture par le
//        reader -> crash 0x663BEFBD. Correctif : la 1re fois que ce moteur lit
//        id=5 (r0=0xD7000), les tables sont encore intactes ; on les COPIE vers
//        une zone RAM libre VERIFIEE (0x02200000/0x02203000, hors portee d'id=5)
//        et on REPOINTE le flux dessus, AVANT de laisser id=5 se charger.
//        (Prouve en GDB : protect-fat-fnt2.gdb.)
//        NB : buffers en DUR (pas d'Alloc) -> aucune dependance au tas de patch,
//        qui est trop petit (0x6A0 o) pour 44 Ko.
//
// U (contrat amont) : la lecture SD n'est plus faite par une transaction carte
//        propre a Dash (dash_readSdDma, commande 0xC0, DMA0 : SUPPRIMEE). Le
//        trampoline appelle l'interface ReadSectorsDma de la PLATEFORME compilee
//        (M3DS : B1/B2/BA/B3 ; MELONDS : C0 + DMA ; autres : leur propre code),
//        exactement comme le loader le fait deja pour CARDi_SetCardDma (chemin
//        A, SDK). Le moteur de Dash n'arme plus son DMA et n'ecrit plus ROMCTRL :
//        la plateforme est le SEUL responsable de la transaction (voir
//        dash-ETAT-DES-FAITS.md Suites 377/379). Un crochet de fin de file
//        (dash_finish_entry) clot le lot comme CARDi_OnReadCard.
//
// Zero pile : pile IRQ trop petite ; lr et r0 sauves dans la section.

.global dash_cmd_patch_entry
.type dash_cmd_patch_entry, %function
dash_cmd_patch_entry:
    str  lr,  saved_lr

    // --- protection FAT/FNT, une seule fois, au debut d'id=5 (r0 = offset ROM) ---
    ldr  r1,  dbr_prot_done
    cmp  r1,  #0
    bne  1f
    ldr  r1,  dpr_id5_start      // 0x000D7000
    cmp  r0,  r1
    bne  1f
    mov  r1,  #1
    str  r1,  dbr_prot_done
    str  r0,  saved_r0           // preserver l'offset ROM (pas de pile)

    ldr  r2,  dpr_fnt_src        // 0x023B8358
    ldr  r3,  dpr_fnt_dst        // 0x02203000 (zone libre verifiee)
    ldr  r1,  dpr_fnt_words      // 0x0240 mots (0x900 : table + sous-table racine)
    bl   prot_copy_words

    ldr  r2,  dpr_fat_src        // 0x023B2340 (v20d: adresse FAT corrigee, etait 0x023B5380)
    ldr  r3,  dpr_fat_dst        // 0x02200000
    ldr  r1,  dpr_fat_words      // 0x0BF6 mots (FAT complete 0x2FD8)
    bl   prot_copy_words

    ldr  r1,  dpr_stream_fnt     // 0x020DA3FC = &[stream+0x28]
    ldr  r2,  dpr_fnt_dst
    str  r2,  [r1]
    ldr  r1,  dpr_stream_fat     // 0x020DA3F4 = &[stream+0x20]
    ldr  r2,  dpr_fat_dst
    str  r2,  [r1]

    ldr  r0,  saved_r0           // restaurer l'offset ROM
1:
    mov  r12, r0                 // r12 = offset ROM
    ldr  r3,  dbr_fixcp15
    blx  r3
    mov  r0,  r12
    ldr  r3,  dbr_remap
    blx  r3                      // r0 = secteur SD
    mov  r12, r0
    str  r12, saved_sector       // secteur SD (pas de pile : sauvegarde en section)

    // U : desarmer le canal DMA du moteur AVANT l'appel plateforme (equivalent de
    // ce que CARDi_OnReadCard fait pour le chemin A). Le moteur de Dash laisse le
    // canal arme (mode carte, mot repete) entre deux secteurs d'une meme file :
    // MIi_CardDmaCopy32 attendrait indefiniment le bit 31 du canal, et la phase
    // commande M3 (sondage 0xA7586000) verrait ses mots d'etat captes par le DMA.
    ldr  r0,  dpr_chan_ptr       // 0x020D613C : numero du canal DMA du moteur (= 2)
    ldr  r0,  [r0]
    ldr  r3,  dpr_mistopdma      // 0x0209DA10 : MI_StopDma (celle du SDK de Dash)
    blx  r3                      // r4 (destination) est preserve (callee-saved)

    // ReadSectorsDma(srcSector, previousSrcSector, dmaChannel, dst), comme
    // patch_cardisetcarddma_entry (CardiSetCardDmaPatchCode.s) : previousSector
    // est memorise AVANT l'appel ; 0 = premier secteur d'un lot.
    ldr  r0,  saved_sector
    ldr  r1,  dbr_prev
    str  r0,  dbr_prev
    ldr  r2,  dpr_chan_ptr
    ldr  r2,  [r2]
    mov  r3,  r4                 // destination = r4 (v29 : valeur vivante au point d'accroche)
    ldr  r12, dbr_rsd
    blx  r12                     // fonction de la plateforme (Thumb) : rend la main
                                 // AVANT la fin ; DMA + IRQ de fin sont les siens
    ldr  pc,  saved_lr

// copie par mots : r2=src, r3=dst, r1=nb_mots ; scratch r0. Retour via lr.
.type prot_copy_words, %function
prot_copy_words:
    cmp  r1,  #0
    bxeq lr
2:
    ldr  r0,  [r2], #4
    str  r0,  [r3], #4
    subs r1,  r1, #1
    bne  2b
    bx   lr

// Crochet de fin de file : remplace l'unique instruction `bl OS_DisableIrqMask`
// de 0x0206D2A4 (r0 = 0x80000 deja charge par le moteur en 0x0206D29C).
// Equivalent de patch_cardionreadcard_entry : cloture du lot cote plateforme
// (M3 : B3), remise a zero de previousSector, puis OS_DisableIrqMask(0x80000).
// Rien n'est clos si ce moteur n'a lance aucune lecture (previousSector == 0).
.global dash_finish_entry
.type dash_finish_entry, %function
dash_finish_entry:
    str  lr,  saved_lr2
    ldr  r1,  dbr_prev
    cmp  r1,  #0
    beq  4f
    ldr  r3,  dbr_finish
    blx  r3                      // fin de lot de la plateforme (Thumb ou ARM)
    mov  r0,  #0
    str  r0,  dbr_prev
4:
    mov  r0,  #0x80000           // argument de OS_DisableIrqMask (instruction d'origine)
    ldr  r3,  dpr_osdisableirq   // 0x0209BA18
    ldr  lr,  saved_lr2
    bx   r3                      // appel terminal : retour direct au moteur

.balign 4
saved_lr:         .word 0
saved_lr2:        .word 0
saved_sector:     .word 0
saved_r0:         .word 0
.global dbr_fixcp15
dbr_fixcp15:      .word 0
.global dbr_remap
dbr_remap:        .word 0
.global dbr_global_ptr
dbr_global_ptr:   .word 0
.global dbr_rsd
dbr_rsd:          .word 0   // ReadSectorsDma de la plateforme
.global dbr_finish
dbr_finish:       .word 0   // fin de lot de la plateforme
dbr_prev:         .word 0   // previousSector de CE moteur (0 = aucun lot en cours)
dbr_prot_done:    .word 0

// constantes (specifiques Dash APDP, relevees + verifiees au runtime)
dpr_id5_start:    .word 0x000D7000
dpr_fnt_src:      .word 0x023B8358
dpr_fat_src:      .word 0x023B2340
dpr_fnt_dst:      .word 0x02205000
dpr_fat_dst:      .word 0x02200000
dpr_fnt_words:    .word 0x00001F1E
dpr_fat_words:    .word 0x00000BF6
dpr_stream_fnt:   .word 0x020DA3FC
dpr_stream_fat:   .word 0x020DA3F4
dpr_chan_ptr:     .word 0x020D613C   // canal DMA du moteur (init 0x0206D69C)
dpr_mistopdma:    .word 0x0209DA10   // MI_StopDma
dpr_osdisableirq: .word 0x0209BA18   // OS_DisableIrqMask

.pool

.end
