#include "visualizer.h"

#include <iostream>
#include <fstream>
#include <assert.h>
#include <algorithm>
#include <stdio.h>
#include <utility>
#include <vector>

#include "utility.hpp"



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
    
    maze m = make_maze(IMAGE_DIM, IMAGE_DIM, overrides, elevation);
    
    std::cout << "here! after make maze" << std::endl;
     
    vertex_descriptor roverPos = vertex((ROVER_X+ROVER_Y*IMAGE_DIM), m.m_grid);
    vertex_descriptor humanPos = vertex((BACHELOR_X+BACHELOR_Y*IMAGE_DIM), m.m_grid);

    std::cout << "here! before solve call" << std::endl;

    if (m.solve(roverPos, humanPos))
        std::cout << "Solved the maze." << std::endl;
    else
        std::cout << "The maze is not solvable." << std::endl;

    std::set<std::pair<size_t, size_t>> path1;
    //path1.reserve(m.m_solution_length);
    
    for(const auto& elem: m.m_solution)
    {
        
        //size_t index = get(boost::vertex_index, m.m_grid, elem);
        //size_t xCorr = index%IMAGE_DIM;
        //size_t yCorr = (index-xCorr)/IMAGE_DIM;
        //std::cout<<"elem[0], elem[1]"<<elem[0]<<", "<<elem[1]<<std::endl;
        //std::cout<<"xCorr, yCorr"<<xCorr<<", "<<yCorr<<std::endl;
        path1.insert(std::make_pair(elem[0], elem[1]));
    }

    std::cout << "here! after path1 creation" << std::endl;
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
            if (donut(x, y, ROVER_X, ROVER_Y) ||
                donut(x, y, BACHELOR_X, BACHELOR_Y) ||
                donut(x, y, WEDDING_X, WEDDING_Y) || path1.find(std::make_pair(x,y)) != path1.end())
            {
                return uint8_t(visualizer::IPV_PATH);
            }
            
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
    //system("edisplay pic.bmp");
    return 0;
}
