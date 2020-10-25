// https://github.com/ismaelbelghiti/combinatorial-generators-cpp

#pragma once

#include <cstdlib>
#include <algorithm>
#include <vector>
#include <queue>
#include <numeric>
#include <string>

/*
  Return an integer uniformly drawn from range [mini,maxi]
*/
int rand_int(int mini, int maxi) {
    return rand() % (maxi - mini + 1) + mini;
}

/*
  Return a float uniformly drawn from range [mini, maxi]
*/
float rand_float(float mini, float maxi) {
    float x = (float)rand() / (float)RAND_MAX;
    return mini + x*(maxi-mini);
}


/*
  Return the result of std::random_shuffle on the input vector (which
  is given by value)
*/
template<typename T>
std::vector<T> rand_perm(std::vector<T> v) {
    std::random_shuffle(v.begin(), v.end());
    return v;
}


/*
  Return a vector of length 2*nbPairs, where each integer in [0,nbPairs) appears
  twice, defining a balanced list of parentheses uniformly drawn.
*/
std::vector<int> rand_parentheses(int nbPairs) {
    // we use the cyclic lemma
    std::vector<int> signs = std::vector<int>(nbPairs, 1);
    std::vector<int> minus(nbPairs + 1, -1);
    signs.insert(signs.end(), minus.begin(), minus.end());
    std::random_shuffle(signs.begin(), signs.end());

    std::vector<int> cum(2 * nbPairs + 1);
    std::partial_sum(signs.begin(), signs.end(), cum.begin());
    int iMin = min_element(cum.begin(), cum.end()) - cum.begin();
    std::rotate(signs.begin(), signs.begin() + iMin + 1, signs.end());
    signs.pop_back();

    std::vector<int> res;
    std::vector<int> idStack;
    int cur = 0;
    for (int i = 0; i < 2 * nbPairs; i++) {
        if (signs[i] == 1) {
            idStack.push_back(cur);
            cur++;
            res.push_back(idStack.back());
        } else {
            res.push_back(idStack.back());
            idStack.pop_back();
        }
    }
    return res;
}


/*
  Rand uniformly a spanning tree of the complete graph on vertex
  set {0,1,..,n-1}. The result is given as a list of edges.
*/
std::vector< std::pair<int,int> > rand_tree(int nbNodes) {
    // we use Prufer encoding
    std::vector<int> pruferCode;
    for (int i = 0; i < nbNodes - 2; i++) {
        pruferCode.push_back(rand_int(0, nbNodes - 1));
    }
    pruferCode.push_back(nbNodes - 1);

    std::vector< std::pair<int,int> > resEdges;
    std::vector<int> nbChildren(nbNodes, 0);
    for (int i = 0; i < nbNodes - 1; i++) {
        nbChildren[pruferCode[i]]++;
    }
    int posInCode = 0;
    for (int idNode = 0; idNode < nbNodes - 1; idNode++) {
        int idCur = idNode;
        while (idCur <= idNode && nbChildren[idCur] == 0) {
            int idFather = pruferCode[posInCode];
            posInCode++;
            resEdges.push_back(std::make_pair(idCur, idFather));
            nbChildren[idFather]--;
            idCur = idFather;
        }
    }
    return resEdges;
}


/*
  Rand uniformly a spanning tree of the graph whose number of vertices
  and edge list are given in input (we assume that the graph is connected
  and that the vertices are numbered  from 0 to nbNodes-1).
  The result is given as a list of edges. We use Wilson's algorithm.
  Remark : quite slow for big graphs.
*/
std::vector< std::pair<int,int> > rand_spanning_tree(
        int nbNodes, std::vector< std::pair<int,int> > edges) {

    if (nbNodes <= 1) {
        return std::vector< std::pair<int,int> >();
    }

    std::vector< std::vector<int> > neigh(nbNodes, std::vector<int>());
    for (int iEdge = 0; iEdge < (int)edges.size(); iEdge++) {
        neigh[edges[iEdge].first].push_back(edges[iEdge].second);
        neigh[edges[iEdge].second].push_back(edges[iEdge].first);
    }

    std::vector<bool> marked(nbNodes, false);
    marked[rand_int(0, nbNodes - 1)] = true;
    int nbMarkedNodes = 1;
    std::vector<int> suiv(nbNodes);
    std::vector< std::pair<int,int> > resEdges;

    while (nbMarkedNodes < nbNodes) {
        int idStart = rand_int(0, nbNodes - 1);
        int idCur = idStart;
        while (!marked[idCur]) {
            int idNeigh = neigh[idCur][rand() % neigh[idCur].size()];
            suiv[idCur] = idNeigh;
            idCur = idNeigh;
        }
        idCur = idStart;
        while (!marked[idCur]) {
            marked[idCur] = true;
            nbMarkedNodes++;
            resEdges.push_back(std::make_pair(idCur, suiv[idCur]));
            idCur = suiv[idCur];
        }
    }

    return resEdges;
}


