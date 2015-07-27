#pragma once

#include <vector>
#include <functional>
#include <string>
#include "GlobalFunctions.h"
#include "RTSGameGameMode.h"
using namespace std;
class AHUD;

struct FBox2DU : public FBox2D
{
  // +---------->
  // | min
  // |  TL----TR
  // |   |    |
  // |   |    |
  // v  BL----BR
  //          max
  FBox2DU( FVector2D min, FVector2D max )
  {
    Min = min;
    Max = max;
  }
  FBox2DU()
  {
    Min.X = Min.Y = 1e6f;
    Max.X = Max.Y = -1e6f;
  }
  // Translates the box by V
  FBox2DU operator+( FVector2D v ){ return FBox2DU( Min+v, Max+v ); }
  FBox2DU operator-( FVector2D v ){ return FBox2DU( Min-v, Max-v ); }
  float left(){ return Min.X; }
	float right(){ return Max.X; }
	float top(){ return Min.Y; }
	float bottom(){ return Max.Y; }

  FVector2D TL() { return Min; }
  FVector2D TR() { return FVector2D( Max.X, Min.Y ); }
  FVector2D BR() { return Max; }
  FVector2D BL() { return FVector2D( Min.X, Max.Y ); }
  
  void print(FString msg) {
    UE_LOG( LogTemp, Warning, TEXT( "%s fbox2du bounds min=(%f %f) max=(%f %f) meas=(%f %f)"),
      *msg, Min.X, Min.Y, Max.X, Max.Y, Size().X, Size().Y );
  }
  FVector2D Size() { 
    return Max - Min;
  }
};

class AGameObject;

class TextWidget;
class AMyHUD;

class HotSpot
{
public:
  string Name; //the name of the widget
  FString TooltipText; // tooltip that appears on flyover
  static TextWidget *TooltipWidget; // Reference to the widget to send tooltip to on flyover
  static AMyHUD* hud;
  // None (null) alignment means positioned absolutely
  enum HAlign { None=0, Left=1<<0, HCenter=1<<1, Right=1<<2, 
    ToLeftOfParent=1<<3, ToRightOfParent=1<<4, HFull=1<<5 };
  enum VAlign { Top=1<<5, VCenter=1<<6, Bottom=1<<7,
    OnTopOfParent=1<<8, BelowBottomOfParent=1<<9, VFull=1<<10 };
  enum Alignment {
    TopLeft=Top|Left, TopRight=Top|Right, TopCenter=Top|HCenter,
    CenterLeft=Left|VCenter, CenterCenter=VCenter|HCenter, CenterRight=VCenter|Right,
    BottomLeft=Left|Bottom, BottomCenter=Bottom|HCenter, BottomRight=Bottom|Right,
  };
  enum LayoutMode { Pixels, Percentages };

  int Align;
  int Layout;
  bool hidden;
  float displayTime; // Amount of time to display remaining
  //FVector2D Pos;
  FVector2D Size;
  FVector2D Pad, Margin; // Amount of padding (space inside)
  // and Margin (space outside) this widget.
  // Margin is used for locating a widget inside the parent
  // while Pad is used for locating children inside this widget.

  //
  // +----------------------+
  // |                      |
  // |   Pos=[Margin]       |
  // |   *----------------+ |
  // |   |                | |
  // |   |      text      | |
  // |   |                | |
  // |   +----------------+ |
  // |                      |
  // +----------------------+
  // The Position of the widget is equal to the margin.
  FLinearColor Color;
  // Associations with AGameObject's contained in this hotspot. Empty if no objects
  // are associated with the spot (used for item slots)
  vector<AGameObject*> Objects;
protected:
  HotSpot* Parent;
  vector<HotSpot*> children;  // Children of this hotspot.
  bool dirty;  // Set if the widget's bounds need to be remeasured
  bool bubbleUp; // When set, the event can bubbles up through the widget
public:
  // Returns true (1) if event should be consumed.
  function< int (FVector2D mouse) > OnClicked;
  function< int (FVector2D mouse) > OnRightClicked;
  function< int (FVector2D mouse) > OnHover;
  function< int (FVector2D mouse) > OnDrag;
  // A function that runs when the widget is dropped @ certain location
  function< int (FVector2D mouse) > OnDrop;
  void defaults();
  HotSpot() {
    defaults();
  }
  HotSpot( FVector2D size ) {
    defaults();
    Size = size;
  }
  virtual ~HotSpot(){ Clear(); }

protected:
  virtual void render( FVector2D offset )
  {
    if( hidden ) return;
    // When the base class renders, it calls render on all the children.
    for( int i = 0 ; i < children.size(); i++ )
      children[i]->render( offset + Pos() );
  }
public:
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
      remove( Parent->children, this );
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
      UE_LOG( LogTemp, Warning, TEXT( "Cannot realign to null parent, %s" ),
        *FString( Name.c_str() ) );
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
    else if( Align & ToLeftOfParent )  P.X = -Size.X - Parent->Margin.X;
    else if( Align & ToRightOfParent )  P.X = PSize.X + Parent->Margin.X;
    else if( Align & HFull )  P.X = 0, Size.X = PSize.X;
    else P.X = PM.X; // When absolutely positioned (None) values, the X position 
    // is just Left aligned

