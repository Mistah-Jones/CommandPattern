// Copyright Epic Games, Inc. All Rights Reserved.

#include "TileConquestPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "TileConquestCharacter.h"
#include "Engine/World.h"

ATileConquestPlayerController::ATileConquestPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void ATileConquestPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	/*
	if(bInputPressed)
	{
		FollowTime += DeltaTime;

		// Look for the touch location
		FVector HitLocation = FVector::ZeroVector;
		FHitResult Hit;
		if(bIsTouch)
		{
			GetHitResultUnderFinger(ETouchIndex::Touch1, ECC_Visibility, true, Hit);
		}
		else
		{
			GetHitResultUnderCursor(ECC_Visibility, true, Hit);
		}
		AActor* hitActor = Hit.GetActor();

		HitLocation = hitActor->GetActorLocation();

		// GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, hitActor);


		// Direct the Pawn towards that location
		APawn* const MyPawn = GetPawn();
		if(MyPawn && hitActor->IsA(ATile::StaticClass()))
		{
			FVector WorldDirection = (HitLocation - MyPawn->GetActorLocation()).GetSafeNormal();
			MyPawn->AddMovementInput(WorldDirection, 1.f, false);
		}
	}
	else
	{
		FollowTime = 0.f;
	}
	*/
}

bool ATileConquestPlayerController::IsTileInRange(ATile* DestinationTile)
{
	// If the tile is more than 1 tile's worth distance away, ignore the input
	FVector currLocation = CurrentTile->GetActorLocation();
	FVector targetLocation = DestinationTile->GetActorLocation();
	return FVector::Distance(currLocation, targetLocation) > 51.f;
}


void ATileConquestPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAction("SetDestination", IE_Pressed, this, &ATileConquestPlayerController::OnSetDestinationPressed);
	InputComponent->BindAction("SetDestination", IE_Released, this, &ATileConquestPlayerController::OnSetDestinationReleased);

	// support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ATileConquestPlayerController::OnTouchPressed);
	InputComponent->BindTouch(EInputEvent::IE_Released, this, &ATileConquestPlayerController::OnTouchReleased);

}

void ATileConquestPlayerController::OnSetDestinationPressed()
{
	// We flag that the input is being pressed
	bInputPressed = true;
	// Just in case the character was moving because of a previous short press we stop it
	StopMovement();
}

void ATileConquestPlayerController::OnSetDestinationReleased()
{
	// Player is no longer pressing the input
	bInputPressed = false;

	// If it was a short press
	if(FollowTime <= ShortPressThreshold)
	{
		// We look for the location in the world where the player has pressed the input
		FVector HitLocation = FVector::ZeroVector;
		FHitResult Hit;
		GetHitResultUnderCursor(ECC_Visibility, true, Hit);

		AActor* hitActor = Hit.GetActor();
		if (hitActor->IsA(ATile::StaticClass()))
		{
			ATile* tileActor = Cast<ATile>(hitActor);
			if (IsTileInRange(tileActor))
			{
				HitLocation = hitActor->GetActorLocation();
				CurrentTile = tileActor;

				// We move there and spawn some particles
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, HitLocation);
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, HitLocation, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);

			}

		}
	}
}

void ATileConquestPlayerController::OnTouchPressed(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	bIsTouch = true;
	OnSetDestinationPressed();
}

void ATileConquestPlayerController::OnTouchReleased(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	bIsTouch = false;
	OnSetDestinationReleased();
}
