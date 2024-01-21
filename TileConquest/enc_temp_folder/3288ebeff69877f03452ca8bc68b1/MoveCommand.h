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
    MoveCommand(MoveCommand* moveCommand)
    {
        PlayerUnit = moveCommand->PlayerUnit;
        CurrentTile = moveCommand->CurrentTile;
        PreviousTile = moveCommand->PreviousTile;
    }
    ~MoveCommand()
    {
        delete this;
    }

    virtual void execute(ATile* destinationTile)
    {
        // Remember the unit's position before the move
        // so we can restore it.
        PreviousTile = PlayerUnit->GetCurrentTile();
        CurrentTile = destinationTile;

        // If the tile is stepped on, don't move to it
        if (destinationTile->SteppedOn)
        {
            return;
        }

        if (PreviousTile == nullptr || CurrentTile == nullptr)
        {
            PlayerUnit->MoveTo(CurrentTile, true, false);
            return;
        }

        // If the two tiles are at the same location, don't call the Movement action
        if (FVector::Distance(PreviousTile->GetActorLocation(), CurrentTile->GetActorLocation()) > 20.0f)
        {
            bool moveSuccessful = PlayerUnit->MoveTo(CurrentTile, true, false);

            if (moveSuccessful)
            {
                PreviousTile->TileMesh->SetVisibility(false);
            }

        }
    }

    virtual void undo()
    {
        // If the player is trying to undo to the tile they are currently on, something has gone wrong and we should ignore the undo
        if (PreviousTile != nullptr)
        {
            if (FVector::Distance(PreviousTile->GetActorLocation(), PlayerUnit->GetCurrentTile()->GetActorLocation()) > 20.0f)
            {
                bool moveSuccessful = PlayerUnit->MoveTo(PreviousTile, false, true);
                if (moveSuccessful)
                {
                    PreviousTile->TileMesh->SetVisibility(true);
                    CurrentTile->StepOn();
                    PlayerUnit->DecreaseMoveCommandCount();
                }
            }
        }
    }

    virtual void redo()
    {
        bool moveSuccessful = PlayerUnit->MoveTo(CurrentTile, true, true);
        if (moveSuccessful)
        {
            if (PreviousTile != nullptr)
            {
                PreviousTile->TileMesh->SetVisibility(false);
            }
        }
    }

    ATileConquestPlayerController* PlayerUnit;
    ATile* PreviousTile;
    ATile* CurrentTile;
};
