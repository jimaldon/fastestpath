#include "visualizer.h"

#include <iostream>
#include <fstream>
#include <assert.h>
#include <algorithm>
#include <stdio.h>
#include <utility>
#include <vector>
#include "astar.h"

// Bits used in the overrides image bytes
enum OverrideFlags
{
    OF_RIVER_MARSH = 0x10,
    OF_INLAND = 0x20,
    OF_WATER_BASIN = 0x40
};

// Some constants
enum {
    IMAGE_DIM = 2048, // Width and height of the elevation and overrides image
    
    ROVER_X = 159,
    ROVER_Y = 1520,
    BACHELOR_X = 1303,
    BACHELOR_Y = 85,
    WEDDING_X = 1577,
    WEDDING_Y = 1294
};

std::ifstream::pos_type fileSize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    if (!in.good())
    {
        throw std::exception();
    }
    return in.tellg(); 
}

std::vector<uint8_t> loadFile(const char* fileName, size_t expectedFileSize)
{
    size_t fsize = fileSize(fileName);
    if (fsize != expectedFileSize)
    {
        throw std::exception();
    }
    std::vector<uint8_t> data(fsize);
    std::ifstream ifile(fileName, std::ifstream::binary);
    if (!ifile.good())
    {
        throw std::exception();
    }
    ifile.read((char*)&data[0], fsize);
    return data;
}

bool donut(int x, int y, int x1, int y1)
{
    int dx = x - x1;
    int dy = y - y1;
    int r2 = dx * dx + dy * dy;
    return r2 >= 150 && r2 <= 400;
}

int main(int argc, char** argv)
{
    printf("%s\n", argv[0]);
    
    const size_t expectedFileSize = IMAGE_DIM * IMAGE_DIM;
    auto elevation = loadFile("assets/elevation.data", expectedFileSize);
    auto overrides = loadFile("assets/overrides.data", expectedFileSize);
    std::ofstream of("pic.bmp");
    

    AStar::Generator gen;
    gen.setWorldSize({2048,2048});
    gen.setHeuristic(AStar::Heuristic::euclidean);
    gen.setDiagonalMovement(true);
    
    //add water+river marsh
    auto Obegin = overrides.begin();
    for(auto i = overrides.begin(); i != overrides.end(); ++i)
    {
        AStar::Vec2i point;
        if(*i & (OF_WATER_BASIN | OF_RIVER_MARSH))
        {
            point.y = (i - Obegin)%IMAGE_DIM;
            point.x = (i - Obegin) - point.y*IMAGE_DIM;
            gen.addCollision(point);
        }

    }
    std::cout << "here! 0" << std::endl;
    auto prelimpath1 = gen.findPath({ROVER_X, ROVER_Y},{BACHELOR_X, BACHELOR_Y});
    auto prelimpath2 = gen.findPath({BACHELOR_X, BACHELOR_Y},{WEDDING_X, WEDDING_Y});

    std::vector<std::pair<size_t, size_t>> path1;
    path1.reserve(prelimpath1.size());

    for(auto& coord1 : prelimpath1)
    {
        path1.push_back(std::make_pair(coord1.x, coord1.y));
    }
    std::cout << "here! 1" << std::endl;
    visualizer::writeBMP(
        of,
        &elevation[0],
        IMAGE_DIM,
        IMAGE_DIM,
        [&] (size_t x, size_t y, uint8_t elevation) {
            
            //Path
            //if(std::find(path1.begin(), path1.end(), std::make_pair(x,y)) != path1.end())
            //{
            //    return uint8_t(visualizer::IPV_PATH);
            //}
            // Marks interesting positions on the map
            std::cout << "here! 2" << std::endl;
            if (donut(x, y, ROVER_X, ROVER_Y) ||
                donut(x, y, BACHELOR_X, BACHELOR_Y) ||
                donut(x, y, WEDDING_X, WEDDING_Y) || std::find(path1.begin(), path1.end(), std::make_pair(x,y)) != path1.end())
            {
                return uint8_t(visualizer::IPV_PATH);
            }
            std::cout << "here! 3" << std::endl;
            // Signifies water
            if ((overrides[y * IMAGE_DIM + x] & (OF_WATER_BASIN | OF_RIVER_MARSH)) ||
                elevation == 0)
            {
                return uint8_t(visualizer::IPV_WATER);
            }
            
            // Signifies normal ground color
            if (elevation < visualizer::IPV_ELEVATION_BEGIN)
            {
                elevation = visualizer::IPV_ELEVATION_BEGIN;
            }
            return elevation;
    });
    system("edisplay pic.bmp");
    return 0;
}
