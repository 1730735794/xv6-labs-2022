#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  int start_address,n;
  argint(0, &start_address);
  argint(1, &n);
  struct proc *pr = myproc();
  pagetable_t pagetable = pr->pagetable;
  uint64 p;
  argaddr(2, &p);
  unsigned int res = 0;
  for(int i = 0; i < n; i++){
     pte_t *pte = walk(pagetable, start_address + i * PGSIZE, 0);
    // If the user enters a very large n
    // it will cause `start_address+i*PGSIZE` to be an illegal address
    // case `pte` is 0
    if(pte && (*pte & PTE_A)){
      *pte -= PTE_A;
      res |= 1 << i;
    }
  }
  copyout(pagetable, p, (char *)&res, 4);
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
