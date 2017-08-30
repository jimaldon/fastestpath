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
         
    vertex_descriptor roverPos = vertex((ROVER_X+ROVER_Y*IMAGE_DIM), m.m_grid);
    vertex_descriptor humanPos = vertex((BACHELOR_X+BACHELOR_Y*IMAGE_DIM), m.m_grid);
    vertex_descriptor weddingPos = vertex((WEDDING_X+WEDDING_Y*IMAGE_DIM), m.m_grid);

    if (m.solve(roverPos, humanPos))
        std::cout << "Rover has reached the bachelor!" << std::endl;
    else
        std::cout << "Rover cacn't reach the bachelor." << std::endl;

    std::set<std::pair<size_t, size_t>> path1;
    
    double pathTime1 = 0;
    for(auto elem = m.m_solution.begin(); elem != m.m_solution.end(); ++elem)
    {
        
        if(std::next(elem,1) != m.m_solution.end())
            pathTime1 += m.timeWeight(*elem, *std::next(elem,1), elevation);

        path1.insert(std::make_pair(elem->at(0), elem->at(1)));
    }

    std::cout << "Time taken by rover to reach the bachelor is " << pathTime1 << " island seconds."<<std::endl;
    std::cout<<std::endl;
    m.m_solution.clear();


    //Wedding party -> Bachelor to Wedding
    if (m.solve(humanPos, weddingPos))
        std::cout << "The bachelor has reached his wedding!" << std::endl;
    else
        std::cout << "Looks like the bachelor stays a bachelor for a while longer." << std::endl;

    std::set<std::pair<size_t, size_t>> path2;

    double pathTime2 = 0;
    for(auto elem = m.m_solution.begin(); elem != m.m_solution.end(); ++elem)
    {

        if(std::next(elem,1) != m.m_solution.end())
            pathTime2 += m.timeWeight(*elem, *std::next(elem,1), elevation);

        path2.insert(std::make_pair(elem->at(0), elem->at(1)));
    }

    std::cout << "Time taken by the rover to reach the wedding from the bachelor's position is " << pathTime2 << " island seconds."<<std::endl;
    std::cout<<std::endl;

    double totalTime = pathTime1+pathTime2;
    std::cout << "Total time taken = " << totalTime << "island seconds. "<<std::endl;
    int minutes = totalTime/60;
    int hours = minutes/60;
    std::cout << "Or in island time, the entire trip took " << int(hours%60) << " hours " << int(minutes%60) 
    << " minutes " << int(totalTime)%60 << " seconds."<<std::endl;
    std::cout<<std::endl;

    std::cout << "You can open the map with $fed pic.bmp or $edisplay pic.bmp on Linux systems." <<std::endl;


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
                donut(x, y, WEDDING_X, WEDDING_Y) || path1.find(std::make_pair(x,y)) != path1.end()|| path2.find(std::make_pair(x,y)) != path2.end())
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
