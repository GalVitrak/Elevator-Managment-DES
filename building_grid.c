/*
 * building_grid.c - Elevators x floors matrix (dynamic) and ASCII shaft view
 * PRESENTATION: 2D dynamic structure — menu 5 prints building_grid after sync.
 */
#include "building_grid.h"
#include "simulation.h"
#include "elevator.h"
#include "floor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int building_grid_init(BuildingGrid* grid, int numFloors, int numElevators,
                       int undergroundFloors)
{
    size_t count;

    if (grid == NULL || numFloors <= 0 || numElevators <= 0 || undergroundFloors < 0) {
        return 0;
    }

    grid->numFloors = numFloors;
    grid->numElevators = numElevators;
    grid->undergroundFloors = undergroundFloors;
    count = (size_t)numElevators * (size_t)numFloors;
    grid->cells = (unsigned char*)calloc(count, sizeof(unsigned char));
    return grid->cells != NULL;
}

void building_grid_destroy(BuildingGrid* grid)
{
    if (grid == NULL) {
        return;
    }
    free(grid->cells);
    grid->cells = NULL;
    grid->numFloors = 0;
    grid->numElevators = 0;
}

static unsigned char* building_grid_cell(BuildingGrid* grid, int elevator, int floor)
{
    return &grid->cells[(size_t)elevator * (size_t)grid->numFloors + (size_t)floor];
}

void building_grid_sync(BuildingGrid* grid, const Simulation* sim)
{
    int elev;
    int floor;

    if (grid == NULL || sim == NULL || grid->cells == NULL) {
        return;
    }

    if (grid->numFloors != sim->numFloors || grid->numElevators != sim->numElevators) {
        return;
    }

    memset(grid->cells, (int)' ', (size_t)grid->numFloors * (size_t)grid->numElevators);

    for (elev = 0; elev < sim->numElevators; elev++) {
        const Elevator* cab = &sim->elevators[elev];
        int pos = cab->currentFloor;

        if (pos >= 0 && pos < sim->numFloors) {
            *building_grid_cell((BuildingGrid*)grid, elev, pos) = (unsigned char)('0' + elev);
        }

        for (floor = 0; floor < sim->numFloors; floor++) {
            if (floor == pos) {
                continue;
            }
            if (elevator_has_stop(cab, floor)) {
                unsigned char* cell = building_grid_cell((BuildingGrid*)grid, elev, floor);
                if (*cell == (unsigned char)' ') {
                    *cell = (unsigned char)'*';
                }
            }
        }
    }

    for (floor = 0; floor < sim->numFloors; floor++) {
        const Floor* f = &sim->floors[floor];
        int col;
        if (f->upButtonPressed) {
            for (col = 0; col < sim->numElevators; col++) {
                unsigned char* cell = building_grid_cell((BuildingGrid*)grid, col, floor);
                if (*cell == (unsigned char)' ') {
                    *cell = (unsigned char)'^';
                }
            }
        }
        if (f->downButtonPressed) {
            for (col = 0; col < sim->numElevators; col++) {
                unsigned char* cell = building_grid_cell((BuildingGrid*)grid, col, floor);
                if (*cell == (unsigned char)' ') {
                    *cell = (unsigned char)'v';
                }
            }
        }
    }
}

void building_grid_print(const BuildingGrid* grid)
{
    int floor;
    int elev;

    if (grid == NULL || grid->cells == NULL) {
        return;
    }

    printf("\n  Building grid (rows=floors high->low, cols=elevators)\n");
    printf("  Legend: 0-9=cab  *=scheduled stop  ^=up call  v=down call\n     ");
    for (elev = 0; elev < grid->numElevators; elev++) {
        printf(" [%d]", elev);
    }
    printf("\n");

    for (floor = grid->numFloors - 1; floor >= 0; floor--) {
        int displayFloor = floor - grid->undergroundFloors;
        printf("  %3d |", displayFloor);
        for (elev = 0; elev < grid->numElevators; elev++) {
            size_t idx = (size_t)elev * (size_t)grid->numFloors + (size_t)floor;
            unsigned char ch = grid->cells[idx];
            if (ch == (unsigned char)' ') {
                printf("  . ");
            } else {
                printf("  %c ", (char)ch);
            }
        }
        printf("|\n");
    }
    printf("     ");
    for (elev = 0; elev < grid->numElevators; elev++) {
        printf("----");
    }
    printf("\n");
}
