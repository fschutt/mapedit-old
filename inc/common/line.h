#pragma once

struct Line
{
	virtual bool isBezierCurve() = 0;
	Line();
	virtual ~Line();
};
