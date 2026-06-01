#ifndef BUILDING_GRID_H
#define BUILDING_GRID_H

struct Simulation;

/*
 * Dynamic elevators x floors view matrix (row = floor, column = shaft).
 * Synced from simulation state for ASCII display and course requirement (2D matrix).
 */
typedef struct {
    int numFloors;
    int numElevators;
    int undergroundFloors;  /* for row labels: display = index - undergroundFloors */
    unsigned char* cells; /* index: elevator * numFloors + floor */
} BuildingGrid;

int building_grid_init(BuildingGrid* grid, int numFloors, int numElevators,
                       int undergroundFloors);
void building_grid_destroy(BuildingGrid* grid);
void building_grid_sync(BuildingGrid* grid, const struct Simulation* sim);
void building_grid_print(const BuildingGrid* grid);

#endif /* BUILDING_GRID_H */