    if( Align & Top )  P.Y = PM.Y;
    else if( Align & Bottom )  P.Y = PSize.Y - Size.Y - PM.Y;
    else if( Align & VCenter )  P.Y = (PSize.Y - Size.Y)/2;
    else if( Align & BelowBottomOfParent )  P.Y = PSize.Y + Parent->Margin.Y;
    else if( Align & OnTopOfParent )  P.Y = -Size.Y - Parent->Margin.Y;
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
      UE_LOG( LogTemp, Warning, TEXT( "Widget %s had NO CHILDREN!" ), *FString(Name.c_str()) );
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
  void recomputeSize()
  {
    Size = GetChildBounds().Size() + Pad*2.f;
  }

  void show(){
    hidden = 0;
    for( int i = 0; i < children.size(); i++ ) children[i]->show();
  }
  void hide(){
    hidden = 1;
    for( int i = 0; i < children.size(); i++ ) children[i]->hide();
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
  HotSpot* Act( FVector2D v, FVector2D offset, function< int (FVector2D) > HotSpot::* f )
  {
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
      (this->*f)( v - offset - Pos() ); // pass in relative coordinates
      // If the widget doesn't bubble events, then you must return that it hit here

      // If the widget consumes events, then halt propagation
      // Deepest child gets searched, then we return from here
      if( !bubbleUp ) return this; 
    }

    return 0; //not hit or bubbles are allowed.
  }

  // returns the child hit (or this) if event tripped, NULL if event not tripped
  HotSpot* Click( FVector2D v, FVector2D offset )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnClicked );
  }
  HotSpot* RightClick( FVector2D v, FVector2D offset )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnRightClicked );
  }
  HotSpot* Hover( FVector2D v )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnHover );
  }
  HotSpot* Drag( FVector2D v )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnDrag );
  }
  HotSpot* Drop( FVector2D v )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnDrop );
  }

};

class TextWidget : public HotSpot
{
public:
  UFont *Font;
  FString Text;
  float Scale;

  // During construction, we assign Text directly, then call Measure.
  // Construction is assumed to happen when the HUD is available.
  TextWidget( FString fs, UFont *font=0, float scale=1.f ) : Text( fs ), Font(font), Scale( 1.f )
  {
    Name = TCHAR_TO_UTF8( *fs );
    SetAndMeasure(fs);
  }
  virtual ~TextWidget(){}

  // Modifies text inside widget, but does not remeasure text size (until next draw call)
  void Set( FString text )
  {
    Text = text;
    dirty = 1;
    // We do not call Measure() here, instead we call it at the
    // next RENDER call, since if the HUD is not available, we cannot
    // measure the text.
    //Measure(); 
    // [[ cannot call measure here, since may happen
    //    during callback due to mouse motion etc. ]]
  }

  // Sets the text and re-measures the size of the text immediately afterwards
  // the HUD must be ready for this
  void SetAndMeasure( FString text )
  {
    Set( text );
    Measure();
  }

  void Measure();
  
protected:
  // We call Measure() each call to render() since text cannot be measured except when
  // canvas is ready (ie during render() calls)
  virtual void render( FVector2D offset ) override;
};

