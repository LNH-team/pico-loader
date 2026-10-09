"""Emulate built split helper guards/state; hardware SD and peer are not simulated."""
import os,subprocess,struct
from pathlib import Path
from unicorn import Uc,UC_ARCH_ARM,UC_MODE_ARM,UC_HOOK_CODE
from unicorn.arm_const import *
root=Path(__file__).resolve().parents[1];elf=root/'build/picoLoader9_DSPICO.elf';prefix=os.environ.get('ARM_NONE_EABI_PREFIX', str(Path(os.environ.get('WONDERFUL_TOOLCHAIN', '/opt/wonderful')) / 'toolchain/gcc-arm-none-eabi/bin/arm-none-eabi-'))
syms={}
for line in subprocess.check_output([prefix+'nm',str(elf)],text=True).splitlines():
 p=line.split()
 if len(p)==3:syms[p[2]]=int(p[0],16)
secs=['patch_waitsplit_probe','patch_waitsplit_resume'];bases=[0x02010000,0x02010200];blobs=[]
for sec in secs:
 f=root/'build'/f'{sec}.bin';subprocess.run([prefix+'objcopy','--dump-section',sec+'='+str(f),str(elf)],check=True);blobs.append(bytearray(f.read_bytes()))
def ptr(sym,index):return bases[index]+(syms[sym]&~1)-syms['__start_'+secs[index]]
def patch(sym,index,value):struct.pack_into('<I',blobs[index],syms[sym]-syms['__start_'+secs[index]],value)
fix,takeover,done=0x02020000,0x02020100,0x02020200
patch('waitSplitRequestAddress',0,ptr('waitSplitRequestEntry',1))
patch('waitSplitReturnAddress',0,ptr('waitSplitReturnEntry',1))
patch('waitSplitFix',1,fix|1);patch('waitSplitTakeover',1,takeover|1)
regs=[UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_R2,UC_ARM_REG_R3,UC_ARM_REG_R4,UC_ARM_REG_R5,UC_ARM_REG_R6,UC_ARM_REG_R7,UC_ARM_REG_R8,UC_ARM_REG_R9,UC_ARM_REG_R10,UC_ARM_REG_R11,UC_ARM_REG_R12]
for name,mode,request,busy in [('no_request',0x1f,0,0),('busy_card',0x1f,14,0x80000000),('irq_context',0x12,14,0),('svc_context',0x13,14,0),('takeover',0x1f,14,0)]:
 u=Uc(UC_ARCH_ARM,UC_MODE_ARM);u.mem_map(0x02000000,0x400000);u.mem_map(0x04000000,0x2000)
 for base,b in zip(bases,blobs):u.mem_write(base,bytes(b))
 for addr in [fix,takeover,done]:u.mem_write(addr,b'\x70\x47')
 u.mem_write(0x04000180,struct.pack('<H',request));u.mem_write(0x040001a4,struct.pack('<I',busy));u.mem_write(0x04000208,struct.pack('<I',1))
 initial=0xA0000000|mode;u.reg_write(UC_ARM_REG_CPSR,initial);u.reg_write(UC_ARM_REG_SP,0x023ff000);u.reg_write(UC_ARM_REG_LR,done|1)
 for i,reg in enumerate(regs):u.reg_write(reg,0x12340000+i)
 before=[u.reg_read(reg) for reg in regs];hits=[]
 def code(uc,addr,size,data):
  if addr==fix: hits.append('fix');uc.reg_write(UC_ARM_REG_PC,uc.reg_read(UC_ARM_REG_LR))
  if addr in [takeover,done]:hits.append('takeover' if addr==takeover else 'return');uc.emu_stop()
 u.hook_add(UC_HOOK_CODE,code);u.emu_start(ptr('waitSplitProbeEntry',0),0,count=1000)
 if name=='takeover':
  assert hits==['fix','takeover'],hits
  assert struct.unpack('<I',u.mem_read(0x04000208,4))[0]==0
  assert u.reg_read(UC_ARM_REG_SP)==0x023ff000-32-24
 else:
  assert hits==['return'],hits
  assert [u.reg_read(reg) for reg in regs]==before
  assert u.reg_read(UC_ARM_REG_SP)==0x023ff000
  assert u.reg_read(UC_ARM_REG_LR)==done|1
  assert u.reg_read(UC_ARM_REG_CPSR)&0xf000001f==initial
  assert struct.unpack('<I',u.mem_read(0x04000208,4))[0]==1
 print('PASS',name)
assert all(len(b)+4<=124 for b in blobs)
print('Split helper sizes:',*[len(b) for b in blobs])
