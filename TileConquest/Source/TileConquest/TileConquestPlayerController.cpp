// Copyright Epic Games, Inc. All Rights Reserved.

#include "TileConquestPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "TileConquestCharacter.h"
#include "Engine/World.h"
#include <MoveCommand.h>

TArray<MoveCommand*> MoveCommands;
// TQueue<MoveCommand*> MoveCommands;
MoveCommand* m_CurrentMoveCommand;
int moveCommandIndex = 0;

ATileConquestPlayerController::ATileConquestPlayerController()
{
	MoveCommands.Empty();
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	m_CurrentMoveCommand = new MoveCommand(this);
	// MoveCommands.Enqueue(m_CurrentMoveCommand);
	// auto firstMoveCommand = new MoveCommand(this);
	// MoveCommands.Add(firstMoveCommand);
}

void ATileConquestPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
}

void ATileConquestPlayerController::DecreaseMoveCommandCount()
{
	moveCommandIndex--;
}

bool ATileConquestPlayerController::IsTileInRange(ATile* DestinationTile)
{
	// If the current tile is null, allow the player to move there. This avoids a crash!
	if (CurrentTile == nullptr)
	{
		return true;
	}

	// If the tile is more than 1 tile's worth distance away, ignore the input
	FVector currLocation = CurrentTile->GetActorLocation();
	FVector targetLocation = DestinationTile->GetActorLocation();
	return FVector::Distance(currLocation, targetLocation) < 151.f;
}

bool ATileConquestPlayerController::MoveTo(ATile* destinationTile, bool activateStep, bool isUndoOrRedo)
{
	// If the destination is null, we have likely hit the end of the undo / redo chain
	// This prevents a crash!	
	if (destinationTile == nullptr)
	{
		return false;
	}

	if (IsTileInRange(destinationTile))
	{
		FVector HitLocation = destinationTile->GetActorLocation();
		CurrentTile = destinationTile;

		// We move there and spawn some particles
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, HitLocation);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, HitLocation, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);

		// Finally, update the tile to react to the step
		if (activateStep)
		{
			destinationTile->StepOn();
		}

		// Only add a movement command if it wasn't an undo or redo
		if (!isUndoOrRedo)
		{
			MoveCommands.Add(new MoveCommand(m_CurrentMoveCommand));
			moveCommandIndex = MoveCommands.Num() - 1;
		}

		auto intstring = FString::FromInt(moveCommandIndex);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, intstring);

		return true;
	}
	else
	{
		return false;
	}
}

ATile* ATileConquestPlayerController::GetCurrentTile() 
{
	return CurrentTile;
}

void ATileConquestPlayerController::UndoCallback()
{
	/*
	if (MoveCommands[moveCommandIndex]->PreviousTile == nullptr)
	{
		// ignore undo command
		return;
	}
	MoveCommands[moveCommandIndex]->undo();
	moveCommandIndex--;
	if (moveCommandIndex < 0)
	{
		moveCommandIndex = 0;
	}
	*/
	// Don't let the player undo if they haven't done anything yet!
	if (MoveCommands.Num() > 0)
	{
		// Don't let the player undo past the first move
		if (moveCommandIndex >= 0)
		{
			MoveCommands[moveCommandIndex]->undo();
			// moveCommandIndex--;
		}
	}

}

void ATileConquestPlayerController::RedoCallback()
{
	/*
	if (MoveCommands[moveCommandIndex]->PreviousTile == nullptr)
	{
		// ignore undo command
		return;
	}
	MoveCommands[moveCommandIndex]->undo();
	moveCommandIndex--;
	if (moveCommandIndex < 0)
	{
		moveCommandIndex = 0;
	}
	*/
	// The player hasn't done anything yet!
	if (m_CurrentMoveCommand != nullptr)
	{
		// Don't let the player redo a move they haven't made yet
		if (moveCommandIndex < MoveCommands.Num() - 1)
		{
			moveCommandIndex++;
			MoveCommands[moveCommandIndex]->redo();
		}
	}
}

void ATileConquestPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAction("SetDestination", IE_Pressed, this, &ATileConquestPlayerController::OnSetDestinationPressed);
	InputComponent->BindAction("SetDestination", IE_Released, this, &ATileConquestPlayerController::OnSetDestinationReleased);
	InputComponent->BindAction
	(
		"Undo", // The input identifier (specified in DefaultInput.ini)
		IE_Pressed, // React when button pressed (or on release, etc., if desired)
		this, // The object instance that is going to react to the input
		&ATileConquestPlayerController::UndoCallback // The function that will fire when input is received
	);
	InputComponent->BindAction
	(
		"Redo", // The input identifier (specified in DefaultInput.ini)
		IE_Pressed, // React when button pressed (or on release, etc., if desired)
		this, // The object instance that is going to react to the input
		&ATileConquestPlayerController::RedoCallback // The function that will fire when input is received
	);

	// support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ATileConquestPlayerController::OnTouchPressed);
	InputComponent->BindTouch(EInputEvent::IE_Released, this, &ATileConquestPlayerController::OnTouchReleased);

}

void ATileConquestPlayerController::OnSetDestinationPressed()
{
	// Just in case the character was moving because of a previous short press we stop it
	// StopMovement();
}

void ATileConquestPlayerController::CleanUpCommandList()
{
	/*
	auto sizeOfArray = MoveCommands.Num() - 1;
	for (int i = moveCommandIndex+1; i < sizeOfArray; i++)
	{
		MoveCommands.RemoveAt(MoveCommands.Num() - 1);
		// MoveCommands.RemoveAt(i);
	}
	for(MoveCommand* movecomm : MoveCommands)
	{
		if (movecomm->PreviousTile == nullptr)
		{
			MoveCommands.Remove(movecomm);
		}
	}
	*/
}

void ATileConquestPlayerController::OnSetDestinationReleased()
{
	// We look for the location in the world where the player has pressed the input
	FVector HitLocation = FVector::ZeroVector;
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, true, Hit);

	AActor* hitActor = Hit.GetActor();
	if (hitActor != nullptr)
	{
		if (hitActor->IsA(ATile::StaticClass()))
		{
			ATile* tileActor = Cast<ATile>(hitActor);
			// m_CurrentMoveCommand = new MoveCommand(this);
			m_CurrentMoveCommand->execute(tileActor);
			/*
			auto moveCommand = new MoveCommand(this);
			moveCommand->execute(tileActor);
			moveCommandIndex++;
			*/

			//CleanUpCommandList();
			//MoveCommands.Add(moveCommand);
			// MoveTo(tileActor);
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
