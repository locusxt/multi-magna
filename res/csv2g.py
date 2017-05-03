import csv
import pylab as pl

def parse_file(filename):
    f = open(filename, 'rb')
    reader = csv.reader(f)
    l1 = []
    l2 = []
    for r in reader:
        if len(r) == 0:
            continue
        l1.append(r[0])
        l2.append(r[1])
    return (l1, l2)

na = 10
nb = 10

for i in range(na):
    fname = 'a' + str(i) + '.csv'
    r = parse_file(fname)
    x = r[0]
    y = r[1]
    pl.plot(x, y, 'b')

for i in range(nb):
    fname = 'b' + str(i) + '.csv'
    r = parse_file(fname)
    x = r[0]
    y = r[1]
    pl.plot(x, y, 'r')

pl.show()
