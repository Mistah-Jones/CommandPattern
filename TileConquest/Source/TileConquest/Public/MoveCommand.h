// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Command.h"
#include <TileConquest/TileConquestPlayerController.h>
#include <TileConquest/Public/Tile.h>

/**
 * 
 */
class TILECONQUEST_API MoveCommand // : public Command
{
    /*
    MoveCommand();
    ~MoveCommand();

private:
    ATileConquestPlayerController* PlayerUnit;
    ATile* PreviousTile;
    ATile* CurrentTile;
    */
public:
    MoveCommand(ATileConquestPlayerController* unit)
    {
        PlayerUnit = unit;
        CurrentTile = unit->GetCurrentTile();
    }
    // MoveCommand();
	~MoveCommand();

    virtual void execute(ATile* destinationTile)
    {
        // Remember the unit's position before the move
        // so we can restore it.
        PreviousTile = PlayerUnit->GetCurrentTile();
        CurrentTile = destinationTile;

        if (PreviousTile == nullptr || CurrentTile == nullptr)
        {
            PlayerUnit->MoveTo(CurrentTile, true);
            return;
        }

        // If the two tiles are at the same location, don't call the Movement action
        if (FVector::Distance(PreviousTile->GetActorLocation(), CurrentTile->GetActorLocation()) > 20.0f)
        {
            PlayerUnit->MoveTo(CurrentTile, true);
        }
    }

    virtual void undo()
    {
        PlayerUnit->MoveTo(PreviousTile, false);
        CurrentTile->StepOn();
    }

    virtual void redo()
    {
        PlayerUnit->MoveTo(CurrentTile, true);
    }

    ATileConquestPlayerController* PlayerUnit;
    ATile* PreviousTile;
    ATile* CurrentTile;
};
