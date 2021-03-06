#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include<sched.h>
#include<math.h>


unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
//Pranjali
//rdy is ready queue.
int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */
	int i,newEpoch;
	int prev;
	int next,maxGood;
	int prioVar;

	/* no switch needed if current process priority higher than next*/
	
	(optr= &proctab[currpid]);
	if(getschedclass()==EXPDISTSCHED)
	{
		
	
		prioVar=expdev(0.1);

				 if (optr->pstate == PRCURR)
				  {	
					optr->pstate = PRREADY;
					insert(currpid,rdyhead,optr->pprio); // enter the current process in the ready queue.-pranjali  insert.c  --  insert an process into a q list in key order
				}
				next = q[rdyhead].qnext;
				
					while (q[next].qkey <(int) prioVar)	 
					{	
						
						 //as the processes are ordered on priority, this is going to be the process with priority least greatest than the exponentially generated priority. 
					
					next = q[next].qnext;
					}
				if(next>NPROC)//rdytail has largest integer value.
				{
				//schedule last process
				next=q[rdytail].qprev;
				}
				 
				        //schedule this process

				        nptr = &proctab[ (currpid = next) ]; //pointer pointing to last process in ready queue.-Pranjali
				        nptr->pstate = PRCURR;
				        dequeue(next);
       			 	
       			 #ifdef	RTCLOCK
			preempt = QUANTUM;		/* reset preemption counter	*/
			#endif
		
	}

	else if (getschedclass()==2)
	{
				//kprintf("\nLINUX%d--%d",preempt,currpid);
				/*decide first if new epoch or not*/
				/*if nw epoch calculate quantum for all processes*/
				/*if not new epoch calculate goodness for current process and unused quantum*/
				/*schedule the process with highest goodness value*/
		
				newEpoch=1;
		
				//adjust current process
			proctab[currpid].quantum=preempt;
			if(proctab[currpid].quantum !=0)
	
				{proctab[currpid].goodness=proctab[currpid].pprio+proctab[currpid].quantum;}
			else
				{proctab[currpid].goodness=0;}
	
			for (i=0;i<NPROC;i++)
			{
		
				if( ( proctab[i].pstate==PRREADY || proctab[i].pstate==PRCURR ) && proctab[i].quantum!=0) //process hass some quantum left for current epoch//
				{
						newEpoch=0;	
					//	kprintf("OE%d--%d\t",i,proctab[i].pprio);
				}
		
			}
	
			if(newEpoch==1)
			{//calculate new quantums for new Epoch
			//kprintf("--NE--");
			for (i=0;i<NPROC;i++)
			{	if(proctab[i].pstate!=PRFREE)
				{	proctab[i].quantum=proctab[i].pprio+(int)(0.5*proctab[i].quantum);
					proctab[i].goodness=proctab[i].pprio;
				}
			}
			}
		
			//debug loop
			for (i=0;i<NPROC;i++)
			{
		
				if( proctab[i].pstate==PRREADY || proctab[i].pstate==PRCURR) //process hass some quantum left for current epoch//
				{
				
					//	kprintf("%d--%d--%d--%d\t",i,proctab[i].pprio,proctab[i].quantum,proctab[i].goodness);
				}
		
			}//debug loop ends here
			if (optr->pstate == PRCURR) {
				optr->pstate = PRREADY;
				insert(currpid,rdyhead,optr->pprio); // enter the current process in the ready queue.-pranjali  insert.c  --  insert an process into a q list in key order
			}
					/* mark it currently running	*/
	
			maxGood=0;
			for (i=0;i<NPROC;i++)
			{	if((proctab[i].pstate==PRREADY || proctab[i].pstate==PRCURR) && proctab[i].goodness>=maxGood && proctab[i].quantum!=0)
				{	maxGood=proctab[i].goodness;
			
					next=i;
				}
			}
			//kprintf("mg%d--",maxGood);
			nptr=&proctab[(currpid = dequeue(next))];
			nptr->pstate = PRCURR;
	
			preempt = nptr->quantum;
			//kprintf("Here--%d-%d\n",currpid,preempt);
	}
	else 
	{	
			if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
			   (lastkey(rdytail)<optr->pprio)) { //lastkey will be defined in queue.h
		//last process in ready queue has a priority lesser than the current process priority--pranjali
				return(OK);
			}
	
			/* force context switch */
		// if the above condition is not true context switch has to  take place --pranjali. Essentiatlly this indicates that the process can be executed next but there is another process in ready queue which has a priority higher than the priority of current process.-pranjali
			if (optr->pstate == PRCURR) {
				optr->pstate = PRREADY;
				insert(currpid,rdyhead,optr->pprio); // enter the current process in the ready queue.-pranjali  insert.c  --  insert an process into a q list in key order
			}

			/* remove highest priority process at end of ready list */

			nptr = &proctab[ (currpid = getlast(rdytail)) ]; //pointer pointing to last process in ready queue.-Pranjali
			nptr->pstate = PRCURR;		/* mark it currently running	*/
		#ifdef	RTCLOCK
			preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
	}
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);//context switch ko old and new process pass kiya. stack pointer and interrupt mask is passed.
	
	/* The OLD process returns here when resumed. */
	return OK;
}
