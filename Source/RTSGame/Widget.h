#pragma once

#include <vector>
#include <functional>
using namespace std;
class AHUD;

struct AABB2D
{
  FVector2D mins, maxes;
  AABB2D() : mins( 1e6, 1e6 ), maxes( -1e6, -1e6 ) { }
  float left(){ return mins.X; }
	float right(){ return maxes.X; }
	float top(){ return mins.Y; }
	float bottom(){ return maxes.Y; }
	bool hit( FVector2D v )
	{
		// +---+ top (0)
		// |   |
		// +---+ bottom (2) (bottom > top)
		// L   R
		return left() < v.X   &&   v.X < right() &&
           top() < v.Y   &&   v.Y < bottom();
	}
  inline void Add( const FVector2D& v ) {
    if( v.X < mins.X )  mins.X = v.X;
    else if( v.X > maxes.X )  maxes.X = v.X;

    if( v.Y < mins.Y )  mins.Y = v.Y;
    else if( v.Y > maxes.Y )  maxes.Y = v.Y;
  }
  inline void Add( const AABB2D& aabb ) {
    Add( aabb.mins );
    Add( aabb.maxes );
  }
};

class AGameObject;

struct HotSpot
{
  static AHUD* hud;

  FVector2D Pos, Size;
  FLinearColor Color;
  // Associations with AGameObject's contained in this hotspot. Empty if no objects
  // are associated with the spot (used for item slots)
  vector<AGameObject*> Objects;
protected:
  vector<HotSpot*> children;  // Children of this hotspot.
public:
  function< void () > OnClicked;
  function< void () > OnHover;
  function< void () > OnDrag;
  // A function that runs when the widget is dropped @ certain location
  function< void (FVector2D dropLocation) > OnDrop;

  HotSpot() : Pos(0,0), Size(32,32), Color( FLinearColor::White ) {}
  HotSpot( FVector2D pos ) : Pos(pos), Size(32,32), Color( FLinearColor::White ) {}
  HotSpot( FVector2D pos, FVector2D size ) : Pos( pos ), Size( size ), Color( FLinearColor::White )
  {
  }
  virtual ~HotSpot(){}
protected:
  virtual void render( FVector2D offset )
  {
    // When the base class renders, it calls render on all the children.
    for( int i = 0 ; i < children.size(); i++ )
      children[i]->render( offset + Pos );
  }
public:
  void Add( HotSpot* w ) { children.push_back( w ); }
  virtual void render(){ render( FVector2D(0,0) ); }
  
  float left(){ return Pos.X; }
	float right(){ return Pos.X + Size.X; }
	float top(){ return Pos.Y; }
	float bottom(){ return Pos.Y + Size.Y; }
	//bool hit( FVector2D v )
	//{
	//	// +---+ top (0)
	//	// |   |
	//	// +---+ bottom (2) (bottom > top)
	//	// L   R
	//	return left() < v.X   &&   v.X < right() &&
  //         top()  < v.Y   &&   v.Y < bottom();
	//}
  // Check main bounds
  bool hit( FVector2D v ) { return GetBounds( FVector2D(0,0) ).hit( v ); }
  FVector2D getHitPercent( FVector2D v ) { return (v - Pos)/Size; }
  AABB2D GetBounds( FVector2D offset ){
    AABB2D bounds;
    // bound this and all children
    bounds.Add( offset + Pos );
    bounds.Add( offset + Pos + Size );
    for( int i = 0; i < children.size(); i++ ) {
      bounds.Add( children[i]->GetBounds( Pos ) );
    }
    return bounds;
  }
  AABB2D GetBounds() { return GetBounds( FVector2D(0,0) ); }
  void Clear(){
    for( int i = 0; i < children.size(); i++ )
      delete children[i];
    children.clear();
  }
  void Click( FVector2D v ) {
    // Check if hit parent. sometimes children are outside parent, which
    // is why
    if( OnClicked && hit( v ) )  OnClicked();
    for( int i = 0; i < children.size(); i++ )
      children[i]->Click( v - Pos ); // relatively position click point to parent
  }
  void Hover( FVector2D v ) {
    if( OnHover && hit( v ) )  OnHover();
    for( int i = 0; i < children.size(); i++ )
      children[i]->Hover( v - Pos );
  }
};

struct TextWidget : public HotSpot
{
private:
  FString Text;

public:
  TextWidget( FString fs ) : Text( fs )
  {
    // Measures size in Text using HUD object (must be init & ready)
    SetText( fs );
  }
  TextWidget( FString fs, FVector2D pos, FVector2D size ) : HotSpot( pos, size ), Text( fs )
  {
    SetText( fs );
  }
  virtual ~TextWidget(){}

