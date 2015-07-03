#include "RTSGame.h"
#include "Menu.h"

#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Components/Slider.h"

#include "RTSGameInstance.h"
#include "FlyCam.h"

UMenu::UMenu( const FObjectInitializer& PCIP ) : UUserWidget( PCIP )
{
}

void UMenu::OnWidgetRebuilt()
{
  UE_LOG( LogTemp, Warning, TEXT("OnWidgetRebuilt()") );
  UButton* button;
  button = (UButton*)GetWidgetFromName( TEXT("saveGame") );
  if( button )
    if( !button->OnClicked.IsAlreadyBound( this, &UMenu::SaveGame ) )
      button->OnClicked.AddDynamic( this, &UMenu::SaveGame );

  button = (UButton*)GetWidgetFromName( TEXT("quitGame") );
  if( button )
    if( !button->OnClicked.IsAlreadyBound( this, &UMenu::QuitGame ) )
      button->OnClicked.AddDynamic( this, &UMenu::QuitGame );
  
  button = (UButton*)GetWidgetFromName( TEXT("returnToGame") );
  if( button )
    if( !button->OnClicked.IsAlreadyBound( this, &UMenu::ReturnToGame ) )
      button->OnClicked.AddDynamic( this, &UMenu::ReturnToGame );

  USlider *slider = (USlider*)GetWidgetFromName( TEXT("musicVolume") );
  if( slider )
    if( !slider->OnValueChanged.IsAlreadyBound( this, &UMenu::musicVolumeChanged ) )
      slider->OnValueChanged.AddDynamic( this, &UMenu::musicVolumeChanged );

  slider = (USlider*)GetWidgetFromName( TEXT("sfxVolume") );
  if( slider )
    if( !slider->OnValueChanged.IsAlreadyBound( this, &UMenu::sfxVolumeChanged ) )
      slider->OnValueChanged.AddDynamic( this, &UMenu::sfxVolumeChanged );

}

// The function that gets called on mouse clicks
void UMenu::SaveGame_Implementation()
{
  UE_LOG( LogTemp, Warning, TEXT("SaveGame()") );
}

void UMenu::QuitGame_Implementation()
{
  UE_LOG( LogTemp, Warning, TEXT("QuitGame()") );
}

void UMenu::ReturnToGame_Implementation()
{
  UE_LOG( LogTemp, Warning, TEXT("ReturnToGame()") );
  RemoveFromViewport();
}

void UMenu::musicVolumeChanged_Implementation( float v )
{
  UE_LOG( LogTemp, Warning, TEXT("musicVolumeChanged(%f)"), v );
  Game->flycam->music->SetVolumeMultiplier( v );
}

void UMenu::sfxVolumeChanged_Implementation( float v )
{
  UE_LOG( LogTemp, Warning, TEXT("sfxVolumeChanged(%f)"), v );
  Game->flycam->sfxVolume = v;
}


