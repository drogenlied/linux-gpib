#include <ibsys.h>

struct semaphore osMutex = MUTEX;

void osLockMutex( void )
{
  DBGin("osLockMutex");
  DBGprint(DBG_BRANCH,("Locking Process %d ",current->pid));
  down(&osMutex);
  DBGout();
}

void osUnlockMutex( void )
{
  DBGin("osUnlockMutex");
  DBGprint(DBG_BRANCH,("Unlocking Process %d ",current->pid));
  up(&osMutex);
  DBGout();

}