/*
  Return the spanning tree obtained by a randomized DFS on the graph defined
  by its number of vertices and its edge list, starting the search from a given
  start vertex (we assume that the graph is connected and that the vertices are
  numbered from 0 to nbNodes-1). The result is given as a list of edges.
*/
std::vector< std::pair<int,int> > rand_dfs_tree(
        int nbNodes, std::vector< std::pair<int,int> > edges, int idStart) {

    std::vector< std::vector<int> > neigh(nbNodes, std::vector<int>());
    for (int iEdge = 0; iEdge < (int)edges.size(); iEdge++) {
        neigh[edges[iEdge].first].push_back(edges[iEdge].second);
        neigh[edges[iEdge].second].push_back(edges[iEdge].first);
    }
    for (int idNode = 0; idNode < nbNodes; idNode++) {
        std::random_shuffle(neigh[idNode].begin(), neigh[idNode].end());
    }

    std::vector< std::pair<int,int> > resEdges;
    std::vector<int> idNodeStack;
    std::vector<int> iNeighStack;
    std::vector<bool> marked(nbNodes, false);
    idNodeStack.push_back(idStart);
    iNeighStack.push_back(0);
    marked[idStart] = true;

    while (!idNodeStack.empty()) {
        int idNodeCur = idNodeStack.back();
        int iNeighCur = iNeighStack.back();
        if (iNeighCur == (int)neigh[idNodeCur].size()) {
            idNodeStack.pop_back();
            iNeighStack.pop_back();
            continue;
        }
        int idNeigh = neigh[idNodeCur][iNeighCur];
        iNeighStack.back()++;
        if (!marked[idNeigh]) {
            marked[idNeigh] = true;
            idNodeStack.push_back(idNeigh);
            iNeighStack.push_back(0);
            resEdges.push_back(std::make_pair(idNodeCur, idNeigh));
        }
    }
    return resEdges;
}


/*
  Return the spanning tree obtained by a randomized BFS on the graph defined
  by its number of vertices and its edge list, starting the search from a given
  start vertex (we assume that the graph is connected and that the vertices are
  numbered from 0 to nbNodes-1). The result is given as a list of edges.
*/
std::vector< std::pair<int,int> > rand_bfs_tree(
        int nbNodes, std::vector< std::pair<int,int> > edges, int idStart) {

    std::vector< std::vector<int> > neigh(nbNodes, std::vector<int>());
    for (int iEdge = 0; iEdge < (int)edges.size(); iEdge++) {
        neigh[edges[iEdge].first].push_back(edges[iEdge].second);
        neigh[edges[iEdge].second].push_back(edges[iEdge].first);
    }
    for (int idNode = 0; idNode < nbNodes; idNode++) {
        random_shuffle(neigh[idNode].begin(), neigh[idNode].end());
    }

    std::vector< std::pair<int,int> > resEdges;
    std::queue<int> idNodeQueue;
    std::vector<bool> marked(nbNodes, false);
    idNodeQueue.push(idStart);
    marked[idStart] = true;
    while (!idNodeQueue.empty()) {
        int idNodeCur = idNodeQueue.front();
        idNodeQueue.pop();
        for (int iNeigh = 0; iNeigh < (int)neigh[idNodeCur].size(); iNeigh++) {
            int idNeigh = neigh[idNodeCur][iNeigh];
            if (!marked[idNeigh]) {
                marked[idNeigh] = true;
                resEdges.push_back(std::make_pair(idNodeCur, idNeigh));
                idNodeQueue.push(idNeigh);
            }
        }
    }
    return resEdges;
}


