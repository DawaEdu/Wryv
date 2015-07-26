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

class HotSpot
{
public:
  string Name; //the name of the widget
  static AHUD* hud;
  // None (null) alignment means positioned absolutely
  enum HAlign { None=0, Left=1<<0, HCenter=1<<1, Right =1<<2, ToLeftOfParent=1<<3, ToRightOfParent=1<<4, HFull=1<<5 };
  enum VAlign { Top =1<<5, VCenter=1<<6, Bottom=1<<7, OnTopOfParent=1<<8,  BelowBottomOfParent=1<<9, VFull=1<<10 };
  int align;
  bool hidden;
  float displayTime; // Amount of time to display remaining
  FVector2D Pos, Size;
  //FVector2D Pad, Margin; // Amount of padding (space inside)
  // and Margin (space outside) this widget.
  //
  // +--------------------+
  // |                    |
  // | +----------------+ |
  // | |                | |
  // | |      text      | |
  // | | padding        | |
  // | +----------------+ |
  // |   margin           |
  // +--------------------+
  
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
  void defaults()
  {
    align = Top | Left;
    hidden = 0;
    displayTime = 0.f;
    Pos = FVector2D(0,0);
    Size = FVector2D(32,32);
    Color = FLinearColor::White;
    Parent = 0;
    dirty = 1;
    bubbleUp = 1;
  }
  HotSpot() {
    defaults();
  }
  HotSpot( FVector2D pos ) {
    defaults();
    Pos = pos;
  }
  HotSpot( FVector2D pos, FVector2D size ) {
    defaults();
    Pos = pos;
    Size = size;
  }
  virtual ~HotSpot(){ Clear(); }
protected:
  virtual void render( FVector2D offset )
  {
    if( hidden ) return;
    // When the base class renders, it calls render on all the children.
    for( int i = 0 ; i < children.size(); i++ )
      children[i]->render( offset + Pos );
  }
public:
  HotSpot* Add( HotSpot* w ) {
    if( w->Parent )  w->Orphan();
    w->Parent = this;
    children.push_back( w );
    // Reflow the widget inside parent
    //repad(); // Recompute Size with new widget inside of it.
    return w;
  }
  void SetByCorners( FVector2D TL, FVector2D BR )
  {
    Pos = TL;
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

  // Resizes the widget given children's size + padding
  void repad() { repad( FVector2D(0,0) ); }
  void repad( FVector2D pad ) {
    // repad's the widget to enclose everything,
    // including the children.
    // *** widget grows infinitely if you push a child to the edge of the bounds
    // each frame, unless you keep it inside within an epsilon. ***
    Size = GetChildBounds().Size() + pad;
  }

  // 
  void realignInParent() { realignInParent( FVector2D(0,0) ); }

  // reflows the contained hotspot to parent widget,
  // using ALIGNMENT settings.
  // ** TODO: can add a margin parameter for flushing outside of widget border.
  void realignInParent( FVector2D pad ) {
    if( !Parent )
    {
      UE_LOG( LogTemp, Warning, TEXT( "Cannot reflush to null parent" ) );
      return; // cannot reflush
    }
    FVector2D PSize = Parent->Size;

    // Null alignment (0) means absolutely positioned (does not reflow in parent)
    if( align & Left )  Pos.X = pad.X;
    else if( align & Right )  Pos.X = PSize.X - Size.X - pad.X;
    else if( align & HCenter )  Pos.X = (PSize.X - Size.X)/2; // centering
    else if( align & ToLeftOfParent )  Pos.X = -Size.X;
    else if( align & ToRightOfParent )  Pos.X = PSize.X;
    else if( align & HFull )  Pos.X = 0, Size.X = PSize.X;

    if( align & Top )  Pos.Y = pad.Y;
    else if( align & Bottom )  Pos.Y = PSize.Y - Size.Y - pad.Y;
    else if( align & VCenter ) Pos.Y = (PSize.Y - Size.Y)/2;
    else if( align & BelowBottomOfParent )  Pos.Y = PSize.Y + pad.Y;
    else if( align & OnTopOfParent )  Pos.Y = -Size.Y - pad.Y;
    else if( align & VFull )  Pos.Y = 0, Size.Y = PSize.Y;
  }

  // Reflows all children into parents according to alignment settings
  void reflow()
  {
    for( int i = 0; i < children.size(); i++ )
      children[i]->reflow();
    realignInParent();
  }

  float left(){ return Pos.X; }
	float right(){ return Pos.X + Size.X; }
	float top(){ return Pos.Y; }
	float bottom(){ return Pos.Y + Size.Y; }
  FVector2D GetAbsPos(){
    // traces back up to the parent and finds the absolute position of this widget
    HotSpot *h = this;
    FVector2D p = Pos;
    while( h->Parent ) { h = h->Parent ; p += h->Pos ; }
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
  
  FBox2DU GetBounds( FVector2D offset ){
    FBox2DU bounds;

    // bound this and all children
    bounds += offset + Pos;
    bounds += offset + Pos + Size;
    for( int i = 0; i < children.size(); i++ ) {
      bounds += children[i]->GetBounds( offset + Pos );
    }
    return bounds;
  }
  FBox2DU GetBounds()
  {
    return GetBounds( FVector2D(0,0) );
  }
  // Get bounds of children, excluding this
  FBox2DU GetChildBounds()
  {
    FBox2DU bounds;
    for( int i = 0; i < children.size(); i++ ) {
      bounds += children[i]->GetBounds( Pos );
    }
    return bounds;
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
    bounds += Pos;
    bounds += Pos + Size;
    return bounds;
  }
  void Clear(){
    for( int i = 0; i < children.size(); i++ )
      delete children[i];
    children.clear();
  }

  bool Act( FVector2D v, FVector2D offset, function< int (FVector2D) > HotSpot::* f )
  {
    // Look into deepest child first.
    for( int i = 0; i < children.size(); i++ )
    {
      // check if a child is hit by the click.
      HotSpot *child = children[i];
      bool h = child->Act( v, offset+Pos, f );  // offset accumulates absolute position offset of widget
      if( h )  return 1; // If the hit was intercepted, return.
    }

    // Actually perform hit checking, if this provides the function f.
    if( this->*f && hit( v - offset ) )
    {
      (this->*f)( v - offset - Pos ); // pass in relative coordinates
      // If the widget doesn't bubble events, then you must return that it hit here

      // If the widget consumes events, then halt propagation
      // Deepest child gets searched, then we return from here
      if( !bubbleUp ) return 1;
    }
    return 0; //not hit or bubbles are allowed.
  }

  // returns true when the point hits the widget (or a child)
  bool Click( FVector2D v, FVector2D offset )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnClicked );
  }
  bool RightClick( FVector2D v, FVector2D offset )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnRightClicked );
  }
  bool Hover( FVector2D v )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnHover );
  }
  bool Drag( FVector2D v )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnDrag );
  }
  bool Drop( FVector2D v )
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

  TextWidget( FString fs, UFont *font=0, float scale=1.f ) : Text( fs ), Font(font), Scale( 1.f )
  {
    Name = TCHAR_TO_UTF8( *fs );
    SetText( fs );
  }
  TextWidget( FString fs, FVector2D pos, UFont *font=0, float scale=1.f ) : 
    HotSpot( pos ), Text( fs ), Font(0), Scale( scale )
  {
    Name = TCHAR_TO_UTF8( *fs );
    SetText( fs );
  }
  virtual ~TextWidget(){}

  // Rendered size of text
  void SetText( FString text )
  {
    Text = text;
    dirty = 1;

    Measure(); // call may fail here, but put it here anyway.
    // [[ cannot call measure here, since may happen
    //    during callback due to mouse motion etc. ]]
  }

  void Measure()
  {
    hud->GetTextSize( Text, Size.X, Size.Y, Font, Scale );
    // ERROR IF THE HUD is not currently ready
    // to draw (ie canvas size won't be available)
  }
  
