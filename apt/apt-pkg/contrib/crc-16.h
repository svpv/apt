// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: crc-16.h,v 1.1.1.1 2000/08/10 12:42:39 kojima Exp $
/* ######################################################################

   CRC16 - Compute a 16bit crc very quickly
   
   ##################################################################### */
									/*}}}*/
#ifndef APTPKG_CRC16_H
#define APTPKG_CRC16_H

#ifdef __GNUG__
#pragma interface "apt-pkg/crc-16.h"
#endif 

#define INIT_FCS  0xffff
unsigned short AddCRC16(unsigned short fcs, void const *buf,
			unsigned long len);

#endif
