#include "Wryv.h"
#include "Util/Box2DU.h"
#include "Util/GlobalFunctions.h"

void FBox2DU::print(FString msg) const {
  LOG( "%s fbox2du bounds min=(%f %f) max=(%f %f) meas=(%f %f)",
    *msg, Min.X, Min.Y, Max.X, Max.Y, Size().X, Size().Y );
}

