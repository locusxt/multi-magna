#!/usr/bin/env python
# -*- coding: utf-8 -*- 

import random

debug = 0

def gen_complete_graph(prefix, node_num):
    res_dict = {}
    res_dict['head'] = "LEDA.GRAPH\nvoid\nvoid\n-2"
    res_dict['nodes_num'] = node_num
    nodes_list = []
    for i in range(node_num):
        node_dict = {}
        node_dict['label'] = "|{" + prefix + "_" + str(i + 1) + "}|"
        node_dict['str'] = node_dict['label']
        nodes_list.append(node_dict)
    res_dict['nodes_list'] = nodes_list
    edges_list = [] 
    for i in range(node_num):
        for j in range(i + 1, node_num):
            edge_dict = {}
            edge_dict['src'] = str(i + 1)
            edge_dict['target'] = str(j + 1)
            edge_dict['label'] = "|{" + prefix + "_" + edge_dict['src'] + \
            '_' + edge_dict['target'] + "}|"
            edge_dict['type'] = "0"
            edge_dict['str'] = edge_dict['src'] + " " + edge_dict['target'] + \
            " " + edge_dict['type'] + " " + edge_dict['label']
            edges_list.append(edge_dict)
    res_dict['edges_list'] = edges_list
    res_dict['edges_num'] = len(edges_list)
    return res_dict

def graph2str(graph_dict):
    gd = graph_dict
    res = ""
    res += gd['head'] + '\n'
    res += str(gd['nodes_num']) + '\n'
    for n in gd['nodes_list']:
        res += n['str'] + '\n'
    res += str(gd['edges_num']) + '\n'
    for e in gd['edges_list']:
        res += e['str'] + '\n'
    return res

#not used
def gen_graphs(prefix, graph_num, node_num):
    file_name_list = prefix + "list.txt"
    list_str = ""
    for i in range(graph_num):
        graph_prefix = prefix + str(i)
        graph_file_name = graph_prefix + ".gw"
        
"""
    生成最小连通图
    每次连通一个点，把新的点随机连到一个已经连通的点上
    该生成方法，先加入的点有更大概率连到边
"""
def gen_min_graph(prefix, node_num):
    res_dict = {}
    res_dict['head'] = "LEDA.GRAPH\nvoid\nvoid\n-2"
    res_dict['nodes_num'] = node_num
    res_dict['prefix'] = prefix
    nodes_list = []
    for i in range(node_num):
        node_dict = {}
        node_dict['label'] = "{|" + prefix + "_" + str(i + 1) + "}|"
        node_dict['str'] = node_dict['label']
        nodes_list.append(node_dict)
    res_dict['nodes_list'] = nodes_list
    edges_list = []
    for i in range(1, node_num):#比i编号小的点保证都已经连通
        j = random.randint(0, i - 1)
        edge_dict = {}
        edge_dict['src'] = str(i + 1)
        edge_dict['target'] = str(j + 1)
        edge_dict['label'] = "|{" + prefix + "_" + edge_dict['src'] + \
            '_' + edge_dict['target'] + "}|"
        edge_dict['type'] = "0"
        edge_dict['str'] = edge_dict['src'] + " " + edge_dict['target'] + \
            " " + edge_dict['type'] + " " + edge_dict['label']
        edges_list.append(edge_dict)
    res_dict['edges_list'] = edges_list
    res_dict['edges_num'] = len(edges_list)
    return res_dict
    
def gen_mtx(node_num):
    res = {}
    for i in range(node_num):
        tmp_d = {}
        for j in range(node_num):
            tmp_d[j] = 0
        res[i] = tmp_d
    return res


