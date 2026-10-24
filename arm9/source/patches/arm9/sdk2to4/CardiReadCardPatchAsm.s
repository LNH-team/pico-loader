.cpu arm946e-s
.section "patch_cardireadcard", "ax"
.syntax unified

.thumb
.global patch_cardireadcard_entry
.type patch_cardireadcard_entry, %function
patch_cardireadcard_entry:

.global patch_cardireadcard_return_offset
patch_cardireadcard_return_offset:
    movs r0, #0x38
    add lr, r0
    push {r1,r2,r3,r4,r6,lr}
    ldr r3, __patch_cardireadcard_fix_cp15_asm_address
    blx r3
.global patch_cardireadcard_mov_src_to_r0
patch_cardireadcard_mov_src_to_r0:
    nop
    ldr r3, __patch_cardireadcard_rom_offset_to_sd_sector_asm_address
    blx r3
    movs r2, #1 // r2 = sector count
.global patch_cardireadcard_mov_dst_to_r1
patch_cardireadcard_mov_dst_to_r1:
    mov r1, r8 // r1 = dst
.global patch_cardireadcard_mov_cardicommon_to_r6
patch_cardireadcard_mov_cardicommon_to_r6:
    movs r6, r4 // r6 = cardi_common
.global patch_cardireadcard_adjust_cardicommon_offset
patch_cardireadcard_adjust_cardicommon_offset:
    subs r6, #0
    ldr r4, [r6, #0x20] // r4 = actual dst
    cmp r4, r1
    bne do_read // if dst != actual dst the cache is in use

    ldr r2, [r6, #0x24] // remaining length
    lsrs r2, r2, #9 // sectors left to read
    beq do_read_fresh // FIX-CACHE-STALE (03/09) : plus rien en attente malgre
                       // dst identique -> coincidence, pas une continuation
                       // reelle ; forcer une vraie lecture au lieu d'un
                       // "cache hit" fantome qui laisserait le tampon tel
                       // quel (rempli par un flux precedent, jamais efface)
    cmp r2, lr
    ble 1f
    mov r2, lr // clip to cluster boundary
1:
    subs r4, r2, #1
    lsls r4, r4, #9
    ldr r3, [r6, #0x1C]
    adds r3, r3, r4
    str r3, [r6, #0x1C]
    ldr r3, [r6, #0x20]
    adds r3, r3, r4
    str r3, [r6, #0x20]

.global patch_cardireadcard_mov_r3_to_dst
patch_cardireadcard_mov_r3_to_dst:
    mov r8, r3

    ldr r3, [r6, #0x24]
    subs r3, r3, r4
    str r3, [r6, #0x24]

    // PATCH v25 (correctif coalescing) : le lot de r2 secteurs vient d'etre
    // calcule et la comptabilite stream (position/dest/restant) mise a jour
    // en consequence -- il faut appeler melonds_readSd avec ce VRAI compte de
    // secteurs, pas avec le r2=1 pose plus bas pour l'autre cas
    // (do_read_fresh, atteint par 'beq' quand restant==0 malgre une
    // destination identique -- ce cas-la doit garder r2=1). Sans ce saut,
    // l'execution tombe (fall-through) dans do_read_fresh et son
    // 'movs r2,#1' ecrase le compte reel : un seul secteur est alors
    // physiquement lu depuis la carte SD alors que la comptabilite
    // stream/l'appelant croient le lot entier transfere.
    b do_read

do_read_fresh: // FIX-CACHE-STALE : remettre r2=1 (ecrase par lsrs ci-dessus) ;
               // r1=dst est deja correct (inchange depuis mov_dst_to_r1)
    movs r2, #1
do_read:
    lsrs r3, r1, #24 // if dst address is invalid (close to zero), ignore the read
    beq ignore_read  // this is intended to fix reads to null pointers that would be ignored if done with DMA

    ldr r3, __patch_cardireadcard_sdread_asm_address
    blx r3

ignore_read:
    pop {r1,r2,r3,r4,r6,pc}

.balign 4

.global __patch_cardireadcard_fix_cp15_asm_address
__patch_cardireadcard_fix_cp15_asm_address:
    .word 0

.global __patch_cardireadcard_rom_offset_to_sd_sector_asm_address
__patch_cardireadcard_rom_offset_to_sd_sector_asm_address:
    .word 0

.global __patch_cardireadcard_sdread_asm_address
__patch_cardireadcard_sdread_asm_address:
    .word 0

.pool

.end