class ImageWidget : public HotSpot
{
public:
  UTexture* Icon;
  FVector2D uv; // The maximum coordinates of the UV texturing
  FVector2D hotpoint; // Usually top left corner (0,0), meaning will render from topleft corner.
  // if its half size, then it will render from the center (such as when an imageWidget is being
  // click-dragged
  FLinearColor Color;
  float Rotation;
  FVector2D PivotPoint; // the pivot about which the rotation is based

  ImageWidget( UTexture* pic ) : Icon( pic ), uv( 1, 1 ), hotpoint(0,0), Color(FLinearColor::White), Rotation( 0.f )
  {
    if( pic ) Name = TCHAR_TO_UTF8( *pic->GetName() );
    if( Icon ){
      Size.X = Icon->GetSurfaceWidth();
      Size.Y = Icon->GetSurfaceHeight();
    }
  }
  ImageWidget( UTexture* pic, FVector2D size ) :
    Icon( pic ), HotSpot( size ), uv( 1, 1 ), hotpoint(0,0), Color(FLinearColor::White), Rotation( 0.f )
  {
    if( pic ) Name = TCHAR_TO_UTF8( *pic->GetName() );
  }
  virtual ~ImageWidget(){}

protected:
  virtual void render( FVector2D offset ) override ;
};

class ITextWidget : public ImageWidget
{
  TextWidget* Text;
public:
  FString GetText(){ return Text->Text; }

  ITextWidget( UTexture* pic, FVector2D size, FString ftext, int textAlignment,
    UFont* font=0, float scale=1.f ) :
    ImageWidget( pic, size )
  {
    FString name = FString::Printf( TEXT("ITextWidget %s"), *ftext );
    if( pic ) name += pic->GetName();
    Name = TCHAR_TO_UTF8( *name );
    Text = new TextWidget( ftext, font, scale );
    Text->Align = textAlignment;
    Add( Text );
  }
};

class SolidWidget : public ImageWidget
{
public:
  static UTexture* SolidWhiteTexture;
  SolidWidget( FLinearColor color ) : ImageWidget( SolidWhiteTexture )
  {
    Name = "SolidWidget";
    Color = color;
  }
  SolidWidget( FVector2D size, FLinearColor color ) : 
    ImageWidget( SolidWhiteTexture, size )
  {
    Name = "SolidWidget";
    Color = color;
  }
  virtual ~SolidWidget(){}
};

class Border : public HotSpot
{
  SolidWidget *left, *top, *bottom, *right;
public:
  FBox2DU Box;
  float Thickness;
  Border( FBox2DU box, float thickness, FLinearColor color )
  {
    Name = "Border Widget";
    Thickness = thickness;

    left = new SolidWidget( color );
    Add( left );
    top = new SolidWidget( color );
    Add( top );
    bottom = new SolidWidget( color );
    Add( bottom );
    right = new SolidWidget( color );
    Add( right );

    hidden = 1;
    Set( box );
  }

  void Set( FBox2DU box )
  {
    Box = box;
    // TL                  TR
    //   +----------------+
    //   |+--------------+|
    //   ||              ||
    //   |+--------------+|
    //   +----------------+
    // BL                  BR
    // Set the widths
    // trbl
    top->SetByCorners( box.TL(), box.TR() + FVector2D(0,Thickness) );
    right->SetByCorners( box.TR() + FVector2D( -Thickness, 0 ), box.BR() );
    bottom->SetByCorners( box.BL() + FVector2D( 0, Thickness ), box.BR() );
    left->SetByCorners( box.TL(), box.BL() + FVector2D( Thickness, 0 ) );
    hidden = 0;
  }
};

class CooldownPie : public HotSpot
{
  float Time, TotalTime; // time to cooldown
public:
  // Creates a cooldownpie whose animation progresses over a period
  CooldownPie( FVector2D size, float t ) :
    HotSpot( size ), Time(0.f), TotalTime(t)
  {
    Name = "CooldownPie";
  }
  void update( float t ) { Time -= t; }
  void render()
  {
    float p = Time/TotalTime;

    // Display the interpolate animation
    // Don't have an animation for this yet.. but it should
    //hud->DrawLine( 
  }
  virtual ~CooldownPie(){}
};