protected:
  // We call Measure() each call to render() since text cannot be measured except when
  // canvas is ready (ie during render() calls)
  virtual void render( FVector2D offset ) override
  {
    if( hidden ) return;
    if( dirty ) { // It seems in first calls to render(), Measure() does not properly measure text width
      Measure();
      dirty = 0;
    }
    hud->DrawText( Text, Color, Pos.X + offset.X, Pos.Y + offset.Y, Font, Scale );
    HotSpot::render( offset );
  }
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
  //ImageWidget( UTexture* pic, FVector2D pos ) :
  //  Icon( pic ), HotSpot( pos ), uv( 1, 1 ), hotpoint(0,0), Color(FLinearColor::White)
  //{
  //  Name = "ImageWidget";
  //  if( Icon ){
  //    Size.X = Icon->GetSurfaceWidth();
  //    Size.Y = Icon->GetSurfaceHeight();
  //  }
  //}
  ImageWidget( UTexture* pic, FVector2D pos, FVector2D size ) :
    Icon( pic ), HotSpot( pos, size ), uv( 1, 1 ), hotpoint(0,0), Color(FLinearColor::White), Rotation( 0.f )
  {
    if( pic ) Name = TCHAR_TO_UTF8( *pic->GetName() );
  }
  virtual ~ImageWidget(){}