/*
  Return the spanning tree obtained by a randomized Dijkstra on the graph defined
  by its number of vertices and its edge list, starting the search from a given
  start vertex (we assume that the graph is connected and that the vertices are
  numbered from 0 to nbNodes-1). The result is given as a list of edges.
*/
std::vector< std::pair<int,int> > rand_dijkstra_tree(
        int nbNodes, std::vector< std::pair<int,int> > edges, int idStart) {

    std::vector< std::vector<int> > neigh(nbNodes, std::vector<int>());
    for (int iEdge = 0; iEdge < (int)edges.size(); iEdge++) {
        neigh[edges[iEdge].first].push_back(edges[iEdge].second);
        neigh[edges[iEdge].second].push_back(edges[iEdge].first);
    }
    for (int idNode = 0; idNode < nbNodes; idNode++) {
        std::random_shuffle(neigh[idNode].begin(), neigh[idNode].end());
    }
    std::vector< std::pair<int,int> > resEdges;

    const int INVALID = -1;
    std::priority_queue< std::pair< float, std::pair<int,int> > > pq;
    std::vector< bool > visited(nbNodes, false);
    pq.push(std::make_pair(0.f, std::make_pair(idStart, INVALID)));
    while (!pq.empty()) {
        std::pair< float, std::pair<int,int> > cur = pq.top();
        pq.pop();
        int idNode = cur.second.first;
        if (visited[idNode]) {
            continue;
        }
        visited[idNode] = true;
        float curDist = -cur.first;
        int idFather = cur.second.second;
        if (idFather != INVALID) {
            resEdges.push_back(std::make_pair(idFather, idNode));
        }
        for (int iNeigh = 0; iNeigh < (int)neigh[idNode].size(); iNeigh++) {
            int idNeigh = neigh[idNode][iNeigh];
            float newDist = curDist + rand_float(0.f, 1.f);
            pq.push(std::make_pair(-newDist, std::make_pair(idNeigh, idNode)));
        }
    }

    return resEdges;
}


/*
  Return the spanning tree obtained by a randomized Prim-like search on the
  graph defined	 by its number of vertices and its edge list, starting the search
  from a given	start vertex (we assume that the graph is connected and that
  the vertices are numbered from 0 to nbNodes-1). The result is given as a list
  of edges.
*/
std::vector< std::pair<int,int> > rand_prim_tree(
        int nbNodes, std::vector< std::pair<int,int> > edges, int idStart) {

    std::vector< std::vector<int> > neigh(nbNodes, std::vector<int>());
    for (int iEdge = 0; iEdge < (int)edges.size(); iEdge++) {
        neigh[edges[iEdge].first].push_back(edges[iEdge].second);
        neigh[edges[iEdge].second].push_back(edges[iEdge].first);
    }
    for (int idNode = 0; idNode < nbNodes; idNode++) {
        std::random_shuffle(neigh[idNode].begin(), neigh[idNode].end());
    }

    const int INVALID = -1;
    std::vector< std::pair<int,int> > resEdges;
    std::vector< std::pair<int,int> > idActiveAndIdFatherNodes;
    std::vector<bool> marked(nbNodes,false);
    idActiveAndIdFatherNodes.push_back(std::make_pair(idStart, INVALID));
    while (!idActiveAndIdFatherNodes.empty()) {
        int iRand = rand_int(0, (int)idActiveAndIdFatherNodes.size() - 1);
        std::swap(idActiveAndIdFatherNodes[iRand], idActiveAndIdFatherNodes.back());
        int idNode = idActiveAndIdFatherNodes.back().first;
        int idFather = idActiveAndIdFatherNodes.back().second;
        idActiveAndIdFatherNodes.pop_back();

        if (marked[idNode]) {
            continue;
        }
        marked[idNode] = true;
        if (idFather != INVALID) {
            resEdges.push_back(std::make_pair(idFather,idNode));
        }

        for (int iNeigh = 0; iNeigh < (int)neigh[idNode].size(); iNeigh++) {
            int idNeigh = neigh[idNode][iNeigh];
            if (!marked[idNeigh]) {
                idActiveAndIdFatherNodes.push_back(std::make_pair(idNeigh, idNode));
            }
        }
    }
    return resEdges;
}