  // Rendered size of text
  void SetText( FString text )
  {
    Text = text;
    hud->GetTextSize( Text, Size.X, Size.Y );
  }
  virtual void render() override { HotSpot::render(); }

protected:
  void render( FVector2D offset ) override
  {
    hud->DrawText( Text, Color, Pos.X + offset.X, Pos.Y + offset.Y );
    HotSpot::render( offset );
  }
  
};

struct ImageWidget : public HotSpot
{
  UTexture* Icon;
  FVector2D uv; // The maximum coordinates of the UV texturing
  FVector2D hotpoint; // Usually top left corner (0,0), meaning will render from topleft corner.
  // if its half size, then it will render from the center (such as when an imageWidget is being
  // click-dragged
  
  ImageWidget( UTexture* pic ) : Icon( pic ), uv( 1, 1 ), hotpoint(0,0)
  {
    if( Icon ){
      Size.X = Icon->GetSurfaceWidth();
      Size.Y = Icon->GetSurfaceHeight();
    }
  }
  ImageWidget( UTexture* pic, FVector2D pos ) :
    Icon( pic ), HotSpot( pos ), uv( 1, 1 ), hotpoint(0,0)
  {
    if( Icon ){
      Size.X = Icon->GetSurfaceWidth();
      Size.Y = Icon->GetSurfaceHeight();
    }
  }
  ImageWidget( UTexture* pic, FVector2D pos, FVector2D size ) :
    Icon( pic ), HotSpot( pos, size ), uv( 1, 1 ), hotpoint(0,0)
  {
  }
  virtual ~ImageWidget(){}
  virtual void render() override { HotSpot::render(); }

protected:
  virtual void render( FVector2D offset ) override 
  {
    FVector2D renderPos = Pos - hotpoint;
    hud->DrawTexture( Icon, renderPos.X + offset.X, renderPos.Y + offset.Y, Size.X, Size.Y, 0, 0, uv.X, uv.Y );
    HotSpot::render( offset );
  }
};

// starting offset point and group of widgets to draw

// Make a custom widget for the overlays etc
struct ResourcesWidget : public HotSpot
{
  TextWidget *gold, *lumber, *stone;
  static UTexture *GoldTexture, *LumberTexture, *StoneTexture;
  int Px; // size of the icons

  // Add in all the subwidgets
  ResourcesWidget(int pxSize) : Px(pxSize)
  {
    // +-----------------+
    // |G1000 W1000 S1000|
    // +-----------------+
    // 
    // The 3 resource types
    ImageWidget* i = new ImageWidget( GoldTexture, FVector2D(0,0), FVector2D(Px,Px) );
    gold = new TextWidget( "1000", FVector2D(Px,0), FVector2D(Px,Px) );
    i->Add( gold );
    Add( i );
    
    AABB2D b = i->GetBounds();
    i = new ImageWidget( LumberTexture, FVector2D(b.right(),0), FVector2D(Px,Px) );
    lumber = new TextWidget( "1000", FVector2D(Px,0), FVector2D(Px,Px) );
    i->Add( lumber );
    Add( i );
    
    b = i->GetBounds();
    i = new ImageWidget( StoneTexture, FVector2D(b.right(),0), FVector2D(Px,Px) );
    stone = new TextWidget( "1000", FVector2D(Px,0), FVector2D(Px,Px) );
    i->Add( stone );
    Add( i );
  }

  void SetValues( int goldCost, int lumberCost, int stoneCost )
  {
    gold->SetText( FString::Printf( TEXT("%d"), goldCost ) );
    lumber->SetText( FString::Printf( TEXT("%d"), lumberCost ) );
    stone->SetText( FString::Printf( TEXT("%d"), stoneCost ) );
  }
};

// a widget that shows usage costs
struct UseWidget : public HotSpot
{
  ImageWidget *mana;
};

struct CostWidget : public HotSpot
{
  TextWidget* topText;
  ResourcesWidget* cost;
  TextWidget* bottomText;

  // +-------------------------------------+
  // | Barracks                            |
  // | [] gold [] lumber [] stone          |
  // | Barracks are used to build militia. |
  // +-------------------------------------+
  // Using a popup for the UI information about the build costs
  // makes it so new players don't actually have to look for it.
  // It pops up, so you always notice it right away.
  CostWidget()
  {
    topText = new TextWidget( "TopText", FVector2D(0,0), FVector2D(16,0) );
    Add(topText);
    cost = new ResourcesWidget(16);
    Add(cost);
    bottomText = new TextWidget( "BottomText", FVector2D(0,44), FVector2D(16,0) );
    Add(bottomText);
  }

  void SetCosts( int goldCost, int lumberCost, int stoneCost )
  {
    cost->SetValues( goldCost, lumberCost, stoneCost );
  }
};

