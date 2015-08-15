#pragma once

#include "Wryv.h"
#include <vector>
#include <set>
#include <functional>
#include <string>
using namespace std;

#include "GlobalFunctions.h"
#include "WryvGameMode.h"
#include "Ability.h"
#include "Types.h"
#include "GameObject.h"
#include "Unit.h"
#include "Box2DU.h"

class TextWidget;
class AGameObject;
class AHUD;
class ATheHUD;

// None (null) alignment means positioned absolutely
enum HAlign { None=0, Left=1<<0, HCenter=1<<1, Right=1<<2, 
  ToLeftOfParent=1<<3, ToRightOfParent=1<<4, HFull=1<<5 };
enum VAlign { Top=1<<10, VCenter=1<<11, Bottom=1<<12,
  OnTopOfParent=1<<13, BelowBottomOfParent=1<<14, VFull=1<<15 };
enum Alignment {
  TopLeft=Top|Left, TopRight=Top|Right, TopCenter=Top|HCenter,
  CenterLeft=Left|VCenter, CenterCenter=VCenter|HCenter, CenterRight=VCenter|Right,
  BottomLeft=Left|Bottom, BottomCenter=Bottom|HCenter, BottomRight=Bottom|Right,
  Full=HFull|VFull
};
enum LayoutMode { Pixels, Percentages };
enum EventCode { NotConsumed=0, Consumed=1 };

class HotSpot
{
public:
  static TextWidget *TooltipWidget; // Reference to the widget to send tooltip to on flyover
  static ATheHUD* hud;

  string CName; //the name of the widget, visible in C++ debugger
  FString Name; //name of the widget
  FString TooltipText; // tooltip that appears on flyover
  
  int Align;
  int Layout;
  bool hidden;
  bool eternal;  // does the widget get deleted after time
  float displayTime; // Amount of time to display remaining
  //FVector2D Pos;
  FVector2D Size;
  FVector2D Pad, Margin; // Amount of padding (space inside)
  // and Margin (space outside) this widget.
  // Margin is used for locating a widget inside the parent
  // while Pad is used for locating children inside this widget.
  bool Dead;

  FLinearColor Color;
  // Associations with AGameObject's contained in this hotspot. Empty if no objects
  // are associated with the spot (used for item slots)
  //vector<AGameObject*> Objects;

protected:
  HotSpot* Parent;
  vector<HotSpot*> children;  // Children of this hotspot.
  bool dirty;  // Set if the widget's bounds need to be remeasured
  //bool bubbleUp; // When set, the event can bubbles up through the widget
public:
  
  // Returns true (1) if event should be consumed.
  function< EventCode (FVector2D mouse) > OnMouseDownLeft;
  // A function that runs when the widget is dropped @ certain location used for "drops"
  function< EventCode (FVector2D mouse) > OnMouseUpLeft;
  function< EventCode (FVector2D mouse) > OnMouseDownRight;
  function< EventCode (FVector2D mouse) > OnMouseUpRight;
  function< EventCode (FVector2D mouse) > OnHover;
  function< EventCode (FVector2D mouse) > OnMouseDragLeft;
  function< void (float t) > OnTick; // a function to run at each tick.

  virtual void HotSpotDefaults();
  HotSpot( FString name ) {
    HotSpotDefaults();
    SetName( name );
  }
  HotSpot( FString name, FVector2D size ) {//: HotSpot( name ){
    HotSpotDefaults();
    Size = size;
    SetName( name );
  }
  virtual ~HotSpot(){ Clear(); }
  void SetName( FString name ){
    Name = name;
    CName = TCHAR_TO_UTF8( *Name );
  }

protected:
  virtual void render( FVector2D offset )
  {
    if( hidden ) return;
    // When the base class renders, it calls render on all the children.
    for( int i = 0 ; i < children.size(); i++ )
      children[i]->render( offset + Pos() );
  }
public:
  virtual void Move( float t ) {
    displayTime -= t;

    // Hide if display time drops below 0, or delete it if it is not 'eternal'
    if( displayTime <= 0 ) {
      if( eternal )  Hide();
      else Dead = 1; // removes on next tick
    }
    else if( displayTime <= 1 ) {
      // display time waning, start to fade
      Color.A = displayTime;
    }

    // tick the children
    for( int i = 0; i < children.size(); i++ )
      children[i]->Move( t );

    Clean();
  }