// starting offset point and group of widgets to draw
// Make a custom widget for the overlays etc
class ResourcesWidget : public HotSpot
{
  TextWidget *gold, *lumber, *stone;
  int Px; // size of the icons
  int Spacing; // spacing between widgets
public:
  static UTexture *GoldTexture, *LumberTexture, *StoneTexture;

  // Add in all the subwidgets
  ResourcesWidget(int pxSize, int spacing) : Px(pxSize), Spacing(spacing)
  {
    Name = "ResourcesWidget";
    // +-----------------+
    // |G1000 W1000 S1000|
    // +-----------------+
    // The 3 resource types
    ImageWidget* i = new ImageWidget( GoldTexture, FVector2D(Px,Px) );
    gold = new TextWidget( "1000" );
    gold->Margin = FVector2D(Px,0);
    i->Add( gold );
    Add( i );
    
    FBox2DU b = i->GetBounds();
    i = new ImageWidget( LumberTexture, FVector2D(Px,Px) );
    i->Margin = FVector2D(b.right()+Spacing,0);
    lumber = new TextWidget( "1000" );
    lumber->Margin = FVector2D(Px,0);
    i->Add( lumber );
    Add( i );
    
    b = i->GetBounds();
    i = new ImageWidget( StoneTexture, FVector2D(Px,Px) );
    i->Margin = FVector2D(b.right()+Spacing,0);
    stone = new TextWidget( "1000" );
    stone->Margin = FVector2D(Px,0);
    i->Add( stone );
    Add( i );

    recomputeSize();
  }
  virtual ~ResourcesWidget(){}
  void SetValues( int goldCost, int lumberCost, int stoneCost )
  {
    gold->Set( FString::Printf( TEXT("%d"), goldCost ) );
    lumber->Set( FString::Printf( TEXT("%d"), lumberCost ) );
    stone->Set( FString::Printf( TEXT("%d"), stoneCost ) );
  }
};

// a widget that shows usage costs
class UseWidget : public HotSpot
{
  ImageWidget *mana;
  UseWidget()
  {
    Name = "UseWidget";
  }
  virtual ~UseWidget(){}
};

// +-------------------------------------+
// | Barracks                            |
// | [] gold [] lumber [] stone          |
// | Barracks are used to build militia. |
// +-------------------------------------+
class CostWidget : public ImageWidget
{
  TextWidget* topText;
  ResourcesWidget* cost;
  TextWidget* bottomText;
  FVector2D Interpadding; // padding on the background image

public:
  // Using a popup for the UI information about the build costs
  // makes it so new players don't actually have to look for it.
  // It pops up, so you always notice it right away.
  CostWidget( UTexture* bkg, FVector2D interpadding, float vSpacing ) : 
    ImageWidget( bkg ), Interpadding(interpadding)
  {
    Name = "CostWidget";
    topText = new TextWidget( "TopText" );

    Add(topText);

    //UE_LOG( LogTemp, Warning, TEXT( "topText->GetBounds()" ) );
    FBox2DU bounds = topText->GetBounds();
    bounds.print("toptext");
    cost = new ResourcesWidget(16, 4);
    cost->Margin = FVector2D( Interpadding.X/2, topText->GetBounds().bottom() + vSpacing );
    Add(cost);

    //UE_LOG( LogTemp, Warning, TEXT( "cost pos: %f %f" ), cost->Pos.X, cost->Pos.Y );
    //UE_LOG( LogTemp, Warning, TEXT( "cost->GetBounds()" ) );
    bounds = cost->GetBounds();
    bounds.print("costbounds");
    bottomText = new TextWidget( "BottomText" );
    bottomText->Align = TopCenter;
    Add(bottomText);

    //UE_LOG( LogTemp, Warning, TEXT( "bottomText->GetBounds()" ) );
    bounds = bottomText->GetBounds();
    bounds.print("bottomText");
    
    Size = GetChildBounds().Size() + Pad;
  }
  virtual ~CostWidget(){}
  void Set( FString top, int goldCost, int lumberCost, int stoneCost, FString bottom )
  {
    topText->Set( top );
    cost->SetValues( goldCost, lumberCost, stoneCost );
    bottomText->Set( bottom );
    dirty = 1;
  }

