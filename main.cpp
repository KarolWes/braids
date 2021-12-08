#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <ctime>
#include <cstdlib>

using namespace std;


void print(vector < vector < pair <int, int> > > *braid){
    for(auto level:*braid){
        for(auto el: level){
            cout << el.first << "(" << el.second << ")\t";
        }
        cout << "\n";
    }
}

int main() {
    srand(time(NULL));
    cout << "Welcome to braid generator" << endl;
    int n = 3;
    int h = 10;
    auto *braid = new vector < vector < pair <int, int> > >;
    auto *level= new vector < pair <int, int> >;
    for(int i = 0; i < n; i++){
        level->push_back(make_pair(i, 0));
    }
    braid->push_back(*level);
    level->clear();
    for(int i = 1; i < h; i++){
        copy(braid->at(i-1).begin(),  braid->at(i-1).end(), back_inserter(*level));
        for(int j = 0; j < n-1; j++){
            int a = rand()%100;
            if(a < 50){
                swap(level->at(j), level->at(j+1));
                if(a < 25){
                    level->at(j).second = 1;
                    level->at(j+1).second = -1;
                }
                else{
                    level->at(j).second = -1;
                    level->at(j+1).second = 1;
                }
                j+=1;
            }
            else{
                level->at(j).second = 0;
            }
        }
        braid->push_back(*level);
        level->clear();
    }
    print(braid);

    return 0;
}