protected:
  virtual void render( FVector2D offset ) override 
  {
    if( hidden ) return;
    FVector2D renderPos = Pos - hotpoint;
    if( !Icon )
    {
      // We have to remove this comment for normal ops because
      // sometimes we want to have null texes eg in slotpalette items when
      // no item is present
      UE_LOG( LogTemp, Warning, TEXT( "Texture not set for ImageWidget" ) );
      // render should not be called when the texture is hidden
    }

    // If hidden, do not draw
    hud->DrawTexture( Icon, renderPos.X + offset.X, renderPos.Y + offset.Y, 
      Size.X, Size.Y, 0, 0, uv.X, uv.Y, Color,
      EBlendMode::BLEND_Translucent, 1.f, 0, Rotation, PivotPoint );
    
    HotSpot::render( offset );
  }
};

class ITextWidget : public ImageWidget
{
  TextWidget* Text;
public:
  FString GetText(){ return Text->Text; }

  ITextWidget( UTexture* pic, FVector2D size, FString ftext, int alignment,
    UFont* font=0, float scale=1.f ) :
    ImageWidget( pic, FVector2D(0,0), size )
  {
    Text = new TextWidget( ftext, font, scale );
    Text->align = alignment;
    Add( Text );
    reflow();
  }
  
};

class SolidWidget : public ImageWidget
{
public:
  static UTexture* SolidWhiteTexture;
  SolidWidget( FLinearColor color ) : ImageWidget( SolidWhiteTexture )
  {
    Color = color;
  }
  SolidWidget( FVector2D pos, FVector2D size, FLinearColor color ) : ImageWidget( SolidWhiteTexture, pos, size )
  {
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
  CooldownPie( FVector2D pos, FVector2D size, float t ) :
    HotSpot( pos, size ), Time(0.f), TotalTime(t)
  {
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
  ResourcesWidget(int pxSize, int spacing, FVector2D pos) : HotSpot(pos), Px(pxSize), Spacing(spacing)
  {
    // +-----------------+
    // |G1000 W1000 S1000|
    // +-----------------+
    // The 3 resource types
    ImageWidget* i = new ImageWidget( GoldTexture, FVector2D(0,0), FVector2D(Px,Px) );
    gold = new TextWidget( "1000", FVector2D(Px,0) );
    i->Add( gold );
    Add( i );
    
    FBox2DU b = i->GetBounds();
    i = new ImageWidget( LumberTexture, FVector2D(b.right()+Spacing,0), FVector2D(Px,Px) );
    lumber = new TextWidget( "1000", FVector2D(Px,0) );
    i->Add( lumber );
    Add( i );
    
    b = i->GetBounds();
    i = new ImageWidget( StoneTexture, FVector2D(b.right()+Spacing,0), FVector2D(Px,Px) );
    stone = new TextWidget( "1000", FVector2D(Px,0) );
    i->Add( stone );
    Add( i );

    Size = GetChildBounds().Size(); // Size of the entire resources widget
  }
  virtual ~ResourcesWidget(){}
  void SetValues( int goldCost, int lumberCost, int stoneCost )
  {
    gold->SetText( FString::Printf( TEXT("%d"), goldCost ) );
    lumber->SetText( FString::Printf( TEXT("%d"), lumberCost ) );
    stone->SetText( FString::Printf( TEXT("%d"), stoneCost ) );
  }
};

// a widget that shows usage costs
class UseWidget : public HotSpot
{
  ImageWidget *mana;
  UseWidget(){}
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
    topText = new TextWidget( "TopText", Interpadding/2 );
    Add(topText);

    //UE_LOG( LogTemp, Warning, TEXT( "topText->GetBounds()" ) );
    FBox2DU bounds = topText->GetBounds();
    bounds.print("toptext");
    cost = new ResourcesWidget(16, 4, FVector2D( Interpadding.X/2, topText->GetBounds().bottom() + vSpacing ) );
    Add(cost);

    //UE_LOG( LogTemp, Warning, TEXT( "cost pos: %f %f" ), cost->Pos.X, cost->Pos.Y );
    //UE_LOG( LogTemp, Warning, TEXT( "cost->GetBounds()" ) );
    bounds = cost->GetBounds();
    bounds.print("costbounds");
    bottomText = new TextWidget( "BottomText", FVector2D( Interpadding.X/2, cost->GetBounds().bottom() + vSpacing ) );
    Add(bottomText);

    //UE_LOG( LogTemp, Warning, TEXT( "bottomText->GetBounds()" ) );
    bounds = bottomText->GetBounds();
    bounds.print("bottomText");
    
    repad( Interpadding );
  }
  virtual ~CostWidget(){}
  void Set( FString top, int goldCost, int lumberCost, int stoneCost, FString bottom )
  {
    topText->SetText( top );
    cost->SetValues( goldCost, lumberCost, stoneCost );
    bottomText->SetText( bottom );
    dirty = 1;
  }

  virtual void render( FVector2D offset ) override
  {
    if( hidden ) return;
    if( dirty )
    {
      repad( Interpadding ) ;
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
  void Set( FString txt )
  {
    Text->SetText( txt );
    // This is the 
    Size = Text->Size + Interspacing; /// cannot re-measure size here.
    Text->Pos = Pos + Interspacing/2;
    hidden = 0; // unhide the tooltip if it was hidden before
    dirty = 1;
  }

  virtual void render( FVector2D offset ) override {
    if( hidden ) return;
    //re-measure the background's size if the text has changed
    if( dirty )
    {
      repad( Interspacing );
      dirty = 0;
    }
    ImageWidget::render( offset );
  }
};

class SlotPalette;

class SlotEntry : public ImageWidget
{
  SlotPalette* Parent; // hides base member
  int Quantity; // the numeric quantity remaining
  TextWidget* TextQuantity;   // The quantity of the item
public:
  SlotEntry( SlotPalette *palette, UTexture* tex, FVector2D pos, FVector2D size, int qty ) :
    Parent(palette), ImageWidget( tex, pos, size ), Quantity( qty )
  {
    TextQuantity = new TextWidget( "A" );
    TextQuantity->align = Right|Bottom;
    Add( TextQuantity );
    dirty = 1;
    //SetQuantity( qty );
    //hidden = 1; // defaults hidden with NULL texture.
  }

  void SetTexture( UTexture* tex, FVector2D originalPos );

  void SetQuantity( int quantity )
  {
    Quantity = quantity;
    TextQuantity->SetText( FString::Printf( TEXT("%d"), quantity ) );
    dirty = 1;
    // 0 qty means qty widget disappears.
    //if( !quantity )  TextQuantity->hidden = 1;
    //else  TextQuantity->hidden = 0;
  }

  virtual void render( FVector2D offset ) override;
};

class SlotPalette : public ImageWidget
{
  ImageWidget* Drag;
  int Rows, Cols;

public:
  FVector2D EntrySize;
  FVector2D Pad; // The padding of each SlotEntry

  static UTexture* SlotPaletteTexture;
  // Drag & Drop for items from slots
  // The item we are dragging (subhotspot)
  SlotPalette( UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D innerPadding ) : 
    Rows(rows), Cols(cols), EntrySize(entrySize), Pad(innerPadding), ImageWidget( bkg )
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
      SlotEntry *iw = new SlotEntry( this, Icon, GetSlotPosition(i), EntrySize, 0 );
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
class StackPanel : public HotSpot
{
public:
  FVector2D EntrySize; // Size of an entry in the stackpanel
  FVector2D Interpadding;  // Padding between entries
  
  StackPanel( FVector2D entrySize, FVector2D interpadding ) :
    HotSpot( FVector2D(0,0), FVector2D(0,0) ),
    EntrySize( entrySize ), Interpadding( interpadding )
  {
  }
  StackPanel( FVector2D pos, FVector2D size, FVector2D entrySize, FVector2D interpadding ) :
    HotSpot( pos, size ), EntrySize( entrySize ), Interpadding( interpadding )
  {
  }
  virtual ~StackPanel(){}
  
  //    _ _
  // > |_|_|
  void StackLeft( ImageWidget* w )
  {
    w->Pos.X = GetBounds().left() - w->Size.X - Interpadding.X;
    w->Size = EntrySize;
    HotSpot::Add( w );
  }

  //  _ _
  // |_|_| <
  void StackRight( ImageWidget* w )
  {
    w->Pos.X = GetBounds().right() + Interpadding.X;
    w->Size = EntrySize;
    HotSpot::Add( w );
  }

  //  v
  //  _
  // |_|
  // |_|
  void StackTop( ImageWidget* w )
  {
    w->Pos.Y = GetBounds().top() - Interpadding.Y - w->Size.Y;
    w->Size = EntrySize;
    HotSpot::Add( w );
  }

  //  _
  // |_|
  // |_|
  //  ^
  void StackBottom( ImageWidget* w )
  {
    FBox2DU b = GetBounds();
    w->Pos.Y = b.bottom() + Interpadding.Y;
    w->Size = EntrySize;
    HotSpot::Add( w );
  }
};

class Minimap : public ImageWidget
{
  Border* borders;
public:
  Minimap( UTexture* icon, FVector2D pos, FVector2D size,
    float borderSize, FLinearColor borderColor ) : 
    ImageWidget( icon, pos, size )
  {
    FBox2DU box( FVector2D(0,0), Size );
    borders = new Border( box, borderSize, borderColor );
    Add( borders );


  }
  virtual ~Minimap(){}
};

class Panel : public ImageWidget
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
  Panel( UTexture* texPanelBkg, UTexture* texMinimap, FVector2D size, float margin ) :
    ImageWidget( texPanelBkg, FVector2D( 0, 0 ), size )
  {
    float s = size.X - 2*margin; // padded inner size
    portrait = new ImageWidget( 0, FVector2D( margin, margin ), FVector2D(s,s) );
    Add( portrait );

    unitStats = new TextWidget( "Stats:", FVector2D( margin, portrait->GetBounds().bottom() + margin ) );
    Add( unitStats );

    SolidWidget *leftBorder = new SolidWidget( FVector2D( -4, 0 ), FVector2D( margin, size.Y ), FLinearColor( 0.1f, 0.1f, 0.1f, 1.f ) );
    Add( leftBorder );

    abilities = new SlotPalette( SlotPalette::SlotPaletteTexture, 2, 3, FVector2D( s/3,s/3 ), FVector2D(8,8) );
    Add( abilities );
    abilities->Pos.X = margin;
    abilities->Pos.Y = unitStats->bottom() + margin;

    minimap = new Minimap( texMinimap, FVector2D(0, abilities->GetBounds().bottom() + margin),
      FVector2D( texMinimap->GetSurfaceWidth(), texMinimap->GetSurfaceHeight() ),
      4.f, FLinearColor( 0.1f, 0.1f, 0.1f, 1.f ) );
    Add( minimap );
    
  }
  virtual ~Panel(){}
};

class StatusBar : public SolidWidget
{
  TextWidget* Text;
public:
  StatusBar( FVector2D canvasSize, float height, FLinearColor bkgColor ) :
    SolidWidget( FVector2D( 0, canvasSize.Y - height ),
    FVector2D( canvasSize.X, height ), bkgColor )
  {
    Text = new TextWidget( "status text" );
    Add( Text );
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
  StackPanel *Panel;
  ImageWidget *Thumbnail;
  ITextWidget *OKButton;
  
  UTexture* SlotBkg;
  ITextWidget* Selected;
  UFont* Font;

  // Constructs the MapSelection Screen.
  MapSelectionScreen( FVector2D canvasSize, 
    UTexture* titleTex, UTexture *slotBkg, FVector2D entrySize,
    FVector2D pad, UFont* font, function< void (FString text) > OnSelect ) :
    HotSpot( FVector2D(0,0), canvasSize ),
    SlotBkg( slotBkg ), Selected( 0 ), Font( font )
  {
    // Throw in the title
    Title = new ImageWidget( titleTex );
    Title->Pos = FVector2D( 75, 50 );
    //Title->align = Left | Top; // no alignment
    Add( Title );

    // The stack of menu items.
    Panel = new StackPanel( entrySize, pad );
    Panel->Pos = FVector2D( 80, 0 );
    Panel->Size = FVector2D( 256, 256 );
    Panel->align = Left | VCenter;
    Add( Panel );

    Thumbnail = new ImageWidget( SlotBkg, FVector2D( 80, 0 ), FVector2D(256,256) );
    Thumbnail->align = Right | VCenter;
    Add( Thumbnail );

    OKButton = new ITextWidget( slotBkg, entrySize, "OK",
      HAlign::Right | VAlign::Bottom );
    OKButton->Pos = FVector2D( 20, 10 );
    OKButton->OnClicked = [this,OnSelect]( FVector2D mouse ){
      OnSelect( Selected->GetText() );
      return 0;
    };
    Add( OKButton );
  }

  // Adds a slot
  ITextWidget* AddText( FString ftext, int alignment )
  {
    ITextWidget* text = new ITextWidget( SlotBkg, Panel->EntrySize, ftext, alignment );
    Add( text );
    // When clicked call Select() on this textNode.
    text->OnClicked = [this,text](FVector2D mouse){
      Select( text );
      return 0;
    };
    
    Panel->StackBottom( text );
    Panel->repad();        // resize the panel with new item in it
    Panel->reflow();

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
class MissionObjectivesScreen : public StackPanel
{
  UTexture *SlotBkg;
public:
  MissionObjectivesScreen( UTexture *slotBkg, FVector2D slotSize, FVector2D pad ) :
    StackPanel( slotSize, pad ),
    SlotBkg( slotBkg )
  {
    
  }

  // Add a series of mission objectives here
  ImageWidget* AddText( FString text, int align )
  {
    ImageWidget* widget = new ImageWidget( SlotBkg, FVector2D(0,0), EntrySize );
    TextWidget* textNode = new TextWidget( text );
    textNode->align = align;
    widget->Size = EntrySize;
    widget->Add( textNode );
    
    Add( widget );
    reflow();

    return widget;
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
  Panel* rightPanel;      // containing: portrait, unitStats, abilities, minimap
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

  UserInterface( FVector2D pos, FVector2D size ) : HotSpot( pos, size ) {
    resources = 0, rightPanel = 0, costWidget = 0, tooltip = 0,
    itemBelt = 0, buffs = 0, building = 0, statusBar = 0,
    selectBox = 0, controls = 0, mapSelectionScreen = 0,
    missionObjectivesScreen = 0, mouseCursor = 0;
  }

  // don't need to explicitly delete children of the UI object
  virtual ~UserInterface(){ }

  void Show( int mode )
  {
    if( mode == ARTSGameGameMode::Title || mode == ARTSGameGameMode::Menu )
    {
      resources -> hidden = 1;
      rightPanel -> hidden = 1;
      costWidget -> hidden = 1;
      tooltip -> hidden = 1;
      itemBelt -> hidden = 1;
      buffs -> hidden = 1;
      building -> hidden = 1;
      statusBar -> hidden = 1;
      selectBox -> hidden = 1;
      controls -> hidden = 1;
      mapSelectionScreen -> hidden = 1;
      missionObjectivesScreen -> hidden = 1;
    }
    
    if( mode == ARTSGameGameMode::Menu )
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
      statusBar -> show();
      selectBox -> show();
      controls -> show();
      mapSelectionScreen -> hide();
    }
  }

  // layout all widgets based on canvas (screen) size
  void layout( FVector2D canvasSize )
  {
    Size = canvasSize;
    rightPanel->Pos.X = Size.X - rightPanel->Size.X;
    // buffs appear listed in the 
    buffs->Pos = FVector2D( 0, Size.Y / 2 );
    building->Pos = FVector2D( 0, 2*Size.Y / 3 );

    statusBar->Pos = FVector2D( 0, Size.Y - statusBar->Size.Y );
    statusBar->Size.X = Size.X;

    // This becomes full size
    //mapSelectionScreen->Size = Size;
    //mapSelectionScreen->reflow();
  }

};

// Covers entries with a cooldownpie each
//struct SpawnQueue : public StackPanel
//{
//  
//};

