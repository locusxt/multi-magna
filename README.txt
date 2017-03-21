#######################
##### DESCRIPTION #####
#######################

This README describes the usage of the command line interface of multiMAGNA++, which implements
our network alignment algorithm, multiMAGNA++. multiMAGNA++ is a one-to-one multiple network
aligner and a genetic algorithm.

###################
##### OPTIONS #####
###################

-G [file containing list of networks to align]

This file should contain a list of file names of the networks to align. It will typically look like:

====== BEGIN ======
ex1.gw
ex2.gw
ex3.gw
======= END =======

Each of the network files can be either in LEDA (.gw) format or in edge list format.

 A typical LEDA file might look like:

====== BEGIN ======
LEDA.GRAPH
void
void
-2
3
|{A}|
|{B}|
|{C}|
3
1 2 0 |{}|
1 3 0 |{}|
2 3 0 |{}|
======= END =======

while a typical edge list format file might look like:

====== BEGIN ======
A B
A C
B C
======= END =======

In LEDA, the first four lines are meant to specify the format of the data to appear in the remainder
of the file, though my code does not need this information, and is therefore ignored. The fifth line
indicates the number of nodes in the network. Following this are the nodes themselves, each of
which is delimited on each end by `|{' and `}|', respectively. Edges are specified after the nodes,
in a similar format. See http://reference.wolfram.com/mathematica/ref/format/LEDA.html for more details.

For the edge list format, each line contains two node names that are adjacent to each other in the network.

------------------------------------------------------------

-o [output file prefix]
This parameter specifies the prefix each output alignment should have. Suppose we are aligning net1.gw, net2.gw and
net3.gw. Then we might want `net1_net2_net3' to appear at the beginning of the name of each alignment file outputted
by multiMAGNA++. The -o parameter provides the means to do this.

------------------------------------------------------------

-m [optimizing measure]
This implementation of multiMAGNA++ is capable of optimizing the conserved interaction quality (CIQ) measure
proposed in the BEAMS paper.
To optimize CIQ, pass `CIQ' to the -m flag. Note that `CIQ' appears in the names of the outputted alignment files.

------------------------------------------------------------

-d [file containing list of node similarity files]
This implementation of multiMAGNA++ allows us to add node conservation to edge conservation.
The input for this is the file that contains a list of the node similarity files.
It might look like:

====== BEGIN ======
ex1_ex3.dat
ex1_ex2.dat
ex2_ex3.dat
======= END =======

The scores in the node similarity files must be similarity scores, which means the higher the number,
the more similar the nodes are. The scores must also be non-negative real numbers.

One format of the score data file is real numbers separated by spaces. For example, with the source graph of 2 nodes
and target graph of 3 nodes, the score file will look as follows.
<FILE START>
2 3
0.90 0.75 0.50
0.65 0.95 0.60
<FILE END>
The final node score is calculated by taking the average of the node pairs.

Another format is:
<FILE START>
A1 A2 0.90
A1 B2 0.75
A1 C2 0.50
B1 A2 0.65
B1 B2 0.95
B1 C2 0.60
<FILE END>

------------------------------------------------------------

-a [alpha value]
This value, which is between 0 and 1 inclusive, accounts for the the weighting between the edge score and the node score.
Given edge score, E, and the node score N, the final score is
alpha*E + (1-alpha)*N
By default alpha = 1, which means it only uses edge scores by default.
If alpha = 0, then it uses only the node scores, which is equivalent to solving the generalized assignment problem.

------------------------------------------------------------

-p [population size]
This parameter specifies the size of the population used by multiMAGNA++. Generally, a larger population
size means better alignments. This value also appears in the names of the outputted alignment files.

------------------------------------------------------------

-e [elite percentage]
This value tells multiMAGNA++ what percentage of the population it should consider "elite," or how many of
the better alignments are passed to the next generation. By default, this value is 0.5, as multiMAGNA++ discards
half of the population with each new generation. The user is free to vary this parameter, though 0.5 typically
works best.

------------------------------------------------------------

-n [number of generations]
This parameter specifies the number of generations for which to run multiMAGNA++. Generally, running multiMAGNA++
for more generations yields higher quality alignments.

------------------------------------------------------------

-f [frequency of output]
This parameter is used to specify how many times multiMAGNA++ should output the best alignment in the population.
For example, suppose the user wants to run multiMAGNA++ for 1000 generations, with a frequency 5. Then the best
alignment from the population is written after generations 0, 200, 400, 600, 800, and 1000. (The best of the zeroth
generation is also outputted, regardless of the frequency). It is strongly recommended that the frequency of output
divide the number of generations. By default, the frequency is set to 1, which means only the best alignment from
the very last generation is outputted.

------------------------------------------------------------

-t [number of threads]
This parameter specifies the number of threads used to run multiMAGNA++.
The recommended number of threads is the number of cores available in the computer.


##############################
##### SAMPLE EXECUTION 1 #####
##############################

Suppose we have networks test_graph1.gw, test_graph2.gw, and test_graph3.gw.
Let test_graphs.txt contain a listing of the above files.
We want to optimize CIQ, and let multiMAGNA++ run for 100 generations on a population size of 10, using 64 threads.
In addition, we want the best alignment of the population halfway through execution. The following command does this.

./magna -G test_graphs.txt -o test_run -m CIQ -p 10 -n 100 -f 2 -t 64

After execution, as a result, the following three files are written in the same directory as the executable:
test_run_CIQ_10_100_0.txt
test_run_CIQ_10_100_50.txt
test_run_CIQ_10_100_100.txt


##############################
##### SAMPLE EXECUTION 2 #####
##############################

Suppose we have networks test_graph1.gw, test_graph2.gw, and test_graph3.gw.
Let test_graphs.txt contain a listing of the above files.
And node score similarity matrix files, test_graph1_graph2.dat, test_graph1_graph3.dat, and test_graph2_graph3.dat.
Let test_graphs_sim.txt contain a listing of the above files.
We want to optimize edge and node conservation, and let multiMAGNA++ run for 100 generations on a population size of 10. We want to weight the edge score and node score with 0.6.
In addition, we want the best alignment of the population halfway through execution. The following command does this.

./magna -G test_graphs.txt -o test_run -m CIQ -d test_graphs_sim.txt -p 10 -n 100 -f 2 -a 0.6

After execution, as a result, the following three files are written in the same directory as the executable:
test_run_CIQ_10_100_0.txt
test_run_CIQ_10_100_50.txt
test_run_CIQ_10_100_100.txt

##############################
##### SAMPLE EXECUTION 2 #####
##############################

The run.sh file in the data/ directory contains a demo of multiMAGNA++.

###################
##### AUTHORS #####
###################

Written by Vipin Vijayan (vvijayan@nd.edu)
In collaboration with Prof. Tijana Milenkovic (tmilenko@nd.edu)
