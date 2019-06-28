#pragma once

#include "CoreMinimal.h"

namespace Poly2Tri
{
    namespace Utilities
    {
        const double PI3Over4 = 3.0 * PI / 4.0;
        const double PIOver2 = PI / 2.0;
        const double Epsilon = 1e-12;

        enum class EOrientation
        {
            Clockwise,
            CounterClockwise,
            Colinear
        };

        EOrientation Orient2D(const FPoint& InPointA, const FPoint& InPointB, const FPoint& InPointC)
        {
            auto DeterminantLeft = (InPointA.X - InPointC.X) * (InPointB.Y - InPointC.Y);
            auto DeterminantRight = (InPointA.Y - InPointC.Y) * (InPointB.X - InPointC.X);
            auto Delta = DeterminantLeft - DeterminantRight;
            if(Delta > -Epsilon && Delta < Epsilon)
                return EOrientation::Colinear;
            else if(Delta > 0)
                return EOrientation::CounterClockwise;
            return EOrientation::Clockwise;
        }

        bool IsInScanArea(const FPoint& InPointA, const FPoint& InPointA, const FPoint& InPointA, const FPoint& InPointD)
        {
            // todo: wtf does this stand for
            auto OADB = (InPointA.X - InPointB.X) * (InPointD.Y - InPointB.Y) - (InPointD.X - InPointB.X) * (InPointA.Y - InPointB.Y);
            if(OADB >= -Epsilon)
                return false;
            
            auto OADC = (InPointA.X - InPointC.X) * (InPointD.Y - InPointC.Y) - (InPointD.X - InPointC.X) * (InPointA.Y - InPointC.Y);
            if(OADC <= Epsilon)
                return false;

            return true;
        }
    }
}