#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/init.h"
#include "filesys/filesys.h"
#include <stdlib.h>
#include "lib/kernel/list.h"
#include "devices/input.h"
#include "process.h"
#include "lib/user/syscall.h"
#include "threads/synch.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"

struct fd_elem
{
  int fd;
  struct file *data;
  struct list_elem elem;
};

struct fd_elem* findFd(int fd){
  struct thread *thisThread = thread_current();
  struct list* threadList = &thisThread->fdList;

  struct list_elem *e;

  for (e = list_begin(threadList); e!= list_end(threadList); e = list_next(e)){
    struct fd_elem* iterFd = list_entry(e, struct fd_elem, elem);
    if(iterFd->fd == fd){
      return iterFd;
    }
  }
  exitProgram(-1);
}

void deleteList(){
  struct thread *thisThread = thread_current();
  struct list* threadList = &(thisThread->fdList);

  while(!list_empty(threadList)){
    struct list_elem *e = list_pop_front(threadList);
    struct fd_elem* iterFd = list_entry(e, struct fd_elem, elem);
    file_close(iterFd->data);
    free(iterFd);
  }

  return NULL;
}

/* for debugging */
void printList(struct list* printList){
  struct list_elem *e;

  printf("head: %p, tail: %p \n", &printList->head, &printList->tail);
  for (e = list_begin(printList); e!= list_end(printList); e = list_next(e)){
    struct fd_elem* iterFd = list_entry(e, struct fd_elem, elem);
    
    //printf("This is IN the list, fd: %d, fileP: %p, listElem: %p \n", iterFd->fd, iterFd->data, &iterFd->elem);
    printf("prev: %p, next: %p \n\n", iterFd->elem.prev, iterFd->elem.next);
  }
}

bool isValidAdr(const void* adr)
{
  return !(adr == NULL || !is_user_vaddr(adr) || pagedir_get_page(thread_current()->pagedir, adr) == NULL);
}

bool isValidBuffer(int size, void* buffer)
{
  //check boundaries of the buffer if they are in page etc.
  return isValidAdr(buffer) && isValidAdr(buffer + size);
}

bool isValidString(char* str)
{
  if(!isValidAdr(str)) return false;
  while(true) {
    str++;
    if(!isValidAdr(str)){
     return false;
    }
    if(*(str) == '\0'){
      return true;
    }
  }
}

void halt(void)
{
  power_off();
}

bool create(const char *file, unsigned initial_size)
{
  if(!isValidString(file))
  {
    exitProgram(-1);
  }

  off_t init_size = (off_t)initial_size;
  return filesys_create(file, init_size);
}

int open(const char *file)
{
  if(!isValidString(file))
  {
    exitProgram(-1);
  }

  struct thread *thisThread = thread_current();
  struct list* threadList = &thisThread->fdList;

  struct file *fileOpenResult = filesys_open(file);

  if (fileOpenResult != NULL && list_size(threadList) < 128)
  {
    struct fd_elem *newFd = (struct fd_elem *)malloc(sizeof(struct fd_elem));
    newFd->fd = thisThread->fdCount;
    newFd->data = fileOpenResult;
    thisThread->fdCount++;
    list_push_front(threadList, &newFd->elem);
    return newFd->fd;
  }
  return -1;
}

void close(int fd)
{
  struct fd_elem* fdElem = findFd(fd);
  if(fdElem == NULL){
    return;
  }
  list_remove(&fdElem->elem);
  file_close(fdElem->data);
  free(fdElem);
}

int read(int fd, void *buffer, unsigned size)
{
  if(!isValidBuffer(size, buffer))
  {
    exitProgram(-1);
  }

  switch(fd){
    /* Reads from keyboard */
    case 0:
      printf("");
      uint8_t* tempBuffer = (uint8_t*)buffer;
      for(int i = 0; i<size; i++){
        uint8_t key = input_getc();
        tempBuffer[i] = key;
      }
      return size;
      break;

    /* Not implemented */
    case 1:
      return -1;

    /* Reads from open file */
    default:
      printf("");
      struct fd_elem* fdElem = findFd(fd);
      if(fdElem == NULL){
        return -1;
      }
      return file_read(fdElem->data, buffer, size);
  }
}

int write(int fd, const void *buffer, unsigned size)
{
  if(!isValidBuffer(size, buffer))
  {
    exitProgram(-1);
  }
  if(fd == 0) exitProgram(-1);
  if(fd == 1) {
    putbuf(buffer, size);
    return (int) size;
  }
  else
  {
    struct file* wantedFile = findFd(fd)->data;
    if(wantedFile == NULL) exitProgram(-1);
    return file_write(wantedFile, buffer, (off_t)size);
  }
}

