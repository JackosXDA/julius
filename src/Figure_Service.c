#include "Figure.h"

#include "Building.h"
#include "FigureAction.h"

#include "Data/Building.h"
#include "Data/Constants.h"
#include "Data/Settings.h"
#include "Data/Grid.h"
#include "Data/Figure.h"

#include "building/model.h"

#define FOR_XY_RADIUS \
	int xMin = x - 2;\
	int yMin = y - 2;\
	int xMax = x + 2;\
	int yMax = y + 2;\
	Bound2ToMap(xMin, yMin, xMax, yMax);\
	int gridOffset = GridOffset(xMin, yMin);\
	for (int yy = yMin; yy <= yMax; yy++) {\
		for (int xx = xMin; xx <= xMax; xx++) {\
			int buildingId = Data_Grid_buildingIds[gridOffset];\
			if (buildingId) {

#define END_FOR_XY_RADIUS \
			}\
			++gridOffset;\
		}\
		gridOffset += 162 - (xMax - xMin + 1);\
	}

static int provideEngineerCoverage(int x, int y, int *maxDamageRiskSeen)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].type == BUILDING_HIPPODROME) {
			buildingId = Building_getMainBuildingId(buildingId);
		}
		if (Data_Buildings[buildingId].damageRisk > *maxDamageRiskSeen) {
			*maxDamageRiskSeen = Data_Buildings[buildingId].damageRisk;
		}
		Data_Buildings[buildingId].damageRisk = 0;
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int providePrefectFireCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].type == BUILDING_HIPPODROME) {
			buildingId = Building_getMainBuildingId(buildingId);
		}
		Data_Buildings[buildingId].fireRisk = 0;
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int getPrefectCrimeCoverage(int x, int y)
{
	int minHappinessSeen = 100;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].sentiment.houseHappiness < minHappinessSeen) {
			minHappinessSeen = Data_Buildings[buildingId].sentiment.houseHappiness;
		}
	} END_FOR_XY_RADIUS;
	return minHappinessSeen;
}

