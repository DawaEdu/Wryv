#include "Wryv.h"

#include "UI/HotSpot/Elements/Portrait.h"
#include "UE4/TheHUD.h"
#include "UE4/WryvGameInstance.h"

Portrait::Portrait( FString name, UTexture* tex, float thickness, FLinearColor color ) :
  BorderImage( name, tex, thickness, color )
  //ImageHS( name, tex )
{
  // select only this object
  OnMouseDownLeft = [this]( FVector2D mouse ){
    // re-select this object
    Game->hud->Select( { Object } );
    return Consumed;
  };

  teamColor = new Solid( "Portrait widget", FVector2D( 4.f, 4.f ), FLinearColor::White );
  teamColor->Align = HAlign::HFull | VAlign::Bottom;
  Add( teamColor );

  hpBar = new ProgressBar( FS( "HpBar %s", *Name ), 12.5f, HFull | Bottom );
  hpBar->Margin.Y = 4.f;
  Add( hpBar );
}

void Portrait::Set( AGameObject* go )
{
  Object = go;
  if( Object )
  {
    SetTexture( Object->Stats.Portrait );
    SetName( FS( "Portrait `%s`", *Object->GetName() ) );
    hpBar->Set( go->HpFraction() );
    teamColor->Color = go->team->Color;
  }
  else
  {
    SetTexture( NoTextureTexture );
  }
  Show();
}


