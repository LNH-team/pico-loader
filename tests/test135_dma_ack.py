"""Execute real relocated ack: DMA stops before VRAM writes, only on request."""
import os,subprocess,struct
from pathlib import Path
from unicorn import *
from unicorn.arm_const import *
root=Path(__file__).resolve().parents[1];elf=root/'build/picoLoader9_DSPICO.elf';prefix=os.environ.get('ARM_NONE_EABI_PREFIX', str(Path(os.environ.get('WONDERFUL_TOOLCHAIN', '/opt/wonderful')) / 'toolchain/gcc-arm-none-eabi/bin/arm-none-eabi-'))
syms={}
for line in subprocess.check_output([prefix+'nm',str(elf)],text=True).splitlines():
 p=line.split()
 if len(p)==3:syms[p[2]]=int(p[0],16)
f=root/'build/ack135.bin';subprocess.run([prefix+'objcopy','--dump-section','patch_split_ack='+str(f),str(elf)],check=True)
b=bytearray(f.read_bytes());base=0x02010000;end=0x02020000
struct.pack_into('<I',b,syms['splitAckNext']-syms['__start_patch_split_ack'],end|1)
assert len(b)<=120,len(b)
for request in (0,14):
 u=Uc(UC_ARCH_ARM,UC_MODE_THUMB);u.mem_map(0x02000000,0x400000);u.mem_map(0x04000000,0x2000);u.mem_write(base,bytes(b));u.mem_write(end,b'\x70\x47');u.reg_write(UC_ARM_REG_SP,0x023ff000)
 u.mem_write(0x023ff000,struct.pack('<6I',1,2,3,4,6,end|1));u.mem_write(0x04000180,struct.pack('<H',request))
 for n in range(4):u.mem_write(0x040000b8+12*n,struct.pack('<I',0x80000000))
 writes=[]
 u.hook_add(UC_HOOK_MEM_WRITE,lambda uc,a,addr,size,val,data:writes.append((addr,val)))
 u.hook_add(UC_HOOK_CODE,lambda uc,addr,size,data:uc.emu_stop() if addr==end else None)
 u.emu_start(base+(syms['splitAckEntry']&~1)-syms['__start_patch_split_ack']+1,0,count=200)
 dma=[x for x in writes if x[0] in [0x040000b8+12*n for n in range(4)]]
 if request:
  assert dma==[(0x040000b8+12*n,0) for n in range(4)],dma
  assert writes.index(dma[-1])<next(i for i,x in enumerate(writes) if x[0]==0x04000242)
 else:assert not dma and not any(a==0x04000242 for a,v in writes)
 print('PASS', 'requested DMA quiescence before VRAM' if request else 'normal read leaves DMA and VRAM untouched')
print('Ack size:',len(b))
