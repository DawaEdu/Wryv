#include "RTSGame.h"
#include "DialogBox.h"

#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

UDialogBox::UDialogBox( const FObjectInitializer& PCIP ) : UUserWidget( PCIP )
{
  
}

void UDialogBox::OnWidgetRebuilt()
{
  UE_LOG( LogTemp, Warning, TEXT("OnWidgetRebuilt()") );
  UButton* button;
  button = (UButton*)GetWidgetFromName( TEXT("okButton") );
  if( button ){
    // make sure the function hasn't already been attached (which happens sometimes
    // when widget being rebuilt didn't have buttons discarded)
    if( !button->OnClicked.IsAlreadyBound( this, &UDialogBox::OkButtonClicked ) )
      button->OnClicked.AddDynamic( this, &UDialogBox::OkButtonClicked );
  }
}

// The function that gets called on mouse clicks
void UDialogBox::OkButtonClicked_Implementation()
{
  UE_LOG( LogTemp, Warning, TEXT("OkButtonClicked()") );
  RemoveFromViewport();
}


