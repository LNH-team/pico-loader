"""Execute built ARM7 hotkey trigger; card reads must wait for acknowledgement."""
import os,subprocess,struct
from pathlib import Path
from unicorn import *
from unicorn.arm_const import *
root=Path(__file__).resolve().parents[1];elf=root/'build/picoLoader9_DSPICO.elf';prefix=os.environ.get('ARM_NONE_EABI_PREFIX', str(Path(os.environ.get('WONDERFUL_TOOLCHAIN', '/opt/wonderful')) / 'toolchain/gcc-arm-none-eabi/bin/arm-none-eabi-'))
syms={}
for line in subprocess.check_output([prefix+'nm',str(elf)],text=True).splitlines():
 p=line.split()
 if len(p)==3:syms[p[2]]=int(p[0],16)
f=root/'build/detect136.bin';subprocess.run([prefix+'objcopy','--dump-section','patch_retailhotkeydetect='+str(f),str(elf)],check=True)
base=0x03800000
addr=lambda n:base+(syms[n]&~1)-syms['__start_patch_retailhotkeydetect']
u=Uc(UC_ARCH_ARM,UC_MODE_THUMB);u.mem_map(base,0x10000);u.mem_map(0x04000000,0x2000);u.mem_write(base,f.read_bytes());u.reg_write(UC_ARM_REG_SP,0x0380ff00)
u.mem_write(addr('holdCounter'),struct.pack('<I',29));u.mem_write(0x04000130,b'\0\0')
hits=[];writes=[]
def code(uc,a,size,data):
 if a==addr('patch_retailhotkeydetect_dspico_loader_probe'):raise AssertionError('SD probe before ack')
 if a==addr('detection_done'):hits.append('done');uc.emu_stop()
u.hook_add(UC_HOOK_CODE,code)
u.hook_add(UC_HOOK_MEM_WRITE,lambda uc,a,ptr,size,val,data:writes.append((ptr,val)))
u.emu_start(addr('check_hotkey')|1,0,count=300)
assert hits==['done']
assert (0x04000180,0xe00) in writes
assert struct.unpack('<I',u.mem_read(addr('ackPending'),4))[0]==1
assert not any(0x04000198<=a<0x040001c0 for a,v in writes)
print('PASS request published with no card access before ARM9 ack')
