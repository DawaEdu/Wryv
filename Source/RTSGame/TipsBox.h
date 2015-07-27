#pragma once
#include "Blueprint/UserWidget.h"
#include "TipsBox.generated.h"

class UMultiLineEditableTextBox;

UCLASS()
class RTSGAME_API UTipsBox : public UUserWidget
{
  GENERATED_UCLASS_BODY()
public:
  UMultiLineEditableTextBox *tip;

  virtual void OnWidgetRebuilt() override;
  // The function that gets called on mouse clicks to a UI button
  UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, Category="Mouse")
  void OkButtonClicked();

  UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, Category="Mouse")
  void NextTipButtonClicked();

  // Changes the text in the tip.
  void Set( FString text );
};


