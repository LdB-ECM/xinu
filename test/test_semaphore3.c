#include <stddef.h>
#include <thread.h>
#include <semaphore.h>
#include <stdio.h>
#include <testsuite.h>

#if NSEM
extern bool test_checkSemCount(semaphore s, short c);
extern bool test_checkProcState(tid_typ tid, unsigned char state);
extern bool test_checkResult(unsigned char testResult, unsigned char expected);

extern void test_semWaiter(semaphore s, int times, unsigned char *testResult);
#endif

thread test_semaphore3(bool verbose)
{
#if NSEM
    tid_typ atid, btid;
    bool passed = TRUE;
    semaphore s;
    unsigned char testResult = 0;
    char msg[50];

    testPrint(verbose, "Semaphore creation: ");
    s = semcreate(1);
    if (isbadsem(s))
    {
        passed = FALSE;
        sprintf(msg, "%d", s);
        testFail(verbose, msg);
    }
    else if (test_checkSemCount(s, 1))
    {
        testPass(verbose, "");
    }
    else
    {
        passed = FALSE;
    }

    /* We use a higher priority for thread A to ensure it is not rescheduled to
     * thread B before it is even able to wait on the semaphore.  */
    ready(atid =
          create((void *)test_semWaiter, INITSTK, 32,
                 "SEMAPHORE-A", 3, s, 1, &testResult));
    ready(btid =
          create((void *)test_semWaiter, INITSTK, 31,
                 "SEMAPHORE-B", 3, s, 1, &testResult));
	resched();

    testPrint(verbose, "Wait on semaphore: ");
    /* Process A should be admitted, but B should wait. */
    if (test_checkProcState(atid, THRFREE)
        && test_checkProcState(btid, THRWAIT)
        && test_checkSemCount(s, -1) && test_checkResult(testResult, 1))
    {
        testPass(verbose, "");
    }
    else
    {
        passed = FALSE;
    }

    signal(s);

    /* Process B waited, so signal should release it. */
    testPrint(verbose, "Signal waiting semaphore: ");
    if (test_checkProcState(btid, THRFREE)
        && test_checkSemCount(s, 0) && test_checkResult(testResult, 2))
    {
        testPass(verbose, "");
    }
    else
    {
        passed = FALSE;
    }

    if (TRUE == passed)
    {
        testPass(TRUE, "");
    }
    else
    {
        testFail(TRUE, "");
    }

    /* Processes should be dead, but in case the test failed. */
    kill(atid);
    kill(btid);
    semfree(s);

#else /* NSEM */
    testSkip(TRUE, "");
#endif /* NSEM == 0 */
    return OK;
}