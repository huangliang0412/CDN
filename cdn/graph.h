//#pragma onece

#ifndef CDN_GRAPH_H
#define CDN_GRAPH_H

#include "lib_io.h"
#include "ga.h"
#include <stdlib.h>
#include <vector>
#include <string.h>
using namespace std;

const int MAXNODENUM = 2000;
const int INF = 9999;
const int INFLL = 9999999;

struct Graph;
int conditon(int n, int m, Graph &pic);
bool flag(Graph &pic);
void ServerAdd(vector<int> &server, Graph &pic);

int mcmf(Graph &pic);
void getMcmf(vector<vector<int> > &point, vector<int> &ment, Graph &pic);

void spfa(Graph& pic);
void kmeans(int KCount, vector<int> & clusters, Graph& pic);
struct PointToPoint {
    int desti;
    int price;
    int wid;
    PointToPoint(int p_desti, int p_price, int p_wid): desti(p_desti), price(p_price), wid(p_wid) {}
};

struct UserNode {
    int netNode;
    int demand;
    UserNode(int p_netNode, int p_demand): netNode(p_netNode), demand(p_demand) {}
};

struct Edge {
    int ind;
    int u;
    int c;
    int ru;
    int rc;
    Edge *next;
    Edge *brother;
};
struct Graph {
    vector<vector<PointToPoint> > graph;
    vector<UserNode> customer_nodes;
    vector<vector<int> > p2pShortDist;
    bool isVisited[MAXNODENUM];
    Edge WidEdge[MAX_EDGE_NUM * 4];
    Edge *side[MAXNODENUM];
    int edgnumcursor;
    int sumNode;
    int bigUserNode;
    int pedgnumcursor;
    int flow;
    int dist;
    int Dissopf[MAXNODENUM];
    int cost;
    int demandflow, NetNodeNum, EdgeNum, UserNum;
    int server_cost;
    int gap;

    void readtopo(char * topo[MAX_EDGE_NUM], int line_num);
    void addWidedge(int res, int des, int wid, int pri);

};

#endif //CDN_GRAPH_Htemplate <class T>

