#pragma once

#include "Blueprint/UserWidget.h"
#include "DialogBox.generated.h"

UCLASS()
class RTSGAME_API UDialogBox : public UUserWidget
{
	GENERATED_UCLASS_BODY()
public:
	virtual void OnWidgetRebuilt() override;
  // The function that gets called on mouse clicks
  UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, Category="Mouse")
  void OkButtonClicked();
};