  void Clean()
  {
    // Remove dead ones (recursively)
    for( int i = children.size()-1; i >= 0; i-- )
    {
      // if the child should die, delete it
      if( children[i]->Dead ) {
        delete children[i];
        removeIndex( children, i );
      }
    }
  }

  template <typename T>
  T* Add( T* w ) {
    if( w->Parent )  w->Orphan();
    w->Parent = this;
    children.push_back( w );
    return w;
  }
  void SetByCorners( FVector2D TL, FVector2D BR )
  {
    Margin = TL;
    Size = BR - TL;
  }
  // Orphans the child from its parent
  void Orphan()
  {
    if( Parent )
    {
      removeElement( Parent->children, this );
      Parent = 0;
    }
  }
  // all calls to render a widget externally route through here
  // they all kick-off to subclass::render( FVector2D(0,0) )
  virtual void render(){ if( hidden ) return; render( FVector2D(0,0) ); }

  // reflows the contained hotspot to parent widget,
  // using ALIGNMENT settings.
  // ** TODO: can add a margin parameter for flushing outside of widget border.
  // The DRAW() function always lays out from the top left, so this just dynamically
  // computes Position based on alignment, size & parent size parameters
  FVector2D Pos()
  {
    FVector2D P(0,0);
    if( !Parent )
    {
      //LOG(  "Cannot realign to null parent, %s",
      //  *Name );
      // Can reflush position based on Margins, or just leave it
      //Pos = Margin;
      return P; // cannot reflush
    }
    FVector2D PSize = Parent->Size;
    FVector2D PM = Margin + Parent->Pad;

    // Null alignment (0) means absolutely positioned (does not reflow in parent, Pos is
    // just set from Margins)
    if( Align & Left )  P.X = PM.X;
    else if( Align & Right )  P.X = PSize.X - Size.X - PM.X;
    else if( Align & HCenter )  P.X = (PSize.X - Size.X)/2; // centering
    else if( Align & ToLeftOfParent )  P.X = -Size.X - Parent->Margin.X - Margin.X;
    else if( Align & ToRightOfParent )  P.X = PSize.X + 2*PM.X + Parent->Margin.X;
    else if( Align & HFull )  P.X = 0, Size.X = PSize.X;
    else P.X = PM.X; // When absolutely positioned (None) values, the X position 
    // is just Left aligned

    if( Align & Top )  P.Y = PM.Y;
    else if( Align & Bottom )  P.Y = PSize.Y - Size.Y - PM.Y;
    else if( Align & VCenter )  P.Y = (PSize.Y - Size.Y)/2;
    else if( Align & BelowBottomOfParent )  P.Y = PSize.Y + 2*PM.Y + Parent->Margin.Y;
    else if( Align & OnTopOfParent )  P.Y = -Size.Y - Parent->Margin.Y - Margin.Y;
    else if( Align & VFull )  P.Y = 0, Size.Y = PSize.Y;
    else P.Y = PM.Y; // default Top aligned

    return P;
  }

  float left(){ return Pos().X; }
	float right(){ return Pos().X + Size.X; }
	float top(){ return Pos().Y; }
	float bottom(){ return Pos().Y + Size.Y; }
  FVector2D GetAbsPos(){
    // traces back up to the parent and finds the absolute position of this widget
    HotSpot *h = this;
    FVector2D p = Pos();
    while( h->Parent ) { h = h->Parent ; p += h->Pos() ; }
    return p;
  }
  //bool hit( FVector2D v ) {
  //  FBox2DU bounds = GetBounds();
  //  return bounds.IsInside( v ); // check if v is inside the box (<-)
  //}
  // This function checks absolute coordinates
	// Check main bounds
  bool hit( FVector2D v ) {
    FBox2DU bounds = GetBounds();
    return bounds.IsInside( v ); // check if v is inside the box (<-)
  }
  