/*
  Return the spanning tree obtained by a hunt and kill process on the
  graph defined	 by its number of vertices and its edge list, starting the search
  from a given	start vertex (we assume that the graph is connected and that
  the vertices are numbered from 0 to nbNodes-1). The result is given as a list
  of edges.
*/
std::vector< std::pair<int,int> > rand_hunt_and_kill_tree(
        int nbNodes, std::vector< std::pair<int,int> > edges, int idStart) {

    std::vector< std::vector<int> > neigh(nbNodes, std::vector<int>());
    for (int iEdge = 0; iEdge < (int)edges.size(); iEdge++) {
        neigh[edges[iEdge].first].push_back(edges[iEdge].second);
        neigh[edges[iEdge].second].push_back(edges[iEdge].first);
    }
    for (int idNode = 0; idNode < nbNodes; idNode++) {
        std::random_shuffle(neigh[idNode].begin(), neigh[idNode].end());
    }

    const int INVALID = -1;
    std::vector< std::pair<int,int> > resEdges;
    std::vector< std::pair<int,int> > idActiveAndIdFatherNodes;
    std::vector<bool> marked(nbNodes, false);
    idActiveAndIdFatherNodes.push_back(std::make_pair(idStart, INVALID));
    while (!idActiveAndIdFatherNodes.empty()) {
        int iRand = rand_int(0, (int)idActiveAndIdFatherNodes.size() - 1);
        std::swap(idActiveAndIdFatherNodes[iRand], idActiveAndIdFatherNodes.back());
        int idNode = idActiveAndIdFatherNodes.back().first;
        int idFather = idActiveAndIdFatherNodes.back().second;
        idActiveAndIdFatherNodes.pop_back();

        if (marked[idNode]) {
            continue;
        }
        marked[idNode] = true;
        if (idFather != INVALID) {
            resEdges.push_back(std::make_pair(idFather, idNode));
        }
        while (true) {
            marked[idNode] = true;
            int idNext = idNode;
            for (int iNeigh = 0; iNeigh < (int)neigh[idNode].size(); iNeigh++) {
                int idNeigh = neigh[idNode][iNeigh];
                if (!marked[idNeigh]) {
                    idActiveAndIdFatherNodes.push_back(
                            std::make_pair(idNeigh,idNode));
                    idNext = idNode;
                }
            }
            if (idNext == idNode) {
                break;
            }
            resEdges.push_back(std::make_pair(idNode, idNext));
            idNode = idNext;
        }
    }
    return resEdges;
}


/*
  Return the spanning tree obtained through a randomized Kruskal-like process on the
  graph defined	 by its number of vertices and its edge list (we assume that the
  graph is connected and that the vertices are numbered from 0 to nbNodes-1).
  The result is given as a list of edges.
*/
std::vector< std::pair<int,int> > rand_kruskal_tree(
        int nbNodes, std::vector< std::pair<int,int> > edges) {

    std::vector< std::pair<int,int> > resEdges;
    std::vector<int> idGroup;
    for (int iGroup = 0; iGroup < nbNodes; iGroup++) {
        idGroup.push_back(iGroup);
    }
    std::vector< std::vector<int> > elementOfGroup(nbNodes, std::vector<int>());
    for (int idNode = 0; idNode < nbNodes; idNode++) {
        elementOfGroup[idNode].push_back(idNode);
    }

    int maxGroupSize = 1;
    while (maxGroupSize < nbNodes && !edges.empty()) {
        int iEdge = rand_int(0, (int)edges.size() - 1);
        std::swap(edges[iEdge], edges.back());
        std::pair<int,int> edge = edges.back();
        edges.pop_back();

        int iGroup1 = idGroup[edge.first], iGroup2 = idGroup[edge.second];
        if (iGroup1 == iGroup2) {
            continue;
        }

        resEdges.push_back(edge);
        if (elementOfGroup[iGroup1].size() > elementOfGroup[iGroup2].size()) {
            std::swap(iGroup1, iGroup2);
        }
        std::vector<int> & g1 = elementOfGroup[iGroup1];
        std::vector<int> & g2 = elementOfGroup[iGroup2];
        for (int i = 0; i < (int)g1.size(); i++) {
            int idNode = g1[i];
            g2.push_back(idNode);
            idGroup[idNode] = iGroup2;
        }
        g1.clear();
        if ((int)g2.size() > maxGroupSize) {
            maxGroupSize = (int)g2.size();
        }
    }

    return resEdges;
}

/*
  This function aims to explicit the edges of the underlying grid-graph in
  order to use a subgraph generation algorithm and then convert back its
  result to a maze with the function maze_from_edges defined just after.
*/
std::vector< std::pair<int,int> > maze_edges(int nbLines, int nbCols) {
    std::vector< std::pair<int,int> > resEdges;
    for(int iLine = 0; iLine < nbLines; iLine++) {
        for(int iCol = 0; iCol < nbCols; iCol++) {
            int idNode = iLine * nbCols + iCol;
            int idRightNeigh = idNode + 1;
            int idBottomNeigh = idNode + nbCols;
            if(iCol + 1 < nbCols) {
                resEdges.push_back(std::make_pair(idNode, idRightNeigh));
            }
            if(iLine + 1 < nbLines) {
                resEdges.push_back(std::make_pair(idNode, idBottomNeigh));
            }
        }
    }
    return resEdges;
}