  virtual void render( FVector2D offset ) override
  {
    if( hidden ) return;
    if( dirty )
    {
      //repad() ;
      dirty = 0 ;
    }
    ImageWidget::render( offset );
  }
};

class Tooltip : public ImageWidget
{
  TextWidget* Text;

public:
  FVector2D Interspacing;
  Tooltip( UTexture* bkg, FString txt, FVector2D interspacing ) 
    : ImageWidget( bkg ), Interspacing(interspacing)
  {
    Text = new TextWidget( txt );
    Add( Text );
    Set( txt );
  }
  virtual ~Tooltip(){}

  // The tooltip can be set in a callback, so it may be reset
  // outside of a render which means that the text size
  // measurement will be wrong. That's why we have the dirty bit.
  void Set( FString txt )
  {
    Text->Set( txt );
    hidden = 0; // unhide the tooltip if it was hidden before
    dirty = 1;
  }

  virtual void render( FVector2D offset ) override {
    if( hidden ) return;
    //re-measure the background's size if the text has changed
    if( dirty )
    {
      //repad();
      dirty = 0;
    }
    ImageWidget::render( offset );
  }
};

class SlotPalette;

class SlotEntry : public ImageWidget
{
  SlotPalette* Palette; // The Palette this Entry belongs to
  int Quantity; // the numeric quantity remaining
  TextWidget* TextQuantity;   // The quantity of the item

public:
  SlotEntry( SlotPalette *palette, UTexture* tex, FVector2D size, int qty ) :
    ImageWidget( tex, size ), Palette(palette), Quantity( qty )
  {
    TextQuantity = new TextWidget( "A" );
    TextQuantity->Align = Right|Bottom;
    Add( TextQuantity );
    dirty = 1;
    //SetQuantity( qty );
    //hidden = 1; // defaults hidden with NULL texture.
  }

  void SetTexture( UTexture* tex, FVector2D originalPos );

  void SetQuantity( int quantity )
  {
    Quantity = quantity;
    TextQuantity->Set( FString::Printf( TEXT("%d"), quantity ) );
    dirty = 1;
    // 0 qty means qty widget disappears.
    //if( !quantity )  TextQuantity->hidden = 1;
    //else  TextQuantity->hidden = 0;
  }

  //virtual void render( FVector2D offset ) override;
};

class SlotPalette : public ImageWidget
{
  ImageWidget* Drag;
  int Rows, Cols;

public:
  FVector2D EntrySize;
  FVector2D EntryInterPadding; // The padding of each SlotEntry

  static UTexture* SlotPaletteTexture;
  // Drag & Drop for items from slots
  // The item we are dragging (subhotspot)
  SlotPalette( UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D entryInterPadding ) : 
    Rows(rows), Cols(cols), EntrySize(entrySize), EntryInterPadding(entryInterPadding), ImageWidget( bkg )
  {
    // Init w/ # slots used in this palette
    // The stock size of the width is 100px/slot.
    // We re-calculate the slotsize though based on # slots used.
    Drag = 0;
    SetNumSlots( rows, cols );
  }
  virtual ~SlotPalette(){}

  FVector2D GetSlotPosition( int i )
  {
    FVector2D v;
    int row = i / Cols;
    int col = i % Cols;
    v.X = col * EntrySize.X;
    v.Y = row * EntrySize.Y;
    return v;
  }

  // Direct-children of SlotPalette must be ImageWidget*.
  // Guaranteed to be ImageWidget* because Add is protected.
  // External users of class cannot add a textwidget for example to the slotpanel.
  // (make sure you don't add children to slotpanel that are not imageWidgets)
  // the children of an imagewidget CAN be textnodes etc.
  SlotEntry* GetSlot( int i )
  {
    return (SlotEntry*)children[i];
  }

  SlotEntry* SetSlotTexture( int i, UTexture* tex )
  {
    if( i < 0 || i >= children.size() )  return 0;
    FVector2D texSize( tex->GetSurfaceWidth(), tex->GetSurfaceHeight() );
    SlotEntry* slot = GetSlot( i );
    slot->SetTexture( tex, GetSlotPosition(i) );

    return slot;
  }

