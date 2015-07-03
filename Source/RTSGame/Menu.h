#pragma once

#include "Blueprint/UserWidget.h"
#include "Menu.generated.h"

UCLASS()
class RTSGAME_API UMenu : public UUserWidget
{
	GENERATED_UCLASS_BODY()
public:
	virtual void OnWidgetRebuilt() override;

  // The function that gets called on mouse clicks
  UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, Category="Mouse")  void SaveGame();
  UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, Category="Mouse")  void QuitGame();
  UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, Category="Mouse")  void ReturnToGame();

  UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, Category="Mouse")  void musicVolumeChanged( float v );
  UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, Category="Mouse")  void sfxVolumeChanged( float v );
  
};