static int provideTheaterCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.theater = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideAmphitheaterCoverage(int x, int y, int numShows)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.amphitheaterActor = 96;
			if (numShows == 2) {
				Data_Buildings[buildingId].data.house.amphitheaterGladiator = 96;
			}
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideColosseumCoverage(int x, int y, int numShows)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.colosseumGladiator = 96;
			if (numShows == 2) {
				Data_Buildings[buildingId].data.house.colosseumLion = 96;
			}
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideHippodromeCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.hippodrome = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideMarketGoods(int marketBuildingId, int x, int y)
{
	int serviced = 0;
	struct Data_Building *market = &Data_Buildings[marketBuildingId];
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			serviced++;
			struct Data_Building *house = &Data_Buildings[buildingId];
			int level = house->subtype.houseLevel;
			if (level < HOUSE_LUXURY_PALACE) {
				level++;
			}
			int maxFoodStocks = 4 * house->houseMaxPopulationSeen;
			int foodTypesStoredMax = 0;
			for (int i = Inventory_MinFood; i < Inventory_MaxFood; i++) {
				if (house->data.house.inventory[i] >= maxFoodStocks) {
					foodTypesStoredMax++;
				}
			}
			const model_house *model = model_get_house(level);
			if (model->food_types > foodTypesStoredMax) {
				for (int i = Inventory_MinFood; i < Inventory_MaxFood; i++) {
					if (house->data.house.inventory[i] >= maxFoodStocks) {
						continue;
					}
					if (market->data.market.inventory[i] >= maxFoodStocks) {
						house->data.house.inventory[i] += maxFoodStocks;
						market->data.market.inventory[i] -= maxFoodStocks;
						break;
					} else if (market->data.market.inventory[i]) {
						house->data.house.inventory[i] += market->data.market.inventory[i];
						market->data.market.inventory[i] = 0;
						break;
					}
				}
			}
			if (model->pottery) {
				market->data.market.potteryDemand = 10;
				int potteryWanted = 8 * model->pottery - house->data.house.inventory[Inventory_Pottery];
				if (market->data.market.inventory[Inventory_Pottery] > 0 && potteryWanted > 0) {
					if (potteryWanted <= market->data.market.inventory[Inventory_Pottery]) {
						house->data.house.inventory[Inventory_Pottery] += potteryWanted;
						market->data.market.inventory[Inventory_Pottery] -= potteryWanted;
					} else {
						house->data.house.inventory[Inventory_Pottery] += market->data.market.inventory[Inventory_Pottery];
						market->data.market.inventory[Inventory_Pottery] = 0;
					}
				}
			}
			if (model->furniture) {
				market->data.market.furnitureDemand = 10;
				int furnitureWanted = 4 * model->furniture - house->data.house.inventory[Inventory_Furniture];
				if (market->data.market.inventory[Inventory_Furniture] > 0 && furnitureWanted > 0) {
					if (furnitureWanted <= market->data.market.inventory[Inventory_Furniture]) {
						house->data.house.inventory[Inventory_Furniture] += furnitureWanted;
						market->data.market.inventory[Inventory_Furniture] -= furnitureWanted;
					} else {
						house->data.house.inventory[Inventory_Furniture] += market->data.market.inventory[Inventory_Furniture];
						market->data.market.inventory[Inventory_Furniture] = 0;
					}
				}
			}
			if (model->oil) {
				market->data.market.oilDemand = 10;
				int oilWanted = 4 * model->oil - house->data.house.inventory[Inventory_Oil];
				if (market->data.market.inventory[Inventory_Oil] > 0 && oilWanted > 0) {
					if (oilWanted <= market->data.market.inventory[Inventory_Oil]) {
						house->data.house.inventory[Inventory_Oil] += oilWanted;
						market->data.market.inventory[Inventory_Oil] -= oilWanted;
					} else {
						house->data.house.inventory[Inventory_Oil] += market->data.market.inventory[Inventory_Oil];
						market->data.market.inventory[Inventory_Oil] = 0;
					}
				}
			}
			if (model->wine) {
				market->data.market.wineDemand = 10;
				int wineWanted = 4 * model->wine - house->data.house.inventory[Inventory_Wine];
				if (market->data.market.inventory[Inventory_Wine] > 0 && wineWanted > 0) {
					if (wineWanted <= market->data.market.inventory[Inventory_Wine]) {
						house->data.house.inventory[Inventory_Wine] += wineWanted;
						market->data.market.inventory[Inventory_Wine] -= wineWanted;
					} else {
						house->data.house.inventory[Inventory_Wine] += market->data.market.inventory[Inventory_Wine];
						market->data.market.inventory[Inventory_Wine] = 0;
					}
				}
			}
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideBathhouseCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.bathhouse = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideReligionCoverage(int x, int y, int god)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			switch (god) {
				case God_Ceres:
					Data_Buildings[buildingId].data.house.templeCeres = 96;
					break;
				case God_Neptune:
					Data_Buildings[buildingId].data.house.templeNeptune = 96;
					break;
				case God_Mercury:
					Data_Buildings[buildingId].data.house.templeMercury = 96;
					break;
				case God_Mars:
					Data_Buildings[buildingId].data.house.templeMars = 96;
					break;
				case God_Venus:
					Data_Buildings[buildingId].data.house.templeVenus = 96;
					break;
			}
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideSchoolCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.school = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideAcademyCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.academy = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideLibraryCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.library = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideBarberCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.barber = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideClinicCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.clinic = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideHospitalCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.hospital = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideMissionaryCoverage(int x, int y)
{
	int xMin = x - 4;
	int yMin = y - 4;
	int xMax = x + 4;
	int yMax = y + 4;
	Bound2ToMap(xMin, yMin, xMax, yMax);
	int gridOffset = GridOffset(xMin, yMin);
	for (int yy = yMin; yy <= yMax; yy++) {
		for (int xx = xMin; xx <= xMax; xx++) {
			int buildingId = Data_Grid_buildingIds[gridOffset];
			if (buildingId) {
				if (Data_Buildings[buildingId].type == BUILDING_NATIVE_HUT ||
					Data_Buildings[buildingId].type == BUILDING_NATIVE_MEETING) {
					Data_Buildings[buildingId].sentiment.nativeAnger = 0;
				}
			}
			++gridOffset;
		}
		gridOffset += 162 - (xMax - xMin + 1);
	}
	return 1;
}

static int provideLaborSeekerCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideTaxCollectorCoverage(int x, int y, unsigned char *maxTaxMultiplier)
{
	int serviced = 0;
	*maxTaxMultiplier = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			int taxMultiplier = model_get_house(Data_Buildings[buildingId].subtype.houseLevel)->tax_multiplier;
			if (taxMultiplier > *maxTaxMultiplier) {
				*maxTaxMultiplier = taxMultiplier;
			}
			Data_Buildings[buildingId].houseTaxCoverage = 50;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

int Figure_provideServiceCoverage(int figureId)
{
	int numHousesServiced = 0;
	int x = Data_Figures[figureId].x;
	int y = Data_Figures[figureId].y;
	int buildingId;
	switch (Data_Figures[figureId].type) {
		case Figure_Patrician:
			return 0;
		case Figure_LaborSeeker:
			numHousesServiced = provideLaborSeekerCoverage(x, y);
			break;
		case Figure_TaxCollector:
			numHousesServiced = provideTaxCollectorCoverage(x, y, &Data_Figures[figureId].minMaxSeen);
			break;
		case Figure_MarketTrader:
		case Figure_MarketBuyer:
			numHousesServiced = provideMarketGoods(Data_Figures[figureId].buildingId, x, y);
			break;
		case Figure_BathhouseWorker:
			numHousesServiced = provideBathhouseCoverage(x, y);
			break;
		case Figure_SchoolChild:
			numHousesServiced = provideSchoolCoverage(x, y);
			break;
		case Figure_Teacher:
			numHousesServiced = provideAcademyCoverage(x, y);
			break;
		case Figure_Librarian:
			numHousesServiced = provideLibraryCoverage(x, y);
			break;
		case Figure_Barber:
			numHousesServiced = provideBarberCoverage(x, y);
			break;
		case Figure_Doctor:
			numHousesServiced = provideClinicCoverage(x, y);
			break;
		case Figure_Surgeon:
			numHousesServiced = provideHospitalCoverage(x, y);
			break;
		case Figure_Missionary:
			numHousesServiced = provideMissionaryCoverage(x, y);
			break;
		case Figure_Priest:
			switch (Data_Buildings[Data_Figures[figureId].buildingId].type) {
				case BUILDING_SMALL_TEMPLE_CERES:
				case BUILDING_LARGE_TEMPLE_CERES:
					numHousesServiced = provideReligionCoverage(x, y, God_Ceres);
					break;
				case BUILDING_SMALL_TEMPLE_NEPTUNE:
				case BUILDING_LARGE_TEMPLE_NEPTUNE:
					numHousesServiced = provideReligionCoverage(x, y, God_Neptune);
					break;
				case BUILDING_SMALL_TEMPLE_MERCURY:
				case BUILDING_LARGE_TEMPLE_MERCURY:
					numHousesServiced = provideReligionCoverage(x, y, God_Mercury);
					break;
				case BUILDING_SMALL_TEMPLE_MARS:
				case BUILDING_LARGE_TEMPLE_MARS:
					numHousesServiced = provideReligionCoverage(x, y, God_Mars);
					break;
				case BUILDING_SMALL_TEMPLE_VENUS:
				case BUILDING_LARGE_TEMPLE_VENUS:
					numHousesServiced = provideReligionCoverage(x, y, God_Venus);
					break;
				default:
					break;
			}
			break;
		case Figure_Actor:
			if (Data_Figures[figureId].actionState == FigureActionState_94_EntertainerRoaming ||
				Data_Figures[figureId].actionState == FigureActionState_95_EntertainerReturning) {
				buildingId = Data_Figures[figureId].buildingId;
			} else { // going to venue
				buildingId = Data_Figures[figureId].destinationBuildingId;
			}
			if (Data_Buildings[buildingId].type == BUILDING_THEATER) {
				numHousesServiced = provideTheaterCoverage(x, y);
			} else if (Data_Buildings[buildingId].type == BUILDING_AMPHITHEATER) {
				numHousesServiced = provideAmphitheaterCoverage(x, y,
					Data_Buildings[buildingId].data.entertainment.days1 ? 2 : 1);
			}
			break;
		case Figure_Gladiator:
			if (Data_Figures[figureId].actionState == FigureActionState_94_EntertainerRoaming ||
				Data_Figures[figureId].actionState == FigureActionState_95_EntertainerReturning) {
				buildingId = Data_Figures[figureId].buildingId;
			} else { // going to venue
				buildingId = Data_Figures[figureId].destinationBuildingId;
			}
			if (Data_Buildings[buildingId].type == BUILDING_AMPHITHEATER) {
				numHousesServiced = provideAmphitheaterCoverage(x, y,
					Data_Buildings[buildingId].data.entertainment.days2 ? 2 : 1);
			} else if (Data_Buildings[buildingId].type == BUILDING_COLOSSEUM) {
				numHousesServiced = provideColosseumCoverage(x, y,
					Data_Buildings[buildingId].data.entertainment.days1 ? 2 : 1);
			}
			break;
		case Figure_LionTamer:
			if (Data_Figures[figureId].actionState == FigureActionState_94_EntertainerRoaming ||
				Data_Figures[figureId].actionState == FigureActionState_95_EntertainerReturning) {
				buildingId = Data_Figures[figureId].buildingId;
			} else { // going to venue
				buildingId = Data_Figures[figureId].destinationBuildingId;
			}
			numHousesServiced = provideColosseumCoverage(x, y,
				Data_Buildings[buildingId].data.entertainment.days2 ? 2 : 1);
			break;
		case Figure_Charioteer:
			numHousesServiced = provideHippodromeCoverage(x, y);
			break;
		case Figure_Engineer:
			{int maxDamage = 0;
			numHousesServiced = provideEngineerCoverage(x, y, &maxDamage);
			if (maxDamage > Data_Figures[figureId].minMaxSeen) {
				Data_Figures[figureId].minMaxSeen = maxDamage;
			} else if (Data_Figures[figureId].minMaxSeen <= 10) {
				Data_Figures[figureId].minMaxSeen = 0;
			} else {
				Data_Figures[figureId].minMaxSeen -= 10;
			}}
			break;
		case Figure_Prefect:
			numHousesServiced = providePrefectFireCoverage(x, y);
			Data_Figures[figureId].minMaxSeen = getPrefectCrimeCoverage(x, y);
			break;
		case Figure_Rioter:
			if (FigureAction_Rioter_collapseBuilding(figureId) == 1) {
				return 1;
			}
			break;
	}
	if (Data_Figures[figureId].buildingId) {
		buildingId = Data_Figures[figureId].buildingId;
		Data_Buildings[buildingId].housesCovered += numHousesServiced;
		if (Data_Buildings[buildingId].housesCovered > 300) {
			Data_Buildings[buildingId].housesCovered = 300;
		}
	}
	return 0;
}