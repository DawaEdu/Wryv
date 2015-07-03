#pragma once

#include "Camera/PlayerCameraManager.h"
#include "CameraMan.generated.h"

UCLASS()
class RTSGAME_API ACameraMan : public APlayerCameraManager
{
	GENERATED_BODY()
	
	virtual void UpdateCamera( float DeltaTime );
	
};
