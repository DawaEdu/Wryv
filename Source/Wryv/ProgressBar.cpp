#include "Wryv.h"
#include "ProgressBar.h"

ProgressBar::ProgressBar( FString name, float height, FLinearColor backgroundColor, FLinearColor foregroundColor ) :
  SolidWidget( name, FVector2D( 100.f, height ), backgroundColor )
{
  progress = new SolidWidget( "foreground progress", foregroundColor );
  Add( progress );
  Align = HFull | VCenter;
  progress->Align = Left | VFull;

  Set( 0.f );
}

void ProgressBar::Set( float fraction )
{
  progress->Size.X = fraction * Size.X;
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
