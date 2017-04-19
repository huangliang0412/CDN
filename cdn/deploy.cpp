#include "deploy.h"
#include "graph.h"
#include "ga.h"
#include <stdio.h>
#include <iostream>
#include <math.h>
using namespace std;
//You need to complete the function
bool isRecordPath = true;
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{

	// Output demo
	//char * topo_file = (char *)"17\n\n0 8 0 20\n21 8 0 20\n9 11 1 13\n21 22 2 20\n23 22 2 8\n1 3 3 11\n24 3 3 17\n27 3 3 26\n24 3 3 10\n18 17 4 11\n1 19 5 26\n1 16 6 15\n15 13 7 13\n4 5 8 18\n2 25 9 15\n0 7 10 10\n23 24 11 23";
	char * topo_file;
	Graph pic;
	pic.readtopo(topo, line_num);
	//int i = 0;
	PSO gaAlg(pic);
	vector<int> serverdistribution;
//	kmeans(pic.UserNum, serverdistribution, pic);
	int lowcost = pic.UserNum * pic.server_cost;
	int servNum = 0;
	vector<int> server;  //pic.gap
	for (int i = pic.UserNum - 1; i > 5; i -= pic.gap) {
    for(int j = 0; j < 10; j++){
			kmeans(i, server, pic);
			ServerAdd(server, pic);
			int cost = mcmf(pic) + i * pic.server_cost;
			//std::cout << cost << std::endl;
			if (cost < lowcost) {
				lowcost = cost;
				//std::cout << lowcost << std::endl;
				servNum = i;
				serverdistribution.swap(server);
			}
		}
	}
	gaAlg.addServer(serverdistribution, pic);
	double final = 89;
	if (pic.UserNum < 80){
		final = 60;
	}else if(pic.UserNum < 140){
		final = 80;
	}else{
		final = 89.2;
	}

	int max_p_size = log(servNum);  //1.5
	isRecordPath = false;
	final -= gaAlg.init(max_p_size, pic);
	int i = 0;
    /*
	double small = 0.8 / 1500;
	double middle = 0.8 / 730;
	double big = 0.8 / 140;
	double w = 0.9;
	double w1 = 0.8;
     */
	while ((double)clock() / CLOCKS_PER_SEC < final) {
		//cout << ++i << endl;
        /*
		if(pic.server_cost == 600){
			w -= small;
			gaAlg.run(pic, w);
		}
		else if(pic.server_cost == 800) {
			w-= middle;
			gaAlg.run(pic, w);
		}
		else {
			//w -= big;
			gaAlg.run(pic, w1);
		}
         */
   // while (true) {
        gaAlg.run(pic);

	}


	isRecordPath = true;
	vector<int> flow;
	gaAlg.placeServer(serverdistribution);
	ServerAdd(serverdistribution, pic);
	mcmf(pic);
	vector<vector<int> > result;
	getMcmf(result, flow, pic);
	int node_size = result.size();
	topo_file = new char[node_size * MAXNODENUM * 5];
	topo_file[0] = '\0';
	char line[MAXNODENUM * 5];
	char tmp[100];
	sprintf(line, "%d\n\n", node_size);
	strcat(topo_file, line);
	for (int i = 0; i < node_size; ++i) {
		line[0] = '\0';
		int node_size_1 = result[i].size() - 1;
		for (int j = 0; j < node_size_1; ++j) {
			sprintf(tmp, "%d ", result[i][j]);
			strcat(line, tmp);
		}
		sprintf(tmp, "%d ", result[i][node_size_1] - pic.NetNodeNum);
		strcat(line, tmp);
		sprintf(tmp, "%d\n", (int)flow[i]);
		strcat(line, tmp);
		strcat(topo_file, line);
	}
	write_result(topo_file, filename);
}