struct Tooltip : public ImageWidget
{
private:
  TextWidget* Text;

public:
  FVector2D Pad;
  Tooltip( UTexture* bkg, FString text, FVector2D pad ) : ImageWidget( bkg ), Pad(pad)
  {
    Text = new TextWidget( text );
    SetText( text );
  }

  void SetText( FString txt )
  {
    Text->SetText( txt );
    Size = Text->Size + 2*Pad;
    Pos = Text->Pos - Pad;
  }
};

struct SlotPalette : public ImageWidget
{
  int Rows, Cols;
  FVector2D SlotSize, UVFraction;
  
  // Drag & Drop for items from slots
  // The item we are dragging (subhotspot)
  ImageWidget* Drag;
  
  SlotPalette( int rows, int cols, UTexture* tex, FVector2D pos, FVector2D size ) : 
    Rows(rows), Cols(cols), ImageWidget( tex, pos, size )
  {
    // Init w/ # slots used in this palette
    SlotSize = FVector2D( 100.f, 100.f );
    // Measure the UV coordinates used since the texture is 6x6
    Drag = 0;
    SetNumSlots( Rows, Cols );
  }

  FVector2D GetSlotPosition( int i )
  {
    FVector2D v;
    int row = i / Cols;
    int col = i % Cols;
    v.X = col * SlotSize.X;
    v.Y = row * SlotSize.Y;
    return v;
  }

  // Direct-children of SlotPalette must be ImageWidget*.
  // Guaranteed to be ImageWidget* because Add is protected.
  // External users of class cannot add a textwidget for example to the slotpanel.
  // (make sure you don't add children to slotpanel that are not imageWidgets)
  // the children of an imagewidget CAN be textnodes etc.
  ImageWidget* GetSlot( int i )
  {
    return (ImageWidget*)children[i];
  }

  void SetSlot( int i, UTexture* icon )
  {
    GetSlot(i)->Icon = icon;
  }

  // Widget's by themselves are just hotspots
  vector<ImageWidget*> SetNumSlots( int rows, int cols )
  {
    Rows = rows;
    Cols = cols;
    int numSlots = rows*cols;

    uv = FVector2D( Cols/6., Rows/6. );
    Clear();    // Remove the old ImageWidgets.
    
    // The size of this widget set here.
    Size = FVector2D( Cols, Rows ) * SlotSize;
    vector<ImageWidget*> slots;
    for( int i = 0; i < numSlots; i++ )
    {
      ImageWidget *iw = new ImageWidget( Icon, GetSlotPosition(i), SlotSize );
      slots.push_back( iw );
      Add( iw );
    }
    return slots;
  }

};

struct Panel : public ImageWidget
{
  ImageWidget* Icon;      // pictoral representation of selected unit
  TextWidget* UnitStats;  // The stats of the last selected unit

  // For the group of selected units.
  Panel( UTexture* tex, FVector2D size, float margin ) :
    ImageWidget( tex, FVector2D( 0, 0 ), size )
  {
    float s = size.X - 2*margin;
    Icon = new ImageWidget( 0, FVector2D( margin, margin ), FVector2D(s,s) );
    Add( Icon );

    UnitStats = new TextWidget( "Stats:", FVector2D( margin, size.Y/3 ), FVector2D(s,400) );
    Add( UnitStats );
  }
};

// Supports stacking-in of widgets from left/right or top/bottom
struct StackPanel : public HotSpot
{
  FVector2D IconSize;
  FVector2D Margins;
  StackPanel( FVector2D iconSize, FVector2D margins ) :
    IconSize( iconSize ), Margins( margins )
  {
  }
  
  //    _ _
  // > |_|_|
  void StackLeft( ImageWidget* w )
  {
    w->Pos.X = GetBounds().left() - w->Size.X - Margins.X;
    HotSpot::Add( w );
  }

  //  _ _
  // |_|_| <
  void StackRight( ImageWidget* w )
  {
    w->Pos.X = GetBounds().right() + Margins.X;
    HotSpot::Add( w );
  }

  //  v
  //  _
  // |_|
  // |_|
  void StackTop( ImageWidget* w )
  {
    w->Pos.Y = GetBounds().top() - Margins.Y - w->Size.Y;
    HotSpot::Add( w );
  }

  //  _
  // |_|
  // |_|
  //  ^
  void StackBottom( ImageWidget* w )
  {
    w->Pos.Y = GetBounds().bottom() + Margins.Y;
    HotSpot::Add( w );
  }

  void Remove( int i )
  {
    // remove by index
  }
};

// Covers entries with a cooldownpie each
//struct SpawnQueue : public StackPanel
//{
//  
//};

