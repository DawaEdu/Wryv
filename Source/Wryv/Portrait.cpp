#include "Wryv.h"

#include "Portrait.h"
#include "TheHUD.h"
#include "WryvGameInstance.h"

Portrait::Portrait( FString name, UTexture* tex, float thickness, FLinearColor color ) :
  ImageBorder( name, tex, thickness, color )
  //ImageWidget( name, tex )
{
  // select only this object
  OnMouseDownLeft = [this]( FVector2D mouse ){
    // re-select this object
    Game->hud->Select( { Object } );
    return Consumed;
  };

  hpBar = new ProgressBar( FS( "HpBar %s", *Name ), 12.5f, HFull | Bottom );
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
  }
  else
  {
    SetTexture( NoTextureTexture );
  }
  Show();
}


