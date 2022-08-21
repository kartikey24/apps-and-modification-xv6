#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "processInfo.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int 
sys_numvp(void)
{
  struct proc *p=myproc();
  return PGROUNDUP(p->sz)/PGSIZE;
}

int 
sys_numpp(void)
{
  uint count=0;
  struct proc *p=myproc();
  pde_t *pgdir=p->pgdir;
  uint i;
  for(i=0;i<KERNBASE;i+=PGSIZE)
  {
    pde_t *pde=&pgdir[PDX(i)];
    if((*pde & PTE_P)) //if page directory entry is valid
    {
      pte_t *pgtab = (pte_t*)P2V(PTE_ADDR(*pde));    // getting pagetable by value of the page dir entry
      if((pgtab[PTX(i)] & PTE_P))// && (pgtab[PTX(i)] & PTE_U))           // getting the address value of the page of the virtual address i
      count++;
    }
  }
  return count;
    
}

int
sys_mmap(void)
{
  int size;
  if(argint(0,&size)<0)
  return 0;
  if(size<=0||size%PGSIZE!=0)
  return 0;
  uint numpages=size/PGSIZE;
  int p=mmap_helper(numpages);
  if(p==-1)
  return 0;
  return p;  
}