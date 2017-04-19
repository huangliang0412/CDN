#include "graph.h"
#include "ga.h"
#include <stdio.h>
#include <string.h>
#include <deque>
#include <limits.h>
#include <iostream>
#include <math.h>
#include <algorithm>
#include <random>
#include <chrono>
void Graph::readtopo(char * topo[MAX_EDGE_NUM], int line_num) {
    int line = 0;
    int des, res, pri, wid;
    if (line < line_num)
        sscanf(topo[line], "%d %d %d", &NetNodeNum, &EdgeNum, &UserNum);
    sumNode = NetNodeNum + UserNum;
    bigUserNode = sumNode + 1;
    edgnumcursor = 0;
    memset(side, 0, sizeof(side));
    graph.resize(NetNodeNum, vector<PointToPoint>());
    p2pShortDist.resize(NetNodeNum, vector<int>(NetNodeNum, INF));
    line += 2;
    sscanf(topo[line], "%d", &server_cost);
    line += 2;
    for (int i = 0; i < EdgeNum; ++i, ++line) {
        sscanf(topo[line], "%d %d %d %d", &des, &res, &wid, &pri);
        graph[des].emplace_back(res, pri, wid);
        graph[res].emplace_back(des, pri, wid);
        addWidedge(des, res, wid, pri);
        addWidedge(res, des, wid, pri);
    }
    ++line;
    demandflow = 0;          //一共需要多少流量
    for (int i = 0; i < UserNum; ++i, ++line) {
        sscanf(topo[line], "%d %d %d", &des, &res, &wid);
        customer_nodes.emplace_back(res, wid);
        addWidedge(res, des + NetNodeNum, wid, 0);
        addWidedge(des + NetNodeNum, bigUserNode, wid, 0);         //把所有的消费节点合成一个大的节点
        demandflow += wid;
    }
    pedgnumcursor = edgnumcursor;
    gap = (int)(pow(UserNum,1/3.0) - 1.0);
}

void Graph::addWidedge(int res, int des, int wid, int pri) {
    Edge *edge1 = WidEdge + edgnumcursor++;
    Edge *edge2 = WidEdge + edgnumcursor++;
    *edge1 = {des, wid, pri, wid, pri, side[res], edge2};
    *edge2 = {res, 0, -pri, 0, -pri, side[des], edge1};
    side[res] = edge1;
    side[des] = edge2;
}

void getMcmf(vector<vector<int> > &point, vector<int> &ment, Graph &pic) {
    point.clear();
    ment.clear();
    while (true) {
        vector<int> vec;
        int a = pic.sumNode;
        int dis = INF;
        while (a != pic.bigUserNode) {
            bool flag=false;
            for (Edge *i = pic.side[a]; i; i = i->next) {
                int v = i->ind;
                if (i->ru > i->u) {
                    dis = min(dis, i->ru - i->u);
                    a = v;
                    flag = true;
                    break;
                }
            }
            if (!flag) break;
        }
        if (a != pic.bigUserNode) break;
        a = pic.sumNode;
        ment.push_back(dis);
        while (a != pic.bigUserNode) {
            for (Edge *i =pic.side[a]; i; i = i->next) {
                int v = i->ind;
                if (i->ru > i->u) {
                    i->u += dis;
                    a = v;
                    break;
                }
            }
            if (a != pic.bigUserNode) vec.push_back(a);
        }
        point.push_back(vec);
    }
}

int conditon(int n, int m, Graph &pic) {
    if (n ==pic.bigUserNode)
        return pic.cost += (long long)pic.dist * m, m;
    int temp = m;
    pic.isVisited[n] = true;
    for (Edge *i = pic.side[n]; i; i = i->next) {
        if (i->u && !i->c && !pic.isVisited[i->ind]) {
            int f = conditon(i->ind, min(temp, i->u), pic);
            i->u -= f;
            i->brother->u += f;
            temp -= f;
            if (!temp)
                return m;
        }
    }
    return m - temp;
}

bool flag(Graph &pic) {
    deque <int> q;
    memset(pic.isVisited , 0, sizeof(pic.isVisited));
    memset(pic.Dissopf, 0x3f, sizeof(pic.Dissopf));
    q.push_back(pic.sumNode);
    pic.Dissopf[pic.sumNode] = 0;
    pic.isVisited[pic.sumNode] = true;
    while (!q.empty ()) {
        int u = q.front ();
        q.pop_front ();
        for (Edge *i = pic.side[u]; i; i = i->next) {
            int v = i->ind;
            int dis = pic.Dissopf[u] + i->c;
            if (i->u && dis < pic.Dissopf[v]) {
                pic.Dissopf[v] = dis;
                if (!pic.isVisited[v]) {
                    pic.isVisited[v] = true;
                    if (q.size () && pic.Dissopf[v] < pic.Dissopf[q[0]])
                        q.push_front(v);
                    else
                        q.push_back(v);
                }
            }
        }
        pic.isVisited[u] = false;
    }
    for (Edge *i = pic.WidEdge; i < pic.WidEdge + pic.edgnumcursor; ++i) {
        i->c -=pic.Dissopf[i->ind] - pic.Dissopf[i->brother->ind];
    }
    pic.dist += pic.Dissopf[pic.bigUserNode];
    return pic.Dissopf[pic.bigUserNode] < INF;
}

