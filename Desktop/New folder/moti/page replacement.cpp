//cs21b1021
//Pratham Jain
//FIFO
 
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>
#include <algorithm>

using namespace std;

int NumPhysical, PageSize ,AddressCount;

int AddressCounttest = 271829; //testing
int PageSizetest = 5143; //testing
int NumPhysicaltest = 12; //testing


extern vector<int> read_sequence;
const int BLOCK_MAX_SIZE = 10u;
const int TLB_SIZE = 4u;
int PageFaultCount = 0;
int NumReplaced = 0;
vector<int> fastPageTable;
vector<int> pageTable;
int blockCount = BLOCK_MAX_SIZE;

void fifo()
{
	
	for (vector<int>::const_iterator it = read_sequence.begin();
		 it not_eq read_sequence.end();++it)
	{
		vector<int>::iterator fastPos = find(fastPageTable.begin(), fastPageTable.end(), *it);
		if (fastPos == fastPageTable.end())
		{
			
			vector<int>::iterator pagePos = find(pageTable.begin(), pageTable.end(), *it);
			if (pagePos == pageTable.end())
			{
				
				
				if (blockCount > 0)
				{
					
					--blockCount;
					pageTable.push_back(*it);
				}
				
				else
				{
					pageTable.erase(pageTable.begin());
					pageTable.push_back(*it);
					++NumReplaced;
				}
				
				++PageFaultCount;
			} else
			{
				
				pageTable.erase(pagePos);
				pageTable.push_back(*it);
			}
		
			if (fastPageTable.size() < TLB_SIZE)
			{
				fastPageTable.push_back(*it);
			}
			else
			{
				fastPageTable.erase(fastPageTable.begin());
				fastPageTable.push_back(*it);
			}
		}
		else
		{
				fastPageTable.erase(fastPos);
				fastPageTable.push_back(*it);
		}
	}
}



int main()
{
    cout << "Running :) " << AddressCounttest << endl;
    
    ifstream input_file("inp.txt");
    input_file >> NumPhysical >> PageSize >> AddressCount;//input

    cout << NumPhysical<< PageSize << AddressCount << endl;//testing
    
   
}
