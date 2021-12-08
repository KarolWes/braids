#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <string>

using namespace std;


void print(vector < vector < pair <int, int> > > *braid){
    for(auto level:*braid){
        for(auto el: level){
            cout << el.first << "(" << el.second << ")\t";
        }
        cout << "\n";
    }
}

vector < vector < pair <int, int> > > *read_data(string path)
{
    fstream f;
    f.open(path, ios::in);
    int n, h;
    f >> n >> h;
    auto *braid = new vector < vector < pair <int, int> > >;
    auto *level= new vector < pair <int, int> >;
    for(int i = 0; i < h; i++){
        for(int j = 0; j < n; j++){
            int pos, top;
            f >> pos >> top;
            level->push_back(make_pair(pos, top));
        }
        braid->push_back(*level);
        level->clear();
    }

    f.close();
    return braid;
}

vector < vector < pair <int, int> > > *generate(int n, int h){
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
    return braid;
}

void untangle(vector < vector < pair <int, int> > > *braid){
    int knot; //1 if first is on top, -1 if second
    int test_knot;
    int start_id;
    for(int j = 0; j < braid->at(0).size()-1; j++){
        start_id = 0;
        knot = 0;
        int ini_a = braid->at(start_id).at(j).first;
        int ini_b = braid->at(start_id).at(j+1).first;
        for(int i = 1; i < braid->size(); i++){
            int test_a = braid->at(i).at(j).first;
            int test_b = braid->at(i).at(j+1).first;
            test_knot = braid->at(i).at(j).second;
            if (ini_a == test_b && ini_b == test_a) {
                if (knot == -test_knot) {
                    for(int m = start_id+1; m< i; m++){
                        swap(braid->at(m).at(j).first, braid->at(m).at(j+1).first);
                        braid->at(m).at(j).second = 0;
                        braid->at(m).at(j+1).second = 0;
                    }
                    braid->at(i).at(j).second = 0;
                    braid->at(i).at(j+1).second = 0;
                }
                else{
                    //test_knot = braid->at(start_id).at(j).second;
                }
            }
            else{
                if(test_a == ini_a && test_b == ini_b){
                    test_knot = braid->at(start_id).at(j).second;
                }
                else{
                    start_id = i;
                }
            }
            knot = test_knot;
            ini_b = test_b;
            ini_a = test_a;
        }
    }
}

int main() {
    srand(time(NULL));
    cout << "Welcome to braid generator" << endl;
    int n = 3;
    int h = 10;
    //auto braid = generate(n, h);
    auto braid = read_data("C:\\Users\\Karol\\CLionProjects\\braids\\test.txt");
    print(braid);
    cout << "________\n";
    untangle(braid);
    untangle(braid);
    print(braid);

    return 0;
}
