#include "RTSGame.h"
#include "TipsBox.h"

#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Components/TextBlock.h"
#include "Runtime/UMG/Public/Components/MultiLineEditableTextBox.h"

#include "RTSGameInstance.h"
#include "FlyCam.h"

UTipsBox::UTipsBox( const FObjectInitializer& PCIP ) : UUserWidget( PCIP )
{
  
}

void UTipsBox::OnWidgetRebuilt()
{
  UButton* button;
  button = (UButton*)GetWidgetFromName( TEXT("okButton") );
  if( button )
    if( !button->OnClicked.IsAlreadyBound( this, &UTipsBox::OkButtonClicked ) )
      button->OnClicked.AddDynamic( this, &UTipsBox::OkButtonClicked );

  button = (UButton*)GetWidgetFromName( TEXT("nextTip") );
  if( button )
    if( !button->OnClicked.IsAlreadyBound( this, &UTipsBox::NextTipButtonClicked ) )
      button->OnClicked.AddDynamic( this, &UTipsBox::NextTipButtonClicked );
  tip = (UMultiLineEditableTextBox*)GetWidgetFromName( TEXT("tip") );
}

// The function that gets called on mouse clicks
void UTipsBox::OkButtonClicked_Implementation()
{
  UE_LOG( LogTemp, Warning, TEXT("UTipsBox::OkButtonClicked()") );
  RemoveFromViewport();

  //Game->flycam->LoadLevel( Game->flycam->LevelMap );

}

void UTipsBox::NextTipButtonClicked_Implementation()
{
  UE_LOG( LogTemp, Warning, TEXT("UTipsBox::NextTipButtonClicked()") );
  // This connects via the global Game object to invoke a function on flycam
  Game->flycam->NextTip();
}

void UTipsBox::SetText( FString text )
{
  //tip->SetText( FText::FromString(text) );
  tip = (UMultiLineEditableTextBox*)GetWidgetFromName( TEXT("tip") ); // refresh variable from widget
  tip->SetText( FText::FromString(text) );
}



