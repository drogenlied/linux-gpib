#include <ibsys.h>

DECLARE_MUTEX(osMutex);

IBLCL void osLockMutex( void )
{
  DBGin("osLockMutex");
  DBGprint(DBG_BRANCH,("Locking Process %d ",current->pid));
  down(&osMutex);
  DBGout();
}

IBLCL void osUnlockMutex( void )
{
  DBGin("osUnlockMutex");
  DBGprint(DBG_BRANCH,("Unlocking Process %d ",current->pid));
  up(&osMutex);
  DBGout();

}
