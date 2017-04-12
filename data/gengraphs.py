def gen_complete_graph(prefix, node_num):
    res_str = "LEDA.GRAPH\nvoid\nvoid\n-2\n"
    res_str += str(node_num) + "\n"
    for i in range(node_num):
        res_str += "|{" + prefix + "_" + str(i + 1) + "}|" + "\n"
    res_str += str(node_num * (node_num - 1) / 2) + "\n"
    for i in range(node_num):
        for j in range(i + 1, node_num):
            res_str += str(i + 1) + " " + str(j + 1) + " 0 " + "|{" + prefix + "_" + \
            str(i + 1) +"_" + str(j + 1) + "}|\n"
    return res_str


def gen_graphs(prefix, graph_num, node_num):
    list_file_name = prefix + "list.txt"
    list_str = ""
    for i in range(graph_num):
        graph_prefix = prefix + str(i)
        graph_file_name = graph_prefix + ".gw"
        list_str += graph_file_name + "\n"
        graph_str = gen_complete_graph(graph_prefix, node_num)
        fb = open(graph_file_name, "w")
        fb.write(graph_str)
        fb.close()
    fb2 = open(list_file_name, "w")
    fb2.write(list_str)
    fb2.close()

gen_graphs("g", 50, 20)
#s = gen_complete_graph("g", 10)
#print(s)
