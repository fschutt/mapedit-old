#pragma once
#include <vector>

struct Renderer;
struct Polygon;

struct MultiPolygon{
    MultiPolygon();
    ~MultiPolygon();

    enum class RELATION {
        OUTER,
        INNER
    };

    Renderer* renderer;
    std::vector<std::pair<Polygon*, RELATION>> polygons;
};