"""
    随机生成一个图（无向图）
    在最小生成图上，增加ratio比例的边
    0:最小生成图,  1:完全图
"""
def gen_random_graph(prefix, node_num, ratio):
    gd = gen_min_graph(prefix, node_num)
    max_edges = node_num * (node_num - 1) / 2
    cur_edges = gd['edges_num']
    delta_edges = int((max_edges - cur_edges) * ratio)
    
    if debug == 1:
        print ("=======max, cur, delta edges=======")
        print (max_edges, cur_edges, delta_edges)
    #用邻接矩阵存储图结构，找还可以添加的边
    mtx_dict = gen_mtx(node_num)
    if debug == 1:
        print ("=======edges list=======")
        print (gd['edges_list'])
        print ("=======init mtx_dict=======")
        print (mtx_dict)
    for e in gd['edges_list']:
        s = e['src']
        t = e['target']
        mtx_dict[int(s)-1][int(t)-1] = 1
        mtx_dict[int(t)-1][int(s)-1] = 1
    if debug == 1:
        print ("=======processed mtx_dict=======")
        print (mtx_dict)
    psb_edges = [] #可增加的边 possible edges，无向图只存一个方向
    for i in range(node_num):
        for j in range(i + 1, node_num):
            if mtx_dict[i][j] == 0:
                psb_edges.append((i, j))
    if debug == 1:
        print ("=======psb edges=======")
        print (psb_edges)
    #随机从psb_edges中挑选边加入到图里
    edges_list = gd['edges_list']
    for i in range(delta_edges):
        cur_len = len(psb_edges)
        e_idx = random.randint(0, cur_len - 1)
        src = psb_edges[e_idx][0]
        tgt = psb_edges[e_idx][1]
        edge_dict = {}
        edge_dict['src'] = str(src + 1)
        edge_dict['target'] = str(tgt + 1)
        edge_dict['label'] = "|{" + prefix + "_" + edge_dict['src'] + \
            '_' + edge_dict['target'] + "}|"
        edge_dict['type'] = "0"
        edge_dict['str'] = edge_dict['src'] + " " + edge_dict['target'] + \
            " " + edge_dict['type'] + " " + edge_dict['label']
        edges_list.append(edge_dict)
        del psb_edges[e_idx]
    gd['edges_list'] = edges_list
    gd['edges_num'] = len(edges_list)
    return gd    
    

def copy_graph(new_prefix, graph):
    gd = graph
    nodes_num = gd['nodes_num']
    gd['prefix'] = new_prefix
    nodes_list = []
    prefix = new_prefix
    for i in range(nodes_num):
        node_dict = {}
        node_dict['label'] = "{|" + prefix + "_" + str(i + 1) + "}|"
        node_dict['str'] = node_dict['label']
        nodes_list.append(node_dict)
    gd['nodes_list'] = nodes_list
    edges_list = []
    for e in gd['edges_list']:
        src = e['src']
        tgt = e['target']
        e['label'] = "|{" + prefix + "_" + src + \
            '_' + tgt + "}|"
        e['str'] = src + " " + tgt + \
            " " + e['type'] + " " + e['label']
        edges_list.append(e)
    gd['edges_list'] = edges_list
    return gd

def gen_min_graphs(prefix, graph_num, node_num):
    list_file_name = prefix + "list.txt"
    list_str = ""
    gd = gen_min_graph('tmp', node_num)
    for i in range(graph_num):
        graph_prefix = prefix + str(i)
        graph_file_name = graph_prefix + ".gw"
        list_str += graph_file_name + "\n"
        new_g = copy_graph(graph_prefix, gd)
        graph_str = graph2str(new_g)
        fb = open(graph_file_name, "w")
        fb.write(graph_str)
        fb.close()
    fb2 = open(list_file_name, "w")
    fb2.write(list_str)
    fb2.close()

def gen_random_graphs(prefix, graph_num, node_num, ratio):
    list_file_name = prefix + "list.txt"
    list_str = ""
    gd = gen_random_graph('tmp', node_num, ratio)
    for i in range(graph_num):
        graph_prefix = prefix + str(i)
        graph_file_name = graph_prefix + ".gw"
        list_str += graph_file_name + "\n"
        new_g = copy_graph(graph_prefix, gd)
        graph_str = graph2str(new_g)
        fb = open(graph_file_name, "w")
        fb.write(graph_str)
        fb.close()
    fb2 = open(list_file_name, "w")
    fb2.write(list_str)



#TODO 基于图的随机增加删除
#TODO 友好的main函数


#d = gen_random_graph('g', 10, 0.5)
#s = graph2str(d)
#print(s)

#print random.randint(0, 2)

#gen_min_graphs("g", 20, 20)
gen_random_graphs("r", 20, 20, 0.1)
