// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: aptwidgets.h,v 1.1.1.1 2000/08/10 12:42:39 kojima Exp $
/* ######################################################################

   DeityWidgets - Manages groups of widgets
   
   ##################################################################### */
									/*}}}*/
#ifndef APTWIDGETS_H
#define APTWIDGETS_H

#include <deity/notify.h>

/* We simply declare the items so we can advoid including all the headers
   very few modules actually need more than a few of these at once */
class Widget;
class BasicWidget;
class TextWidget;
class GUIWindow;
class PkgTree;
class ColumnBar;
class MainWindow;
class TabDialog;
class MenuBar;
class pkgCache;
class ExtraCache;
class pkgControlCache;
class MultiLineWidget;
class InfoWidget;

class PackageView
{    
   MainWindow &Owner;

   public:

   // The main elements of the display
   MenuBar *Menu;
   ColumnBar *Columns;
   PkgTree *PTree;
   TabDialog *StatusBox;
   MultiLineWidget *Description;
   TextWidget *Status;
   TextWidget *Usr;
   TextWidget *Debs;
   InfoWidget *InfoPage;
      
   // The contents of the Info Page
   
   // The contents of the Stat Page
   
   // The contents of the Words Page 
   
   void Layout();
   void Create();
   void Sync();
   
   // Resyncronize the status display
   bool SyncStatus(Widget *,Notifyer::Tag,void *);
   bool TreeChange(Widget *,Notifyer::Tag,void *);
   
   PackageView(MainWindow &Owner) : Owner(Owner) {};
};

class MainWindow
{
   public:
   
   // Main Window for Graphics Mode and root widget for text mode
   GUIWindow *Main;
   Widget *Base;
   ExtraCache *CurCache;
   pkgControlCache *CtrlCache;
   
   // PackageView Pane
   PackageView PkgView;

   // Causes the panes to perform layout
   void Layout();
   
   MainWindow(Widget *Root);
};

extern MainWindow *Base;

#endif
