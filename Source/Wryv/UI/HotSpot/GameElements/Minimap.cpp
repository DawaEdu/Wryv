#include "Wryv.h"

#include "UE4/Flycam.h"
#include "UI/HotSpot/GameElements/Minimap.h"
#include "UE4/TheHUD.h"
#include "UE4/WryvGameInstance.h"

UTexture* Minimap::MinimapTexture = 0;

Minimap::Minimap( float borderThickness, FLinearColor borderColor ) : 
  BorderImage( "minimap", MinimapTexture, borderThickness, borderColor )
{
  OnMouseDownLeft = [this](FVector2D mouse) -> EventCode {
    Game->flycam->SetCameraPosition( mouse / Size );
    return Consumed;
  };
  OnMouseDragLeft = [this](FVector2D mouse) -> EventCode {
    Game->flycam->SetCameraPosition( mouse / Size );
    return Consumed;
  };
}

void Minimap::render( FVector2D offset )
{
  BorderImage::render( offset );
  // Convert groundIntnPts to pts on screen in screenspace of thing
  //for( int i = 0; i < pts.size()-1; i++ )
  //  DrawDebugLine( UGameplayStatics::GetWorld(), pts[i], pts[i+1], FColor( 100, 100, 100, 100 ) );
  //if( pts.size() > 1 )
  //  DrawDebugLine( UGameplayStatics::GetWorld(), pts[pts.size()-2], pts[pts.size()-1], FColor( 100, 100, 100, 100 ) );

}



