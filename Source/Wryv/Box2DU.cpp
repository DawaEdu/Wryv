#include "Wryv.h"
#include "Box2DU.h"
#include "GlobalFunctions.h"

void FBox2DU::print(FString msg) {
  LOG( "%s fbox2du bounds min=(%f %f) max=(%f %f) meas=(%f %f)",
    *msg, Min.X, Min.Y, Max.X, Max.Y, Size().X, Size().Y );
}

