// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: pkgtree.h,v 1.1.1.1 2000/08/10 12:42:39 kojima Exp $
/* ######################################################################

   PkgTree - A tree class for displaying packages

   The package tree class handles the various item classes and provides
   them with some utility functions. It also handles all of the user 
   interaction for the item classes. 
   
   To speed up the rendering we cache the location data from the column
   bar, it is regenerated each time the Tree's render function is called.
   
   ##################################################################### */
									/*}}}*/
#ifndef PKGTREE_H
#define PKGTREE_H

#include <deity/tree.h>
#include <apt-pkg/pkgcache.h>

class ColumnBar;
class ExtraCache;
class PackageView;

class PkgTree : public Tree
{
   protected:
   
   // Various internal items
   class Item;
   class DepItem;
   class SecItem;
   class Drawer;
   friend Drawer;
   friend Item;
   friend DepItem;
   friend SecItem;
   
   // Usefull lists
   ExtraCache *Cache;
   ColumnBar *Bar;
   PackageView *Owner;
   
   // Draw Cache
   long ColLocs[8];
   bool ColFields[8];
   bool RegenLocs;
   
   virtual void Render(CombinedGC &GC) {RegenLocs = true; Tree::Render(GC);}
   virtual bool Key(const KeyEvent &Key,Widget *Source);
   virtual void ItemMouse(Tree::Item *I,pkgCache::PkgIterator P,
			  const MouseEvent &Event,int Spacer);
   
   // Helper functions
   void GetLocs(CombinedGC &GC,Rect Pos,long *Locs,bool *Fields);
   
   public:

   // Populates the tree
   void MakeList(ExtraCache *Cache);

   // User actions
   bool DeletePkg(Widget *,Notifyer::Tag,void *);
   bool KeepPkg(Widget *,Notifyer::Tag,void *);
   bool InstallPkg(Widget *,Notifyer::Tag,void *);
   
   // Accessors
   pkgCache::PkgIterator CurrentPkg();
   
   PkgTree(PackageView *Owner,ColumnBar *Bar, Widget *Parent = 0);
};

#endif
