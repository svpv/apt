// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: pkgtreeitem.h,v 1.1.1.1 2000/08/10 12:42:39 kojima Exp $
/* ######################################################################

   PkgTree Items - Tree::Item classes for displaying items in the package 
                   tree

   Each of these classes handles a different sort of item. All of them
   display directly from the ExtraCache structure and store minimal 
   internal data making them cheap to instantiate.

   The drawer class contains functions to help draw the items as well
   as a collection of variables determined for each item to help the
   render process. 
   
   ##################################################################### */
									/*}}}*/
#ifndef PKGTREEITEM_H
#define PKGTREEITEM_H

#include "pkgtree.h"
#include "extracache.h"

class PkgTree::Item : public Tree::Item
{
   protected:
   
   pkgCache::Package *Pkg;
      
   // Base class members
   virtual void Render(CombinedGC &GC,unsigned long Depth,
		       Rect Pos,Tree *Owner);
   virtual long Height();
   virtual bool Expand(Tree *Owner);
   virtual void Contract(Tree *Owner);
   virtual void Mouse(Tree *Owner,const MouseEvent &Event);
      
   public:
   
   pkgCache::PkgIterator Package(pkgCache &Cache) {return pkgCache::PkgIterator(Cache,Pkg);};
   
   virtual unsigned int Type() {return 100;};
   Item(pkgCache::Package *Pkg,PkgTree *Owner,Tree::Item *Parent = 0);
};

class PkgTree::DepItem : public Tree::Item
{
   protected:
   
   pkgCache::Dependency *Dep;
      
   // Base class members
   virtual void Render(CombinedGC &GC,unsigned long Depth,
		       Rect Pos,Tree *Owner);
   virtual bool Expand(Tree *Owner);
   virtual long Height();
   virtual void Mouse(Tree *Owner,const MouseEvent &Event);
      
   public:
   
   pkgCache::PkgIterator Package(pkgCache &Cache) {return pkgCache::DepIterator(Cache,Dep).TargetPkg();};
   virtual unsigned int Type() {return 101;};

   DepItem(pkgCache::Dependency *Dep,PkgTree *Owner,Tree::Item *Parent = 0);   
};

class PkgTree::SecItem : public Tree::Item
{
   protected:
   
   const char *Section;
   
   virtual void Render(CombinedGC &GC,unsigned long Depth,Rect Pos,Tree *Owner);
   virtual long Height();
   virtual void Mouse(Tree *Owner,const MouseEvent &Event);
   
   public:
   
   SecItem(const char *Section,Tree::Item *Parent = 0);
};

class PkgTree::Drawer
{
   CombinedGC &GC;
   ExtraCache &Cache;
   pkgCache::PkgIterator Pkg;
   ExtraCache::StateCache *State;
   int Height;
   unsigned long Depth;
   Tree *Owner;
   Tree::Item *Item;
   Rect Pos;
   bool Selected;
   
   long Locs[8];
   bool Fields[8];
   int Space;
   
   void RenderCheck(bool D,bool K,bool I,bool State);
   void Init();
   
   public:

   // Locations of each region in the locs array
   enum Regions {AllStates = 10,Delete = 0,Keep = 1,Install = 2,Name = 3,
                 CurrentVer = 4,CandidateVer = 5,InstAsWell = 6};

   // Some usefull accessors
   operator pkgCache &() {return Cache;};
   ExtraCache &GetCache() {return Cache;};
   inline bool IsSelected() {return Selected;};
   
   void SetPkg(pkgCache::PkgIterator const &P) 
   {
      Pkg = P;
      State = &Cache[P];
   };
   
   // Drawing functions
   void DrawAsWell();
   void DrawState();
   void DrawCurrentVersion();
   void DrawCandidateVersion();
   void DrawName(const char *Name,XPMImage &Image,Color TextClr = Wc_None);
   void BFill(unsigned int Region);
   
   Drawer(CombinedGC &GC,unsigned long Depth,Rect Pos,Tree *Owner,
	  Tree::Item *Item) :
	 GC(GC), Cache(*((PkgTree *)Owner)->Cache), Pkg(Cache),
         State(0), Height(Pos.h), Depth(Depth),
         Owner(Owner), Item(Item), Pos(Pos)
   {
      Init();
   }
   
   Drawer(CombinedGC &GC,unsigned long Depth,Rect Pos,Tree *Owner,
	  pkgCache::Package *PkgP,Tree::Item *Item) :
	 GC(GC), Cache(*((PkgTree *)Owner)->Cache), Pkg(Cache,PkgP),
         State(&Cache[Pkg]), Height(Pos.h), Depth(Depth),
         Owner(Owner), Item(Item), Pos(Pos)
   {
      Init();
   }
};

#endif
