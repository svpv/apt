// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: acqprogress.h,v 1.1 2002/07/23 17:54:51 niemeyer Exp $
/* ######################################################################

   Acquire Progress - Command line progress meter 
   
   ##################################################################### */
									/*}}}*/
#ifndef ACQPROGRESS_H
#define ACQPROGRESS_H

#include <apt-pkg/acquire.h>

class AcqTextStatus : public pkgAcquireStatus
{
   unsigned int &ScreenWidth;
   char BlankLine[300];
   unsigned long ID;
   unsigned long Quiet;
   
   public:
   
   virtual bool MediaChange(string Media,string Drive);
   virtual void IMSHit(pkgAcquire::ItemDesc &Itm);
   virtual void Fetch(pkgAcquire::ItemDesc &Itm);
   virtual void Done(pkgAcquire::ItemDesc &Itm);
   virtual void Fail(pkgAcquire::ItemDesc &Itm);
   virtual void Start();
   virtual void Stop();
   
   bool Pulse(pkgAcquire *Owner);

   AcqTextStatus(unsigned int &ScreenWidth,unsigned int Quiet);
};

#endif