  // Widget's by themselves are just hotspots
  vector<SlotEntry*> SetNumSlots( int rows, int cols )
  {
    Rows = rows;
    Cols = cols;
    vector<SlotEntry*> slots;

    hidden = !rows && !cols;
    if( hidden ) return slots; // don't change the size vars when 0 size because
    // it will corrupt the variables

    //EntrySize = Size / FVector2D( cols, rows );
    //EntrySize.X = Size.X / cols;
    //EntrySize.Y = Size.Y / rows;
    int numSlots = rows*cols;

    // The uv's are equal to 
    // Measure the UV coordinates used since the texture is 6x6
    uv = FVector2D( cols/6., rows/6. ); // The texture is 6x6 blocks
    Clear();    // Remove the old ImageWidgets.
    
    // The size of this widget set here.
    for( int i = 0; i < numSlots; i++ )
    {
      /// initialize with 0 of item
      SlotEntry *iw = new SlotEntry( this, Icon, EntrySize, 0 );
      //iw->hidden = 1; // initialize as hidden, until texture is set.
      slots.push_back( iw );
      Add( iw );
    }

    // Change the size of the panel to being 
    Size = EntrySize * FVector2D( cols, rows );

    return slots;
  }
};

// Supports stacking-in of widgets from left/right or top/bottom
class StackPanel : public ImageWidget
{
public:
  static UTexture* StackPanelTexture;
  FVector2D EntrySize; // Size of an entry in the stackpanel
  
  StackPanel( UTexture* bkg, FVector2D entrySize ) : ImageWidget( bkg ), EntrySize( entrySize )
  {
  }
  virtual ~StackPanel(){}
  
  // Prepares the widget for stacking
  HotSpot* Prep( HotSpot *widget )
  {
    widget->Size = EntrySize;
    widget->Align = TopLeft;
    return widget;
  }

  //  _ _
  // |_|_| <
  void StackRight( HotSpot* widget )
  {
    recomputeSize();
    Prep(widget);
    // Situates the bounds @ the origin,
    // which means its just a measure of the widget's total size.
    FBox2DU childBounds = GetChildBounds();
    widget->Margin.X = childBounds.right();
    //for( int i = 0 ; i < children.size(); i++ )
    //  children[i]->Margin.X -= EntrySize.X + Pad.X;
    Add( widget );
    recomputeSize();
  }

  //    _ _
  // > |_|_|
  void StackLeft( HotSpot* widget )
  {
    recomputeSize();
    Prep(widget);

    // pull all the children already in the widget over to the left
    for( int i = 0 ; i < children.size(); i++ )
      children[i]->Margin.X += EntrySize.X + Pad.X;
    Add( widget );
    recomputeSize();
  }

  //  v
  //  _
  // |_|
  // |_|
  void StackTop( HotSpot* widget )
  {
    recomputeSize();
    Prep(widget);
    for( int i = 0 ; i < children.size(); i++ )
      children[i]->Margin.Y += EntrySize.Y + Pad.Y;
    Add( widget );
    recomputeSize();
  }

  //  _
  // |_|
  // |_|
  //  ^
  void StackBottom( HotSpot* widget )
  {
    Prep(widget);
    FBox2DU childBounds = GetChildBounds();
    widget->Margin.Y = childBounds.bottom();
    Add( widget );
    recomputeSize();
  }
};

class Minimap : public ImageWidget
{
  Border* borders;
public:
  Minimap( UTexture* icon, float borderSize, FLinearColor borderColor ) : 
    ImageWidget( icon )
  {
    FBox2DU box( FVector2D(0,0), Size );
    borders = new Border( box, borderSize, borderColor );
    Add( borders );
  }
  virtual ~Minimap(){}
};

// The right-side panel
class SidePanel : public ImageWidget
{
public:
  // +-----------+
  // | portrait  |
  // +-----------+
  // | unitStats |
  // +-----------+
  // | abilities |
  // +-----------+
  // | minimap   |
  // +-----------+
  ImageWidget* portrait;  // pictoral representation of selected unit
  TextWidget* unitStats;  // The stats of the last selected unit
  SlotPalette* abilities; //  
  Minimap* minimap;       // the minimap widget for displaying the world map

