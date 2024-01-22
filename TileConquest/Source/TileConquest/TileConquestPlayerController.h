// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerController.h"
#include <Tile.h>
#include "TileConquestPlayerController.generated.h"

/** Forward declaration to improve compiling times */
class UNiagaraSystem;

UCLASS()
class ATileConquestPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATileConquestPlayerController();

	/** Time Threshold to know if it was a short press */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	float ShortPressThreshold;

	/** FX Class that we will spawn when clicking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UNiagaraSystem* FXCursor;

	/** Sound to play when stepping */
	UPROPERTY(EditAnywhere, Category = "sound")
	class USoundBase* GlassBreakSound;
	/** Ambient Background sound */
	UPROPERTY(EditAnywhere, Category = "sound")
	class USoundBase* AmbienceSound;
	/** Sound to play when the level is complete */
	UPROPERTY(EditAnywhere, Category = "sound")
	class USoundBase* SuccessSound;

	bool MoveTo(ATile* destinationTile, bool activateStep, bool isUndoOrRedo);
	ATile* GetCurrentTile();
	void DecreaseMoveCommandCount();

protected:
	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	// End PlayerController interface

	/** Input handlers for SetDestination action. */
	void OnSetDestinationPressed();
	void OnSetDestinationReleased();
	void OnTouchPressed(const ETouchIndex::Type FingerIndex, const FVector Location);
	void OnTouchReleased(const ETouchIndex::Type FingerIndex, const FVector Location);

private:
	bool bInputPressed; // Input is bring pressed
	bool bIsTouch; // Is it a touch device
	float FollowTime; // For how long it has been pressed
	ATile* CurrentTile;
	bool CanMove;

	bool IsTileInRange(ATile* DestinationTile);
	void UndoCallback();
	void RedoCallback();

	void CheckIfLevelComplete();
};



