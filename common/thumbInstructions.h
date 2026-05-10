#pragma once

#define THUMB_R0                    0
#define THUMB_R1                    1
#define THUMB_R2                    2
#define THUMB_R3                    3
#define THUMB_R4                    4
#define THUMB_R5                    5
#define THUMB_R6                    6
#define THUMB_R7                    7

#define THUMB_HI_R8                 8
#define THUMB_HI_R9                 9
#define THUMB_HI_R10                10
#define THUMB_HI_R11                11
#define THUMB_HI_R12                12
#define THUMB_HI_SP                 13
#define THUMB_HI_LR                 14
#define THUMB_HI_PC                 15

#define THUMB_LSLS_IMM(rd, rs, imm) (0x0000 | ((imm) << 6) | ((rs) << 3) | (rd))
#define THUMB_LSRS_IMM(rd, rs, imm) (0x0800 | ((imm) << 6) | ((rs) << 3) | (rd))
#define THUMB_ADDS_IMM(rd, rs, imm) (0x1C00 | ((imm) << 6) | ((rs) << 3) | (rd))
#define THUMB_SUBS_IMM(rd, rs, imm) (0x1E00 | ((imm) << 6) | ((rs) << 3) | (rd))
#define THUMB_MOVS_IMM(rd, imm)     (0x2000 | ((rd) << 8) | (imm))
#define THUMB_MOVS_REG(rd, rs)      THUMB_ADDS_IMM((rd), (rs), 0)
#define THUMB_MOV_HIREG(rd, rs)     (0x4600 | ((((rd) >> 3) & 1) << 7) | ((rs) << 3) | ((rd) & 7))
#define THUMB_NOP                   THUMB_MOV_HIREG(8, 8)
#define THUMB_BX(rs)                (0x4700 | ((rs) << 3))
#define THUMB_BLX(rs)               (0x4780 | ((rs) << 3))
#define THUMB_BX_LR                 THUMB_BX(THUMB_HI_LR)
#define THUMB_LDR_PC_IMM(rd, imm)   (0x4800 | ((rd) << 8) | (((imm) >> 2) & 0xFF))
#define THUMB_STR_IMM(rd, rb, imm)  (0x6000 | (((imm) >> 2) << 6) | ((rb) << 3) | (rd))
#define THUMB_STR_SP_IMM(rd, imm)   (0x9000 | ((rd) << 8) | (((imm) >> 2) & 0xFF))
#define THUMB_LDR_SP_IMM(rd, imm)   (0x9800 | ((rd) << 8) | (((imm) >> 2) & 0xFF))
#define THUMB_ADD_PC_IMM(rd, imm)   (0xA000 | ((rd) << 8) | (((imm) >> 2) & 0xFF))
#define THUMB_SVC(x)                (0xDF00 | (x))