  // For the group of selected units.
  SidePanel( UTexture* texPanelBkg, UTexture* PortraitTexture, UTexture* MinimapTexture, FVector2D size, FVector2D margin ) :
    ImageWidget( texPanelBkg, size )
  {
    Margin = margin;
    portrait = new ImageWidget( PortraitTexture );
    portrait->Align = TopRight;
    Add( portrait );

    unitStats = new TextWidget( "Stats:" );
    unitStats->Margin = FVector2D( 0, portrait->GetBounds().bottom() );
    Add( unitStats );

    SolidWidget *leftBorder = new SolidWidget( FVector2D( 0, size.Y ),
      FLinearColor( 0.1f, 0.1f, 0.1f, 1.f ) );
    leftBorder->Margin = FVector2D( -4, 0 );
    Add( leftBorder );

    abilities = new SlotPalette( SlotPalette::SlotPaletteTexture, 2, 3,
      FVector2D( size.X/3,size.X/3 ), FVector2D(8,8) );
    Add( abilities );
    abilities->Margin.Y = unitStats->bottom();

    minimap = new Minimap( MinimapTexture, 4.f, FLinearColor( 0.1f, 0.1f, 0.1f, 1.f ) );
    minimap->Margin = FVector2D( 0, abilities->GetBounds().bottom() );
    Add( minimap );
    
  }
  virtual ~SidePanel(){}
};

class StatusBar : public SolidWidget
{
public:
  TextWidget* Text;
  StatusBar( FVector2D canvasSize, float height, FLinearColor bkgColor ) :
    SolidWidget( FVector2D( canvasSize.X, height ), bkgColor )
  {
    Text = new TextWidget( "status text" );
    Add( Text );
  }

  void Set( FString text )
  {
    Text->Set( text );
  }
};

class Controls : public HotSpot
{
public:
  ImageWidget *pause;

  Controls( UTexture* texPause )
  {
    pause = new ImageWidget( texPause );
    Add( pause );
  }
};

class MapSelectionScreen : public HotSpot
{
public:
  ImageWidget *Title;
  StackPanel *MapFiles;
  ImageWidget *Thumbnail;
  ITextWidget *OKButton;
  
  UTexture* MapSlotEntryBkg;
  ITextWidget* Selected;
  UFont* Font;

  // Constructs the MapSelection Screen.
  MapSelectionScreen( FVector2D canvasSize, 
    UTexture* titleTex,
    UTexture* mapFilesBkg,
    UTexture* mapSlotEntryBkg,
    UTexture* thumbnailTex,
    FVector2D entrySize,
    UFont* font ) :
    HotSpot( canvasSize ),
    MapSlotEntryBkg( mapSlotEntryBkg ), Selected( 0 ), Font( font )
  {
    // Throw in the title
    Title = new ImageWidget( titleTex );
    Title->Align = TopLeft;
    Title->Margin = FVector2D( 75, 50 );
    Add( Title );
    
    // The stack of menu items.
    MapFiles = new StackPanel( mapFilesBkg, entrySize );
    MapFiles->Align = TopLeft;
    MapFiles->Margin = FVector2D( 12, 12 );
    MapFiles->Pad = FVector2D( 8,8 );
    Add( MapFiles );

    Thumbnail = new ImageWidget( thumbnailTex );
    Thumbnail->Align = CenterRight;
    Thumbnail->Margin = FVector2D( 80, 0 );
    Add( Thumbnail );

    OKButton = new ITextWidget( mapSlotEntryBkg, entrySize, "OK", CenterCenter );
    OKButton->Align = BottomRight;
    OKButton->Margin = FVector2D( 20, 10 );
    OKButton->Size = FVector2D( 75, 50 );
    
    Add( OKButton );
  }

  // Adds a slot
  ITextWidget* AddText( FString ftext, int textAlignment )
  {
    ITextWidget* text = new ITextWidget(
      MapSlotEntryBkg, MapFiles->EntrySize, ftext, textAlignment );
    text->OnClicked = [this,text](FVector2D mouse){
      Select( text );
      return 0;
    };
    MapFiles->StackBottom( text );
    return text;
  }

