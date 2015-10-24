#include "Wryv.h"

#include "ProgressBar.h"

FLinearColor ProgressBar::Fill(0.169, 0.796, 0.278, 1.f); // nice green
FLinearColor ProgressBar::Bkg(0.655, 0.000, 0.125, 1.f); // pinkish

ProgressBar::ProgressBar( FString name, float height, int alignment ) :
  SolidWidget( name, FVector2D( 100.f, height ), Bkg )
{
  Align = alignment;
  InitFill( Fill );
}

ProgressBar::ProgressBar( FString name, float height, FLinearColor backgroundColor, FLinearColor foregroundColor ) :
  SolidWidget( name, FVector2D( 100.f, height ), backgroundColor )
{
  Align = HFull | VCenter;
  InitFill( foregroundColor );
}

void ProgressBar::Reflow()
{
  SolidWidget::Reflow();
  Set( FillFraction );
}

void ProgressBar::InitFill( FLinearColor color )
{
  progress = new SolidWidget( "foreground progress", color );
  progress->Align = Left | VFull; // |***|..|  Fill the progress bar up vertically
  Add( progress );
  FillFraction = 1.f;
  Set( FillFraction );
}

void ProgressBar::Set( float fraction )
{
  FillFraction = fraction;
  progress->Size.X = FillFraction * Size.X;
}

void ProgressBar::SetColors( FLinearColor foreground, FLinearColor background )
{
  Color = foreground;
  progress->Color = background;
}

void ProgressBar::render( FVector2D offset )
{
  SolidWidget::render( offset );
}
