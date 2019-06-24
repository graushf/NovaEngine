#include "Math.h"

#include "Common/CommonStd.h"


NvRandom Math::random;

bool Math::PointInPoly(Point const& test, const Poly& polygon)
{
	Point newPoint, oldPoint;
	Point left, right;

	bool inside = false;

	size_t points = polygon.size();

	// The polygon must at least be a triangle
	if (points < 3)
		return false;

	oldPoint = polygon[points - 1];

	for (unsigned int i = 0; i < points; i++)
	{
		newPoint = polygon[i];
		if (newPoint.x > oldPoint.x)
		{
			left = oldPoint;
			right = newPoint;
		}
		else
		{
			left = newPoint;
			right = oldPoint;
		}

		// A point exactly on the left side of the polygon
		// will not intersect - as if it were "open"
		if ((newPoint.x < test.x) == (test.x <= oldPoint.x)
			&& (test.y - left.y) * (right.x - left.x)
			< (right.y - left.y) * (test.x - left.x))
		{
			inside = !inside;
		}

		oldPoint = newPoint;
	}
	return(inside);
}