  // Make widget selected
  void Select( ITextWidget *widget )
  {
    UE_LOG( LogTemp, Warning, TEXT( "Selected %s" ), *widget->GetText() ) ;

    // turn prev selected back to white
    if( Selected )  Selected->Color = FColor::White;
    Selected = widget;
    Selected->Color = FColor::Yellow;
  }
};

/// Displays mission objectives on screen
class MissionObjectivesScreen : public ImageWidget
{
  UTexture *SlotBkg;
  StackPanel *Objectives;
public:
  MissionObjectivesScreen( UTexture *bkg, UTexture *slotBkg,
    FVector2D slotSize, FVector2D pad ) :
    ImageWidget( bkg )
  {
    
  }

  // Add a series of mission objectives here
  ITextWidget* AddText( FString text, int align )
  {
    ITextWidget *tw = new ITextWidget( SlotBkg, FVector2D( 100, 40 ), text, CenterCenter );
    tw->Margin = FVector2D( 10, 0 );
    tw->Align = CenterLeft;
    return Add( tw );
  }
  
};



// The root UI component
class UserInterface : public HotSpot
{
public:
  // +-----------------------------+
  // | resourcesW     | rightPanel  |
  // |                |  Portrait   |
  // |                |  unitStats  |
  // |                |  abilities  |
  // |                | +---------+ |
  // |  +----------+  | | minimap | |
  // |  | itemBelt |  | +---------+ |
  // +--buffs-----------------------+
  //
  // 
  ResourcesWidget* resources; // The resources widget in the top left
  SidePanel* rightPanel;      // containing: portrait, unitStats, abilities, minimap
  CostWidget* costWidget; // a flyover cost of the hovered item from 
  Tooltip* tooltip;
  SlotPalette* itemBelt;
  StackPanel* buffs;
  StackPanel* building; // Queue of things we are building (in order)
  StatusBar* statusBar; // the bottom status bar has 
  Border* selectBox;
  Controls* controls;
  MapSelectionScreen* mapSelectionScreen;
  MissionObjectivesScreen* missionObjectivesScreen;
  ImageWidget* mouseCursor;

  UserInterface( FVector2D size ) : HotSpot( size ) {
    resources = 0, rightPanel = 0, costWidget = 0, tooltip = 0,
    itemBelt = 0, buffs = 0, building = 0, statusBar = 0,
    selectBox = 0, controls = 0, mapSelectionScreen = 0,
    missionObjectivesScreen = 0, mouseCursor = 0;
  }

  // don't need to explicitly delete children of the UI object
  virtual ~UserInterface(){ }

  void Show( int mode )
  {
    if( mode == ARTSGameGameMode::Title || mode == ARTSGameGameMode::MapSelect )
    {
      resources -> hidden = 1;
      rightPanel -> hidden = 1;
      costWidget -> hidden = 1;
      tooltip -> hidden = 1;
      itemBelt -> hidden = 1;
      buffs -> hidden = 1;
      building -> hidden = 1;
      //statusBar -> hidden = 1;
      selectBox -> hidden = 1;
      controls -> hidden = 1;
      mapSelectionScreen -> hidden = 1;
      missionObjectivesScreen -> hidden = 1;
    }
    statusBar -> show(); // Show the status bar at all times
    
    if( mode == ARTSGameGameMode::MapSelect )
    {
      mapSelectionScreen -> show();
    }
    else if( mode == ARTSGameGameMode::Running ) // in-game
    {
      // show all elements except menu & title elts
      resources -> show();
      rightPanel -> show();
      costWidget -> show();
      tooltip -> show();
      itemBelt -> show();
      buffs -> show();
      building -> show();
      selectBox -> show();
      controls -> show();
      mapSelectionScreen -> hide();
    }
  }

  // layout all widgets based on canvas (screen) size
  void layout( FVector2D canvasSize )
  {
    Size = canvasSize;
    // buffs appear listed in the 
    buffs->Margin = FVector2D( 0, Size.Y / 2 );
    building->Margin = FVector2D( 0, 2*Size.Y / 3 );

    statusBar->Margin = FVector2D( 0, Size.Y - statusBar->Size.Y );
    statusBar->Size.X = Size.X;

    // This becomes full size
    mapSelectionScreen->Size = Size;
  }

};

// Covers entries with a cooldownpie each
//struct SpawnQueue : public StackPanel
//{
//  
//};