void exitProgram(int status)
{
  deleteList();
  thread_current()->parent->exit_status = status;
  printf("%s: exit(%d)\n", thread_current()->name, thread_current()->parent->exit_status);
  thread_exit();
}

pid_t exec (const char* cmd_line)
{
  if(!isValidString(cmd_line))
  {
    exitProgram(-1);
  }

  return process_execute(cmd_line);
}

int wait(pid_t cPid)
{
  return process_wait(cPid);
}

void seek(int fd, unsigned position){
  struct fd_elem* fdItem = findFd(fd);
  struct file* file = fdItem->data;

  int fileSize = file_length(file);
  if(fileSize <= position){
    file_seek(file, (off_t)fileSize);
  }
  else{
    file_seek(file, (off_t)position);
  }
}

unsigned tell(int fd){
  struct fd_elem* fdItem = findFd(fd);
  struct file* file = fdItem->data;

  return (unsigned)file_tell(file);
}

int filesize(int fd){
  struct fd_elem* fdItem = findFd(fd);
  struct file* file = fdItem->data;

  return (int)file_length(file);
}

bool remove(const char* file_name){
  if(!isValidString(file_name))
  {
    exitProgram(-1);
  }
  return filesys_remove(file_name);
}

static void syscall_handler(struct intr_frame *);

void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

int* getArg(int i, void* esp){
  int *arg = ((int *)esp + i);
  if(!isValidAdr(arg)) exitProgram(-1);
  return *arg;
}

static void
syscall_handler(struct intr_frame *f)
{
  if(!isValidAdr(f->esp)){
    exitProgram(-1);
  }

  bool isDebugging = false;
  int syscallNumber = *((int *)f->esp);
  int* arg1;
  int* arg2;
  int* arg3;

  if (isDebugging)
  {
    printf("system call number! %d \n", syscallNumber);
    printf("system call arg1 %d \n", arg1);
    printf("system call arg2 %d \n", arg2);
    printf("system call arg3 %d \n", arg3);
  }

  switch (syscallNumber)
  {

  /* Halts the program */
  case SYS_HALT:
    halt();
    break;

  /* Creates a new file */
  case SYS_CREATE:
    arg1 = getArg(1, f->esp);
    arg2 = getArg(2, f->esp);
    f->eax = (uint32_t)create((const char *)arg1, (unsigned)arg2);
    break;

  /* Opens a file */
  case SYS_OPEN:
    arg1 = getArg(1, f->esp);
    f->eax = (uint32_t)open((const char *)arg1);
    break;

  /* Opens a file. int fd, const void *buffer, unsigned size */
  case SYS_WRITE:
    arg1 = getArg(1, f->esp);
    arg2 = getArg(2, f->esp);
    arg3 = getArg(3, f->esp);
    f->eax = (uint32_t)write((int)arg1, (const void *)arg2, (unsigned)arg3);
    break;

  /* Reads a file */
  case SYS_READ:
    arg1 = getArg(1, f->esp);
    arg2 = getArg(2, f->esp);
    arg3 = getArg(3, f->esp);
    f->eax = (uint32_t)read((int)arg1, (void *)arg2, (unsigned)arg3);
    break;

  /* Closes a file */
  case SYS_CLOSE:
    arg1 = getArg(1, f->esp);
    close((int)arg1);
    break;

  case SYS_EXEC:
    arg1 = getArg(1, f->esp);
    f->eax = (uint32_t)exec((char*)arg1);
    break;

  case SYS_EXIT:
    arg1 = getArg(1, f->esp);
    exitProgram((int)arg1);
    break;

  case SYS_WAIT:
    arg1 = getArg(1, f->esp);
    f->eax = (uint32_t)wait((pid_t)arg1);
    break;

  case SYS_SEEK:
    arg1 = getArg(1, f->esp);
    arg2 = getArg(2, f->esp);
    seek((int)arg1, (unsigned)arg2);
    break;
  
  case SYS_TELL:
    arg1 = getArg(1, f->esp);
    f->eax = (uint32_t)tell((int)arg1);
    break;

  case SYS_FILESIZE:
    arg1 = getArg(1, f->esp);
    f->eax = (uint32_t)filesize((int)arg1);
    break;

  case SYS_REMOVE:
    arg1 = getArg(1, f->esp);
    f->eax = (uint32_t)remove((const char*)arg1);
    break;

  default:
    printf("this is syscall default, not good");
    break;
  }
}