int mcmf(Graph &pic) {

    //print_time("Begin");
    pic.flow = pic.dist = 0;
    pic.cost = 0;
    while (flag(pic)) {
        int tmpf;
        do {
            memset(pic.isVisited , 0, sizeof(pic.isVisited));
            tmpf = conditon(pic.sumNode, INT_MAX, pic);
            pic.flow += tmpf;
        } while (tmpf);
    }
    if (pic.flow != pic.demandflow)
        pic.cost = INFLL;
    //std::cout << "mcmf is excuted counts: "<< costflownumber++ << std::endl;

   // print_time("End");
    return pic.cost;
}

void kmeans(int KCount, vector<int> & clusters, Graph& pic) {
    clusters.resize(KCount);
    int label[MAXNODENUM];
    int shortest;
    int lowest;
    memset(label, -1, sizeof(label));
    vector<vector<int> > kmeanvec(KCount);

    //shuffle(pic.customer_nodes);
    random_shuffle(pic.customer_nodes.begin(), pic.customer_nodes.end());
    for (int i = 0; i < KCount; ++i) {
        clusters[i] = pic.customer_nodes[i].netNode;
    }
    while (true) {
        for (int i = 0; i < KCount; ++i)
            kmeanvec[i].clear();
        bool update = false;
        for (int i = 0; i < pic.UserNum; ++i) {
            shortest = INF;
            lowest = 0;
            for (int j = 0; j < KCount; ++j) {
                if(pic.p2pShortDist[clusters[j]][pic.customer_nodes[i].netNode] < shortest) {
                    shortest = pic.p2pShortDist[clusters[j]][pic.customer_nodes[i].netNode];
                    lowest = j;
                }
            }
            if (label[i] != lowest) {
                update = true;
                label[i] = lowest;
            }
            kmeanvec[label[i]].push_back(i);
        }
        if (!update)
            break;
        for (int j = 0; j < KCount; ++j) {
            shortest = INF;
            lowest = 0;
            for (int k = 0; k < pic.NetNodeNum; ++k) {
                int dist = 0;
                for (unsigned int i = 0; i < kmeanvec[j].size(); ++i) {
                    dist += pic.p2pShortDist[k][pic.customer_nodes[kmeanvec[j][i]].netNode];
                }
                if (dist < shortest) {
                    lowest = k;
                    shortest = dist;
                }
            }
            clusters[j] = lowest;
        }
    }
}

void spfa(Graph& pic) {
    //++spfacount;
    //cout <<"spfa is excuted count:" <<spfacount << endl;
    for(int i = 0; i < pic.NetNodeNum; ++i) {
        pic.p2pShortDist[i][i] = 0;
        deque<int> q;
        memset(pic.isVisited,0,sizeof(pic.isVisited));
        pic.isVisited[i] = true;
        q.push_back(i);    //q保存距离某个点从近到远的距离
        while (!q.empty()) {
            int index = q.front();
            q.pop_front();
            pic.isVisited[index] = false;
            for (unsigned int j = 0; j < pic.graph[index].size(); ++j) {
                int k = pic.graph[index][j].desti;
                int dis =  pic.p2pShortDist[i][index] + pic.graph[index][j].price;
                if (dis < pic.p2pShortDist[i][k])
                {
                    pic.p2pShortDist[i][k] = dis;
                    if (!pic.isVisited[k]) {
                        pic.isVisited[k] = true;
                        if (q.size () && pic.p2pShortDist[i][k] < pic.p2pShortDist[i][q[0]])
                            q.push_front(k);
                        else
                            q.push_back(k);
                    }
                }
            }
        }
    }
}

void ServerAdd(vector<int> &server, Graph &pic) {
    if (pic.edgnumcursor != pic.pedgnumcursor) {
        pic.edgnumcursor =pic.pedgnumcursor;
        for (Edge *j =pic.side[pic.sumNode]; j; j = j->next) {
            int k = j->ind;
            pic.side[k] = pic.side[k]->next;
        }
        pic.side[pic.sumNode] = 0;
        Edge *j = pic.WidEdge + pic.edgnumcursor;
        for (Edge *line = pic.WidEdge; line < j; ++line) {
            line->u = line->ru;
            line->c = line->rc;
        }
    }
    for (unsigned int i = 0; i < server.size(); ++i) {
        pic.addWidedge(pic.sumNode, server[i], INF, 0);
    }
}
