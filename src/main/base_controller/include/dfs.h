#pragma once

#include <iostream>

#include <map>
#include <string>
#include <vector>
#include <algorithm> // For std::find

using namespace std;

const string grape_yellow = "grape_yellow";
const string grape_green  = "grape_green" ;
const string grape_purple = "grape_purple";
const string banana       =       "banana";
const string lemon        =        "lemon";
const string apple_yellow = "apple_yellow"; 
const string basket_stand = "basket_stand";
    

struct Coord{
    string name;
    vector<double> pos;
    vector<Coord*> neighbor;
};

struct Object{
    string type;
    Coord coord;
};

static Coord A, B, C, D, E, F, G, H, I, J;

static void dfs( Coord * d, Coord * c, vector<Coord> path, double dist, vector<vector<double>> & d_path, double & d_dist, vector<string> checked ){

    path.push_back( *c );

    double local_dist = 0;

    for( Coord * n : c->neighbor ){

        double mag = sqrt( pow( c->pos[0] - n->pos[0], 2 ) + pow( c->pos[1] - n->pos[1], 2 ) );
        local_dist = dist + mag;
        
        bool reached = false;

        // Check if the neighbors of the desired point are the current point
        for( Coord * n_d : d->neighbor ){
            if( n->name == n_d->name ){ reached = true; }
        }
        // Check if the neighbors of the point being analyzed are the desired point
        for( Coord * n_c : n->neighbor ){
            if( d->name == n_c->name ){ reached = true; }
        }
        // Check if the point being analyzed is the desired point
        if( n->name == d->name ){ reached = true; }


        if( reached ){

            double mag_f = sqrt( pow( d->pos[0] - n->pos[0], 2 ) + pow( d->pos[1] - n->pos[1], 2 ) );
            for( Coord p : path ){
                cout << p.name << " " ;
            }
            cout << n->name << " " ;
            cout << d->name << " " ;

            double t_dist = local_dist + mag_f + path.size() + 2;
            cout << t_dist << endl;

            if( t_dist < d_dist || d_dist <= 0 ){
                d_path.clear();
                for( Coord p : path ){ 
                    d_path.push_back( p.pos ); 
                }
                d_path.push_back( n->pos ); 
                d_path.push_back( d->pos ); 
                
                d_dist = t_dist;
            }

            continue;
        }

        auto it = std::find(checked.begin(), checked.end(), n->name);
        if( it != checked.end() ){ continue; }
        checked.push_back( n->name );

        dfs( d, n, path, local_dist, d_path, d_dist, checked );
    }

}