  // ABSOLUTE bounds on the object
  FBox2DU GetBounds( FVector2D offset ){
    FBox2DU bounds;
    // bound this and all children
    bounds += offset + Pos();
    bounds += offset + Pos() + Size;
    for( int i = 0; i < children.size(); i++ ) {
      bounds += children[i]->GetBounds( offset + Pos() );
    }
    return bounds;
  }
  FBox2DU GetBounds()
  {
    return GetBounds( FVector2D(0,0) );
  }

  // Get absolute bounds containing the children, excluding this
  FBox2DU GetChildBounds()
  {
    FBox2DU bounds ;
    if( !children.size() )
    {
      LOG(  "Widget %s had NO CHILDREN!", *Name );
      // return the bounds of the container instead
      //bounds += GetBounds();
      // give 0 size box
      bounds = FBox2DU( FVector2D(0,0), FVector2D(0,0) );
    }
    else for( int i = 0; i < children.size(); i++ ) {
      bounds += children[i]->GetBounds( Pos() );
    }
    return bounds;
  }
  
  // Recomputes the widget's bounds size based on the size of the children
  void recomputeSizeToContainChildren()
  {
    Size = GetChildBounds().Size() + Pad*2.f;
  }

  void Show() { hidden = 0; }
  void Hide() { hidden = 1; }
  // ShowChildren without affecting parent
  void ShowChildren(){
    for( int i = 0; i < children.size(); i++ )
      children[i]->Show();
  }
  // HideChildren without affecting parent
  void HideChildren(){
    for( int i = 0; i < children.size(); i++ )
      children[i]->Hide();
  }
  // Get the bounds of just this widget, excluding the children
  // used for when flushing position of something inside a widget
  // towards the right or left. if you get bounds including the
  // children it will tend accumulate an overflow right/left if it overflows.
  FBox2DU GetBoundsExcludingChildren()
  {
    FBox2DU bounds;
    bounds += Pos();
    bounds += Pos() + Size;
    return bounds;
  }
  void Clear(){
    for( int i = 0; i < children.size(); i++ )
      delete children[i];
    children.clear();
  }

  // Checks if a point v hits this widget,
  // if so, an Action (described by function f) is executed on this hotspot.
  HotSpot* Act( FVector2D v, FVector2D offset, function< EventCode (FVector2D) > HotSpot::* f )
  {
    if( hidden ) return 0;

    // Look into deepest child first.
    for( int i = 0; i < children.size(); i++ )
    {
      // check if a child is hit by the click.
      HotSpot *h = children[i]->Act( v, offset + Pos(), f );  // offset accumulates absolute position offset of widget
      if( h )  return children[i]; // If the hit was intercepted, return object hit.
    }

    // Actually perform hit checking, if this provides the function f.
    if( this->*f && hit( v - offset ) )
    {
      // The widget returns 0 when propagation should stop.
      int result = (this->*f)( v - offset - Pos() ); // pass in relative coordinates
      // If the widget doesn't bubble events, then you must return that it hit here

      // If the widget consumes events, then halt propagation by returning this
      if( result == Consumed ) return this;
    }

    return 0; //not hit or bubbles are allowed.
  }

  // returns the child hit (or this) if event tripped, NULL if event not tripped
  HotSpot* MouseDownLeft( FVector2D v )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnMouseDownLeft );
  }
  HotSpot* MouseUpLeft( FVector2D v )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnMouseUpLeft );
  }
  HotSpot* MouseDownRight( FVector2D v )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnMouseDownRight );
  }
  HotSpot* MouseUpRight( FVector2D v )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnMouseUpRight );
  }
  HotSpot* Hover( FVector2D v )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnHover );
  }
  HotSpot* MouseDraggedLeft( FVector2D v )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnMouseDragLeft );
  }
};
