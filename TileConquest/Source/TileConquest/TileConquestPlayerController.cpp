// Copyright Epic Games, Inc. All Rights Reserved.

#include "TileConquestPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "TileConquestCharacter.h"
#include "Engine/World.h"
#include <MoveCommand.h>
#include <Kismet/GameplayStatics.h>
#include <NavigationSystem.h>

TArray<MoveCommand*> MoveCommands;
MoveCommand* m_CurrentMoveCommand;
int moveCommandIndex = 0;
bool foundFirstTile = false;
bool startedMusic = false;

ATileConquestPlayerController::ATileConquestPlayerController()
{
	MoveCommands.Empty();
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	m_CurrentMoveCommand = new MoveCommand(this);
	foundFirstTile = false;
	startedMusic = false;
}

void ATileConquestPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	if (!startedMusic)
	{
		UGameplayStatics::SpawnSound2D(this, AmbienceSound);
		startedMusic = true;
	}
	if (!foundFirstTile)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile::StaticClass(), FoundActors);
		auto playerLocation = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetActorLocation();

		for(AActor* actor : FoundActors)
		{
			if (ATile* tile = Cast<ATile>(actor))
			{
				// Check if it is close enough to be the starting tile
				if (FVector::Distance(playerLocation, tile->GetActorLocation()) < 151.0f)
				{
					m_CurrentMoveCommand->execute(tile);
					foundFirstTile = true;

					// Rebuild Nav Mesh once the level has loaded
					UNavigationSystemV1* navigation_system = UNavigationSystemV1::GetCurrent(GetWorld());

					if (navigation_system)
					{
						navigation_system->Build();
					}

					CanMove = true;

					break;
				}
			}
		}
	}
}

void ATileConquestPlayerController::CheckIfLevelComplete()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile::StaticClass(), FoundActors);
	for (AActor* actor : FoundActors)
	{
		if (ATile* tile = Cast<ATile>(actor))
		{
			if (!tile->SteppedOn)
			{
				return;
			}
		}
	}

	// If you made it this far, all tiles have been stepped on.
	// Make sure the player cannot input any new movements once they've completed the level
	CanMove = false;
	UGameplayStatics::SpawnSound2D(this, SuccessSound);
	// Load the next level once the player has reached their destination.
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
		{
			FString CurrentLevel = GetWorld()->GetMapName();
			if (CurrentLevel.EndsWith("Level_0"))
			{
				UGameplayStatics::OpenLevel(GetWorld(), "Level_1");
			}
			else if (CurrentLevel.EndsWith("Level_1"))
			{
				UGameplayStatics::OpenLevel(GetWorld(), "Level_6");
			}
			else if (CurrentLevel.EndsWith("Level_6"))
			{
				UGameplayStatics::OpenLevel(GetWorld(), "Level_2");
			}
			else if (CurrentLevel.EndsWith("Level_2"))
			{
				UGameplayStatics::OpenLevel(GetWorld(), "Level_3");
			}
			else if (CurrentLevel.EndsWith("Level_3"))
			{
				UGameplayStatics::OpenLevel(GetWorld(), "Level_4");
			}
			else
			{
				UGameplayStatics::OpenLevel(GetWorld(), "Level_0");
			}
		}, 1.5, false);

	
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
		// UAIBlueprintHelperLibrary::MoveTo
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, HitLocation);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, HitLocation, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);

		// Finally, update the tile to react to the step
		if (activateStep)
		{
			destinationTile->StepOn();
			UGameplayStatics::SpawnSound2D(this, GlassBreakSound);
		}

		// Only add a movement command if it wasn't an undo or redo
		if (!isUndoOrRedo)
		{
			MoveCommands.Add(new MoveCommand(m_CurrentMoveCommand));
			moveCommandIndex = MoveCommands.Num() - 1;
		}

		auto intstring = FString::FromInt(moveCommandIndex);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, intstring);

		CheckIfLevelComplete();
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
	if (CanMove)
	{
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
}

void ATileConquestPlayerController::RedoCallback()
{
	if (CanMove)
	{
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

void ATileConquestPlayerController::OnSetDestinationReleased()
{
	if (CanMove)
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
				m_CurrentMoveCommand->execute(tileActor);
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
