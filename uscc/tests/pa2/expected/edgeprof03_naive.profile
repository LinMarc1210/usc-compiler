=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 200000000
while.cond -> while.end : 1
while.body -> if.then : 100000000
while.body -> if.end : 100000000
if.then -> if.end : 100000000
if.end -> if.then5 : 66666667
if.end -> if.end6 : 133333333
if.then5 -> if.end6 : 66666667
if.end6 -> if.then12 : 40000000
if.end6 -> if.end13 : 160000000
if.then12 -> if.end13 : 40000000
if.end13 -> if.then19 : 28571429
if.end13 -> if.end20 : 171428571
if.then19 -> if.end20 : 28571429
if.end20 -> if.then26 : 18181819
if.end20 -> if.end27 : 181818181
if.then26 -> if.end27 : 18181819
if.end27 -> while.cond : 200000000
END_PROFILE