/*
  This function is used to convert back a grid-graph subset of
  edges to a maze. Noting res the result, it can be read as follows:
  - there is a wall between cell (iLine, iCol) and cell (iLine, iCol + 1)
    when (res[iLine][iCol] & 1) == 0,
  - there is a wall between cell (iLine, iCol) and cell (iLine + 1, iCol)
    when (res[iLine][iCol] & 2) == 0.
  Remark: The walls seperating the grid cells  from outside are implicit, only
  the ones separating two celles of the grid are explicited. Please also note
  that operator & has not priority on operator == (that is why we use parentheses
  in the previous expressions).
*/
std::vector< std::vector<int> > maze_from_edges(
        int nbLines, int nbCols, std::vector< std::pair<int,int> > edges) {
    const int RIGHT = 1, BOTTOM = 2;
    std::vector< std::vector<int> > maze(nbLines, std::vector<int>(nbCols, 0));
    for(int iEdge = 0; iEdge < (int)edges.size(); iEdge++) {
        int idNode1 = edges[iEdge].first;
        int idNode2 = edges[iEdge].second;
        if(idNode1 > idNode2) {
            std::swap(idNode1, idNode2);
        }
        int iLine = idNode1 / nbCols;
        int iCol  = idNode1 % nbCols;
        if(idNode2 - idNode1 == 1 && nbCols > 1) { // right
            maze[iLine][iCol] += RIGHT;
        } else { // bottom
            maze[iLine][iCol] += BOTTOM;
        }
    }
    return maze;
}


/*
  This function is the equivalent of rand_tree for mazes.
*/
std::vector< std::vector<int> > rand_maze(int nbLines, int nbCols) {
    std::vector< std::pair<int,int> > treeEdges = rand_spanning_tree(
            nbLines * nbCols, maze_edges(nbLines, nbCols));
    return maze_from_edges(nbLines, nbCols, treeEdges);
}

#include <iostream>
using namespace std;

void display_maze1(vector< vector<int> > maze) {
    int nbLines = (int)maze.size(), nbCols = (int)maze[0].size();
    for(int iLine = 0; iLine < nbLines; iLine++) {
        for(int iCol = 0; iCol < nbCols; iCol++ ) {
            cout << ".";
            if(iCol < nbCols-1) {
                if(!(maze[iLine][iCol] & 1)) {
                    cout << "#";
                } else {
                    cout << ".";
                }
            }
        }
        cout << endl;
        if(iLine < nbLines-1) {
            for(int iCol = 0; iCol < nbCols; iCol++ ) {
                if(!(maze[iLine][iCol] & 2)) {
                    cout << "#";
                } else {
                    cout << ".";
                }
                if(iCol < nbCols - 1) {
                    cout << "#";
                }
            }
            cout << endl;
        }
    }
}


void display_maze(vector<vector<int>> maze) {
    int nbLines = (int)maze.size(), nbCols = (int)maze[0].size();
    for(int iLine = 0; iLine < nbLines; iLine++) {
        for(int iCol = 0; iCol < nbCols; iCol++ ) {
//            cout << ".";
//            if(iCol < nbCols-1) {
                cout << "L:" << iLine << "; C:" << iCol;
                if(!(maze[iLine][iCol] & 1)) {
//                    cout << "V";
                    cout << " V ";
                } else {
//                    cout << ".";
                }
//            }
//            cout << endl;
            if(!(maze[iLine][iCol] & 2)) {
//                cout << "H";
//                cout << "L:" << iLine << "; C:" << iCol << ": horizontal line";
                cout << " H ";
            } else {
//                cout << ".";
            }
            cout << endl;
        }
//        cout << endl;
//        if(iLine < nbLines-1) {
//            for(int iCol = 0; iCol < nbCols; iCol++ ) {
//                if(!(maze[iLine][iCol] & 2)) {
//                    cout << "H";
//                } else {
//                    cout << ".";
//                }
////                if(iCol < nbCols - 1) {
////                    cout << "#";
////                }
//            }
////            cout << endl;
//        }
    }
}
